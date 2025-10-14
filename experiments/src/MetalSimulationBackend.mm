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
    id<MTLBuffer> multiStepParamsBuffer; // Multi-step parameters
    id<MTLBuffer> listenerSamplesBuffer; // Listener samples output

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

    if (!pImpl->pressureBuffer || !pImpl->pressurePrevBuffer || !pImpl->pressureNextBuffer ||
        !pImpl->obstaclesBuffer || !pImpl->paramsBuffer ||
        !pImpl->tripleBuffer || !pImpl->multiStepParamsBuffer || !pImpl->listenerSamplesBuffer) {
        lastError = "Failed to create Metal buffers";
        std::cerr << "MetalBackend: " << lastError << std::endl;
        return false;
    }

    std::cout << "MetalBackend: Initialized successfully" << std::endl;
    std::cout << "MetalBackend: Grid size: " << width << "x" << height << " (" << pImpl->gridSize << " cells)" << std::endl;
    std::cout << "MetalBackend: Buffer size: " << (bufferSize / 1024.0f / 1024.0f) << " MB per field" << std::endl;

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
    int listenerX,
    int listenerY,
    int numSubSteps,
    float c2_dt2_dx2,
    float damping,
    float wallReflection)
{
    if (!isAvailable()) {
        return;
    }

    pImpl->lastStepStart = std::chrono::high_resolution_clock::now();

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

    // Execute all sub-steps on GPU
    // Triple buffering rotation: (current, prev, next) → (next, current, prev)
    int currentIdx = 0;  // Initial current
    int prevIdx = 1;     // Initial previous
    int nextIdx = 2;     // Initial next

    for (int step = 0; step < numSubSteps; step++) {
        // Set up multi-step parameters
        MultiStepParams* params = (MultiStepParams*)[pImpl->multiStepParamsBuffer contents];
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

        // Create command buffer for this step
        id<MTLCommandBuffer> commandBuffer = [pImpl->commandQueue commandBuffer];

        // Create compute encoder
        id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];

        // Set compute pipeline and buffers
        [computeEncoder setComputePipelineState:pImpl->multiStepPipeline];
        [computeEncoder setBuffer:pImpl->tripleBuffer offset:0 atIndex:0];
        [computeEncoder setBuffer:pImpl->obstaclesBuffer offset:0 atIndex:1];
        [computeEncoder setBuffer:pImpl->listenerSamplesBuffer offset:0 atIndex:2];
        [computeEncoder setBuffer:pImpl->multiStepParamsBuffer offset:0 atIndex:3];

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

        // Commit and wait for completion (ensures each step sees correct buffer indices)
        [commandBuffer commit];
        [commandBuffer waitUntilCompleted];

        // Rotate buffer indices for next iteration
        // (current, prev, next) → (next, current, prev)
        int temp = prevIdx;
        prevIdx = currentIdx;
        currentIdx = nextIdx;
        nextIdx = temp;
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
