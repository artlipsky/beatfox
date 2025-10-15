#import "MetalSimulationBackend.h"

#ifdef __APPLE__
#import <Metal/Metal.h>
#import <Foundation/Foundation.h>
#import <chrono>
#import <iostream>

/*
 * Metal Simulation Parameters Structure
 * Must match the struct in WaveEquation.metal
 */
struct SimulationParams {
    int width;
    int height;
    float c2_dt2_dx2;
    float damping;
    float wallReflection;
    float twoOverDamping;
};

/*
 * Multi-step simulation parameters
 * Must match the struct in WaveEquation.metal
 */
struct MultiStepParams {
    int width;
    int height;
    float c2_dt2_dx2;
    float damping;
    float wallReflection;
    float twoOverDamping;
    int currentIdx;
    int prevIdx;
    int nextIdx;
    int listenerX;
    int listenerY;
    int subStepIdx;
    int numAudioSources;
    // NEW: Active region optimization
    int offsetX;        // X offset of active region
    int offsetY;        // Y offset of active region
    int activeWidth;    // Width of active region
    int activeHeight;   // Height of active region
};

/*
 * Audio source data (per sub-step)
 * Must match the struct in WaveEquation.metal
 */
struct AudioSourceData {
    int x;
    int y;
    float pressure;
};

/*
 * MetalSimulationBackend Implementation
 *
 * Uses Metal compute shaders to accelerate wave propagation on GPU
 */
class MetalSimulationBackend::Impl {
public:
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
    id<MTLComputePipelineState> computePipeline;
    id<MTLComputePipelineState> multiStepPipeline;  // New: multi-step kernel
    id<MTLLibrary> library;

    // GPU buffers (single-step execution)
    id<MTLBuffer> pressureBuffer;
    id<MTLBuffer> pressurePrevBuffer;
    id<MTLBuffer> pressureNextBuffer;
    id<MTLBuffer> obstaclesBuffer;
    id<MTLBuffer> paramsBuffer;

    // GPU buffers (multi-step execution)
    id<MTLBuffer> tripleBuffer;         // 3x concatenated pressure buffers
    id<MTLBuffer> multiStepParamsBuffer; // Multi-step parameters (LEGACY - for single buffer approach)
    id<MTLBuffer> listenerSamplesBuffer; // Listener samples output
    id<MTLBuffer> audioSourcesBuffer;    // Audio source data (LEGACY - for single buffer approach)

    // NEW: Pre-allocated arrays of buffers (one per sub-step)
    std::vector<id<MTLBuffer>> multiStepParamsBuffers;  // Array of parameter buffers
    std::vector<id<MTLBuffer>> audioSourcesBuffers;     // Array of audio source buffers
    static const int MAX_SUBSTEPS = 1500;  // Maximum sub-steps per frame (handles real-time at 8.6mm)

    int width;
    int height;
    int gridSize;

    // Performance tracking
    std::chrono::high_resolution_clock::time_point lastStepStart;
    double lastStepTimeMs;
    double totalTimeMs;
    uint64_t totalSteps;

    Impl() :
        device(nil),
        commandQueue(nil),
        computePipeline(nil),
        multiStepPipeline(nil),
        library(nil),
        pressureBuffer(nil),
        pressurePrevBuffer(nil),
        pressureNextBuffer(nil),
        obstaclesBuffer(nil),
        paramsBuffer(nil),
        tripleBuffer(nil),
        multiStepParamsBuffer(nil),
        listenerSamplesBuffer(nil),
        audioSourcesBuffer(nil),
        width(0),
        height(0),
        gridSize(0),
        lastStepTimeMs(0.0),
        totalTimeMs(0.0),
        totalSteps(0)
    {}

    ~Impl() {
        cleanup();
    }

    void cleanup() {
        // With ARC enabled, just set to nil - ARC handles deallocation automatically
        pressureBuffer = nil;
        pressurePrevBuffer = nil;
        pressureNextBuffer = nil;
        obstaclesBuffer = nil;
        paramsBuffer = nil;
        tripleBuffer = nil;
        multiStepParamsBuffer = nil;
        listenerSamplesBuffer = nil;
        audioSourcesBuffer = nil;

        // Clean up buffer arrays
        for (auto& buffer : multiStepParamsBuffers) {
            buffer = nil;
        }
        multiStepParamsBuffers.clear();

        for (auto& buffer : audioSourcesBuffers) {
            buffer = nil;
        }
        audioSourcesBuffers.clear();

        computePipeline = nil;
        multiStepPipeline = nil;
        library = nil;
        commandQueue = nil;
        device = nil;
    }
};

MetalSimulationBackend::MetalSimulationBackend() : pImpl(new Impl()) {}

MetalSimulationBackend::~MetalSimulationBackend() {
    delete pImpl;
}

bool MetalSimulationBackend::initialize(int width, int height) {
    pImpl->width = width;
    pImpl->height = height;
    pImpl->gridSize = width * height;

    // Get default Metal device
    pImpl->device = MTLCreateSystemDefaultDevice();
    if (!pImpl->device) {
        lastError = "Metal is not supported on this system";
        std::cerr << "MetalBackend: " << lastError << std::endl;
        return false;
    }

    std::cout << "MetalBackend: Using GPU: " << [[pImpl->device name] UTF8String] << std::endl;

    // Create command queue
    pImpl->commandQueue = [pImpl->device newCommandQueue];
    if (!pImpl->commandQueue) {
        lastError = "Failed to create Metal command queue";
        std::cerr << "MetalBackend: " << lastError << std::endl;
        return false;
    }

    // Load Metal shader library
    NSError* error = nil;
    NSString* shaderPath = @"src/shaders/WaveEquation.metal";
    NSString* shaderSource = [NSString stringWithContentsOfFile:shaderPath
                                                       encoding:NSUTF8StringEncoding
                                                          error:&error];

    if (error || !shaderSource) {
        lastError = "Failed to load Metal shader file";
        std::cerr << "MetalBackend: " << lastError << std::endl;
        return false;
    }

    pImpl->library = [pImpl->device newLibraryWithSource:shaderSource
                                                  options:nil
                                                    error:&error];
    if (error || !pImpl->library) {
        if (error) {
            lastError = std::string("Failed to compile Metal shader: ") + [[error localizedDescription] UTF8String];
        } else {
            lastError = "Failed to create Metal library";
        }
        std::cerr << "MetalBackend: " << lastError << std::endl;
        return false;
    }

    // Get kernel function
    id<MTLFunction> kernelFunction = [pImpl->library newFunctionWithName:@"waveEquationStep"];
    if (!kernelFunction) {
        lastError = "Failed to find kernel function 'waveEquationStep'";
        std::cerr << "MetalBackend: " << lastError << std::endl;
        return false;
    }

    // Create compute pipeline
    pImpl->computePipeline = [pImpl->device newComputePipelineStateWithFunction:kernelFunction
                                                                          error:&error];
    // ARC handles deallocation of kernelFunction automatically

    if (error || !pImpl->computePipeline) {
        if (error) {
            lastError = std::string("Failed to create compute pipeline: ") + [[error localizedDescription] UTF8String];
        } else {
            lastError = "Failed to create compute pipeline";
        }
        std::cerr << "MetalBackend: " << lastError << std::endl;
        return false;
    }

    // Get multi-step kernel function
    id<MTLFunction> multiStepKernelFunction = [pImpl->library newFunctionWithName:@"multiStepWaveEquation"];
    if (!multiStepKernelFunction) {
        lastError = "Failed to find kernel function 'multiStepWaveEquation'";
        std::cerr << "MetalBackend: " << lastError << std::endl;
        return false;
    }

    // Create multi-step compute pipeline
    pImpl->multiStepPipeline = [pImpl->device newComputePipelineStateWithFunction:multiStepKernelFunction
                                                                            error:&error];
    if (error || !pImpl->multiStepPipeline) {
        if (error) {
            lastError = std::string("Failed to create multi-step compute pipeline: ") + [[error localizedDescription] UTF8String];
        } else {
            lastError = "Failed to create multi-step compute pipeline";
        }
        std::cerr << "MetalBackend: " << lastError << std::endl;
        return false;
    }

    // Create GPU buffers (using shared storage for unified memory on Apple Silicon)
    NSUInteger bufferSize = pImpl->gridSize * sizeof(float);
    NSUInteger obstaclesSize = pImpl->gridSize * sizeof(uint8_t);

    pImpl->pressureBuffer = [pImpl->device newBufferWithLength:bufferSize
                                                       options:MTLResourceStorageModeShared];
    pImpl->pressurePrevBuffer = [pImpl->device newBufferWithLength:bufferSize
                                                           options:MTLResourceStorageModeShared];
    pImpl->pressureNextBuffer = [pImpl->device newBufferWithLength:bufferSize
                                                           options:MTLResourceStorageModeShared];
    pImpl->obstaclesBuffer = [pImpl->device newBufferWithLength:obstaclesSize
                                                        options:MTLResourceStorageModeShared];
    pImpl->paramsBuffer = [pImpl->device newBufferWithLength:sizeof(SimulationParams)
                                                      options:MTLResourceStorageModeShared];

    // Multi-step buffers
    // Triple buffer: 3× pressure field size for index-based triple buffering
    NSUInteger tripleBufferSize = 3 * bufferSize;
    pImpl->tripleBuffer = [pImpl->device newBufferWithLength:tripleBufferSize
                                                      options:MTLResourceStorageModeShared];

    // Multi-step params buffer
    pImpl->multiStepParamsBuffer = [pImpl->device newBufferWithLength:sizeof(MultiStepParams)
                                                              options:MTLResourceStorageModeShared];

    // Listener samples buffer (max ~500 sub-steps per frame)
    NSUInteger maxSubSteps = 500;
    NSUInteger listenerSamplesSize = maxSubSteps * sizeof(float);
    pImpl->listenerSamplesBuffer = [pImpl->device newBufferWithLength:listenerSamplesSize
                                                               options:MTLResourceStorageModeShared];

    // Audio sources buffer (max 16 audio sources)
    NSUInteger maxAudioSources = 16;
    NSUInteger audioSourcesSize = maxAudioSources * sizeof(AudioSourceData);
    pImpl->audioSourcesBuffer = [pImpl->device newBufferWithLength:audioSourcesSize
                                                            options:MTLResourceStorageModeShared];

    if (!pImpl->pressureBuffer || !pImpl->pressurePrevBuffer || !pImpl->pressureNextBuffer ||
        !pImpl->obstaclesBuffer || !pImpl->paramsBuffer ||
        !pImpl->tripleBuffer || !pImpl->multiStepParamsBuffer || !pImpl->listenerSamplesBuffer ||
        !pImpl->audioSourcesBuffer) {
        lastError = "Failed to create Metal buffers";
        std::cerr << "MetalBackend: " << lastError << std::endl;
        return false;
    }

    // NEW: Pre-allocate arrays of buffers for batched execution
    // This eliminates the need for per-step synchronization waits!
    std::cout << "MetalBackend: Pre-allocating " << pImpl->MAX_SUBSTEPS << " parameter buffers for batched execution..." << std::endl;

    pImpl->multiStepParamsBuffers.reserve(pImpl->MAX_SUBSTEPS);
    pImpl->audioSourcesBuffers.reserve(pImpl->MAX_SUBSTEPS);

    for (int i = 0; i < pImpl->MAX_SUBSTEPS; i++) {
        // Allocate parameter buffer for this sub-step
        id<MTLBuffer> paramsBuffer = [pImpl->device newBufferWithLength:sizeof(MultiStepParams)
                                                                options:MTLResourceStorageModeShared];
        if (!paramsBuffer) {
            lastError = "Failed to pre-allocate parameter buffer array";
            std::cerr << "MetalBackend: " << lastError << std::endl;
            return false;
        }
        pImpl->multiStepParamsBuffers.push_back(paramsBuffer);

        // Allocate audio sources buffer for this sub-step
        id<MTLBuffer> audioBuffer = [pImpl->device newBufferWithLength:audioSourcesSize
                                                               options:MTLResourceStorageModeShared];
        if (!audioBuffer) {
            lastError = "Failed to pre-allocate audio sources buffer array";
            std::cerr << "MetalBackend: " << lastError << std::endl;
            return false;
        }
        pImpl->audioSourcesBuffers.push_back(audioBuffer);
    }

    std::cout << "MetalBackend: Initialized successfully" << std::endl;
    std::cout << "MetalBackend: Grid size: " << width << "x" << height << " (" << pImpl->gridSize << " cells)" << std::endl;
    std::cout << "MetalBackend: Buffer size: " << (bufferSize / 1024.0f / 1024.0f) << " MB per field" << std::endl;
    float totalBufferSize = (bufferSize * 3 + listenerSamplesSize +
                             pImpl->MAX_SUBSTEPS * (sizeof(MultiStepParams) + audioSourcesSize)) / 1024.0f / 1024.0f;
    std::cout << "MetalBackend: Total GPU memory: " << totalBufferSize << " MB (including " << pImpl->MAX_SUBSTEPS << " pre-allocated parameter buffers)" << std::endl;

    return true;
}

bool MetalSimulationBackend::isAvailable() const {
    return pImpl->device != nil && pImpl->computePipeline != nil;
}

void MetalSimulationBackend::executeStep(
    const std::vector<float>& pressure,
    const std::vector<float>& pressurePrev,
    std::vector<float>& pressureNext,
    const std::vector<uint8_t>& obstacles,
    float c2_dt2_dx2,
    float damping,
    float wallReflection)
{
    if (!isAvailable()) {
        return;
    }

    pImpl->lastStepStart = std::chrono::high_resolution_clock::now();

    // Copy data to GPU buffers (unified memory = fast)
    memcpy([pImpl->pressureBuffer contents], pressure.data(), pImpl->gridSize * sizeof(float));
    memcpy([pImpl->pressurePrevBuffer contents], pressurePrev.data(), pImpl->gridSize * sizeof(float));
    memcpy([pImpl->obstaclesBuffer contents], obstacles.data(), pImpl->gridSize * sizeof(uint8_t));

    // Set simulation parameters
    SimulationParams* params = (SimulationParams*)[pImpl->paramsBuffer contents];
    params->width = pImpl->width;
    params->height = pImpl->height;
    params->c2_dt2_dx2 = c2_dt2_dx2;
    params->damping = damping;
    params->wallReflection = wallReflection;
    params->twoOverDamping = 2.0f * damping;

    // Create command buffer
    id<MTLCommandBuffer> commandBuffer = [pImpl->commandQueue commandBuffer];
    id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];

    // Set compute pipeline and buffers
    [computeEncoder setComputePipelineState:pImpl->computePipeline];
    [computeEncoder setBuffer:pImpl->pressureBuffer offset:0 atIndex:0];
    [computeEncoder setBuffer:pImpl->pressurePrevBuffer offset:0 atIndex:1];
    [computeEncoder setBuffer:pImpl->pressureNextBuffer offset:0 atIndex:2];
    [computeEncoder setBuffer:pImpl->obstaclesBuffer offset:0 atIndex:3];
    [computeEncoder setBuffer:pImpl->paramsBuffer offset:0 atIndex:4];

    // Calculate thread groups (16x16 threads per group is optimal for M-series GPUs)
    MTLSize threadgroupSize = MTLSizeMake(16, 16, 1);
    MTLSize gridSize = MTLSizeMake(
        (pImpl->width + threadgroupSize.width - 1) / threadgroupSize.width * threadgroupSize.width,
        (pImpl->height + threadgroupSize.height - 1) / threadgroupSize.height * threadgroupSize.height,
        1
    );

    // Dispatch compute kernel
    [computeEncoder dispatchThreads:gridSize threadsPerThreadgroup:threadgroupSize];
    [computeEncoder endEncoding];

    // Commit and wait for completion
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];

    // Copy result back to CPU
    memcpy(pressureNext.data(), [pImpl->pressureNextBuffer contents], pImpl->gridSize * sizeof(float));

    // Update performance stats
    auto stepEnd = std::chrono::high_resolution_clock::now();
    pImpl->lastStepTimeMs = std::chrono::duration<double, std::milli>(stepEnd - pImpl->lastStepStart).count();
    pImpl->totalTimeMs += pImpl->lastStepTimeMs;
    pImpl->totalSteps++;
}

void MetalSimulationBackend::executeFrame(
    const std::vector<float>& initialPressure,
    const std::vector<float>& initialPressurePrev,
    std::vector<float>& finalPressure,
    std::vector<float>& finalPressurePrev,
    const std::vector<uint8_t>& obstacles,
    std::vector<float>& listenerSamples,
    const std::vector<std::vector<AudioSourceData>>& audioSourcesPerStep,
    int listenerX,
    int listenerY,
    int numSubSteps,
    float c2_dt2_dx2,
    float damping,
    float wallReflection,
    int activeMinX,
    int activeMinY,
    int activeMaxX,
    int activeMaxY)
{
    if (!isAvailable()) {
        return;
    }

    pImpl->lastStepStart = std::chrono::high_resolution_clock::now();

    // Calculate active region dimensions
    // If no active region specified (defaults), use full grid
    if (activeMaxX < 0) activeMaxX = pImpl->width - 1;
    if (activeMaxY < 0) activeMaxY = pImpl->height - 1;

    int offsetX = activeMinX;
    int offsetY = activeMinY;
    int activeWidth = activeMaxX - activeMinX + 1;
    int activeHeight = activeMaxY - activeMinY + 1;

    // Clamp to grid bounds
    offsetX = std::max(0, std::min(offsetX, pImpl->width - 1));
    offsetY = std::max(0, std::min(offsetY, pImpl->height - 1));
    activeWidth = std::max(1, std::min(activeWidth, pImpl->width - offsetX));
    activeHeight = std::max(1, std::min(activeHeight, pImpl->height - offsetY));

    // Log optimization stats (only if active region is smaller than full grid)
    if (activeWidth * activeHeight < pImpl->gridSize) {
        float coveragePercent = 100.0f * (activeWidth * activeHeight) / float(pImpl->gridSize);
        std::cout << "MetalBackend: Active region optimization - computing "
                  << activeWidth << "x" << activeHeight << " (" << coveragePercent << "% of grid)" << std::endl;
    }

    // Resize output vectors
    listenerSamples.resize(numSubSteps, 0.0f);
    finalPressure.resize(pImpl->gridSize);
    finalPressurePrev.resize(pImpl->gridSize);

    // Copy initial state to GPU triple buffer
    // Buffer 0: initial current pressure
    // Buffer 1: initial previous pressure
    // Buffer 2: will hold next state
    float* tripleBufferPtr = (float*)[pImpl->tripleBuffer contents];
    memcpy(tripleBufferPtr, initialPressure.data(), pImpl->gridSize * sizeof(float));  // Buffer 0
    memcpy(tripleBufferPtr + pImpl->gridSize, initialPressurePrev.data(), pImpl->gridSize * sizeof(float));  // Buffer 1
    // Buffer 2 doesn't need initialization - it will be written to

    // Copy obstacles to GPU
    memcpy([pImpl->obstaclesBuffer contents], obstacles.data(), pImpl->gridSize * sizeof(uint8_t));

    // Validate numSubSteps doesn't exceed our pre-allocated buffer capacity
    if (numSubSteps > pImpl->MAX_SUBSTEPS) {
        std::cerr << "MetalBackend: ERROR: numSubSteps (" << numSubSteps
                  << ") exceeds MAX_SUBSTEPS (" << pImpl->MAX_SUBSTEPS << ")" << std::endl;
        numSubSteps = pImpl->MAX_SUBSTEPS;  // Clamp to maximum
    }

    // PHASE 1: Pre-fill all parameter and audio source buffers upfront
    // This allows GPU to run all sub-steps without CPU intervention!
    int currentIdx = 0;  // Initial current
    int prevIdx = 1;     // Initial previous
    int nextIdx = 2;     // Initial next

    for (int step = 0; step < numSubSteps; step++) {
        // Get audio sources for this sub-step
        int numAudioSources = 0;
        if (step < static_cast<int>(audioSourcesPerStep.size())) {
            numAudioSources = std::min(static_cast<int>(audioSourcesPerStep[step].size()), 16);
            if (numAudioSources > 0) {
                // Copy audio source data to this step's dedicated buffer
                AudioSourceData* audioSourcesPtr = (AudioSourceData*)[pImpl->audioSourcesBuffers[step] contents];
                memcpy(audioSourcesPtr, audioSourcesPerStep[step].data(), numAudioSources * sizeof(AudioSourceData));
            }
        }

        // Set up parameters in this step's dedicated buffer
        MultiStepParams* params = (MultiStepParams*)[pImpl->multiStepParamsBuffers[step] contents];
        params->width = pImpl->width;
        params->height = pImpl->height;
        params->c2_dt2_dx2 = c2_dt2_dx2;
        params->damping = damping;
        params->wallReflection = wallReflection;
        params->twoOverDamping = 2.0f * damping;
        params->currentIdx = currentIdx;
        params->prevIdx = prevIdx;
        params->nextIdx = nextIdx;
        params->listenerX = listenerX;
        params->listenerY = listenerY;
        params->subStepIdx = step;
        params->numAudioSources = numAudioSources;
        // Active region optimization fields
        params->offsetX = offsetX;
        params->offsetY = offsetY;
        params->activeWidth = activeWidth;
        params->activeHeight = activeHeight;

        // Rotate buffer indices for next iteration
        int temp = prevIdx;
        prevIdx = currentIdx;
        currentIdx = nextIdx;
        nextIdx = temp;
    }

    // PHASE 2: Create and encode all command buffers at once
    // No waits! GPU can execute all sub-steps in parallel/pipelined fashion
    std::vector<id<MTLCommandBuffer>> commandBuffers;
    commandBuffers.reserve(numSubSteps);

    currentIdx = 0;  // Reset for encoding
    prevIdx = 1;
    nextIdx = 2;

    for (int step = 0; step < numSubSteps; step++) {
        // Create command buffer for this step
        id<MTLCommandBuffer> commandBuffer = [pImpl->commandQueue commandBuffer];

        // Create compute encoder
        id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];

        // Set compute pipeline and buffers
        // Each step uses its own pre-filled parameter and audio buffers!
        [computeEncoder setComputePipelineState:pImpl->multiStepPipeline];
        [computeEncoder setBuffer:pImpl->tripleBuffer offset:0 atIndex:0];
        [computeEncoder setBuffer:pImpl->obstaclesBuffer offset:0 atIndex:1];
        [computeEncoder setBuffer:pImpl->listenerSamplesBuffer offset:0 atIndex:2];
        [computeEncoder setBuffer:pImpl->multiStepParamsBuffers[step] offset:0 atIndex:3];
        [computeEncoder setBuffer:pImpl->audioSourcesBuffers[step] offset:0 atIndex:4];

        // Calculate thread groups (16x16 threads per group is optimal for M-series GPUs)
        MTLSize threadgroupSize = MTLSizeMake(16, 16, 1);
        // OPTIMIZED: Dispatch only threads for active region, not full grid!
        MTLSize gridSize = MTLSizeMake(
            (activeWidth + threadgroupSize.width - 1) / threadgroupSize.width * threadgroupSize.width,
            (activeHeight + threadgroupSize.height - 1) / threadgroupSize.height * threadgroupSize.height,
            1
        );

        // Dispatch compute kernel
        [computeEncoder dispatchThreads:gridSize threadsPerThreadgroup:threadgroupSize];
        [computeEncoder endEncoding];

        // Store command buffer for later commit
        commandBuffers.push_back(commandBuffer);

        // Update indices for next step (even though we don't use them in encoding,
        // we need to track final state)
        int temp = prevIdx;
        prevIdx = currentIdx;
        currentIdx = nextIdx;
        nextIdx = temp;
    }

    // PHASE 3: Commit all command buffers at once
    // GPU can now execute all sub-steps with maximum parallelism!
    for (auto& commandBuffer : commandBuffers) {
        [commandBuffer commit];
    }

    // PHASE 4: Wait only once for ALL sub-steps to complete
    // This is the ONLY synchronization point - massive performance win!
    for (auto& commandBuffer : commandBuffers) {
        [commandBuffer waitUntilCompleted];
    }

    // Copy final state back to CPU
    // After all sub-steps, currentIdx points to the final current state
    // and prevIdx points to the final previous state
    memcpy(finalPressure.data(), tripleBufferPtr + currentIdx * pImpl->gridSize, pImpl->gridSize * sizeof(float));
    memcpy(finalPressurePrev.data(), tripleBufferPtr + prevIdx * pImpl->gridSize, pImpl->gridSize * sizeof(float));

    // Copy listener samples back to CPU
    memcpy(listenerSamples.data(), [pImpl->listenerSamplesBuffer contents], numSubSteps * sizeof(float));

    // Update performance stats
    auto stepEnd = std::chrono::high_resolution_clock::now();
    pImpl->lastStepTimeMs = std::chrono::duration<double, std::milli>(stepEnd - pImpl->lastStepStart).count();
    pImpl->totalTimeMs += pImpl->lastStepTimeMs;
    pImpl->totalSteps++;
}

const std::string& MetalSimulationBackend::getLastError() const {
    return lastError;
}

MetalSimulationBackend::PerformanceStats MetalSimulationBackend::getPerformanceStats() const {
    PerformanceStats stats;
    stats.lastStepTimeMs = pImpl->lastStepTimeMs;
    stats.avgStepTimeMs = (pImpl->totalSteps > 0) ? (pImpl->totalTimeMs / pImpl->totalSteps) : 0.0;
    stats.totalSteps = pImpl->totalSteps;
    stats.totalTimeMs = pImpl->totalTimeMs;
    return stats;
}

void MetalSimulationBackend::resetPerformanceStats() {
    pImpl->lastStepTimeMs = 0.0;
    pImpl->totalTimeMs = 0.0;
    pImpl->totalSteps = 0;
}

#else // Not on Apple platform

// Stub implementation for non-Apple platforms
class MetalSimulationBackend::Impl {};

MetalSimulationBackend::MetalSimulationBackend() : pImpl(nullptr) {}
MetalSimulationBackend::~MetalSimulationBackend() {}

bool MetalSimulationBackend::initialize(int, int) {
    lastError = "Metal is only available on Apple platforms";
    return false;
}

bool MetalSimulationBackend::isAvailable() const {
    return false;
}

void MetalSimulationBackend::executeStep(
    const std::vector<float>&,
    const std::vector<float>&,
    std::vector<float>&,
    const std::vector<uint8_t>&,
    float, float, float)
{
    // No-op on non-Apple platforms
}

void MetalSimulationBackend::executeFrame(
    const std::vector<float>&,
    const std::vector<float>&,
    std::vector<float>&,
    std::vector<float>&,
    const std::vector<uint8_t>&,
    std::vector<float>&,
    const std::vector<std::vector<AudioSourceData>>&,
    int, int, int,
    float, float, float)
{
    // No-op on non-Apple platforms
}

const std::string& MetalSimulationBackend::getLastError() const {
    return lastError;
}

MetalSimulationBackend::PerformanceStats MetalSimulationBackend::getPerformanceStats() const {
    return {0.0, 0.0, 0, 0.0};
}

void MetalSimulationBackend::resetPerformanceStats() {}

#endif // __APPLE__
