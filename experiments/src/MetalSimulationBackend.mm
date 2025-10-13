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
 * MetalSimulationBackend Implementation
 *
 * Uses Metal compute shaders to accelerate wave propagation on GPU
 */
class MetalSimulationBackend::Impl {
public:
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
    id<MTLComputePipelineState> computePipeline;
    id<MTLLibrary> library;

    // GPU buffers
    id<MTLBuffer> pressureBuffer;
    id<MTLBuffer> pressurePrevBuffer;
    id<MTLBuffer> pressureNextBuffer;
    id<MTLBuffer> obstaclesBuffer;
    id<MTLBuffer> paramsBuffer;

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
        library(nil),
        pressureBuffer(nil),
        pressurePrevBuffer(nil),
        pressureNextBuffer(nil),
        obstaclesBuffer(nil),
        paramsBuffer(nil),
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
        computePipeline = nil;
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

    if (!pImpl->pressureBuffer || !pImpl->pressurePrevBuffer || !pImpl->pressureNextBuffer ||
        !pImpl->obstaclesBuffer || !pImpl->paramsBuffer) {
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

const std::string& MetalSimulationBackend::getLastError() const {
    return lastError;
}

MetalSimulationBackend::PerformanceStats MetalSimulationBackend::getPerformanceStats() const {
    return {0.0, 0.0, 0, 0.0};
}

void MetalSimulationBackend::resetPerformanceStats() {}

#endif // __APPLE__
