//
//  SimulationControllerBridge.h
//  BeatfoxSimulation
//
//  Objective-C++ bridge between Swift and C++ SimulationController
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// State struct exposed to Swift
@interface SimulationStateWrapper : NSObject
@property (nonatomic, assign) BOOL showHelp;
@property (nonatomic, assign) float timeScale;
@property (nonatomic, assign) BOOL obstacleMode;
@property (nonatomic, assign) BOOL listenerMode;
@property (nonatomic, assign) BOOL sourceMode;
@property (nonatomic, assign) int selectedPreset;
@property (nonatomic, assign) float sourceVolumeDb;
@property (nonatomic, assign) BOOL sourceLoop;
@property (nonatomic, assign) float impulsePressure;
@property (nonatomic, assign) int impulseRadius;

// Simulation info
@property (nonatomic, assign) int width;
@property (nonatomic, assign) int height;
@property (nonatomic, assign) float physicalWidth;
@property (nonatomic, assign) float physicalHeight;
@property (nonatomic, assign) float waveSpeed;
@property (nonatomic, assign) BOOL hasListener;
@property (nonatomic, assign) int listenerX;
@property (nonatomic, assign) int listenerY;
@property (nonatomic, assign) int numAudioSources;
@property (nonatomic, assign) int numObstacles;
@property (nonatomic, assign) int gridSpacing;
@property (nonatomic, assign) float pixelSize;
@property (nonatomic, assign) BOOL isGPUEnabled;
@property (nonatomic, assign) BOOL isGPUAvailable;
@property (nonatomic, assign) int obstacleRadius;

// Audio info
@property (nonatomic, assign) BOOL isAudioInitialized;
@property (nonatomic, assign) BOOL isMuted;
@property (nonatomic, assign) float volume;
@end

/// Audio source info for UI rendering
@interface AudioSourceInfo : NSObject
@property (nonatomic, assign) int x;
@property (nonatomic, assign) int y;
@property (nonatomic, assign) BOOL isPlaying;
@end

/// Bridge class that wraps SimulationController
@interface SimulationControllerBridge : NSObject

- (instancetype)init;
- (void)dealloc;

// State queries
- (SimulationStateWrapper *)getState;
- (void)updateState;

// UI Commands
- (void)setImpulsePressure:(float)pressure;
- (void)setImpulseRadius:(int)radius;
- (void)setShowHelp:(BOOL)show;
- (void)setSelectedPreset:(int)presetIndex;
- (void)setSourceVolumeDb:(float)volumeDb;
- (void)setSourceLoop:(BOOL)loop;

// Mode toggles
- (void)toggleHelp;
- (void)toggleObstacleMode;
- (void)toggleListenerMode;
- (void)toggleSourceMode;

// Domain operations
- (void)addImpulseAtX:(int)x y:(int)y pressure:(float)pressure radius:(int)radius;
- (void)addObstacleAtX:(int)x y:(int)y radius:(int)radius;
- (void)clearObstacles;
- (BOOL)loadObstaclesFromSVG:(NSString *)filePath;
- (void)clearWaves;
- (void)setListenerPositionX:(int)x y:(int)y;
- (void)toggleListener;
- (void)setTimeScale:(float)scale;

// Audio operations
- (void)toggleMute;
- (void)setVolume:(float)volume;
- (BOOL)loadAudioFile:(NSString *)filePath;
- (void)clearAudioSources;
- (void)addAudioSourceAtX:(int)x y:(int)y preset:(int)presetIndex volumeDb:(float)volumeDb loop:(BOOL)loop;

// Physics control
- (void)setWaveSpeed:(float)speed;
- (float)getDamping;
- (void)setAirAbsorption:(float)damping;

// GPU control
- (void)toggleGPU;
- (BOOL)isGPUEnabled;
- (BOOL)isGPUAvailable;

// Obstacle control
- (void)setObstacleRadius:(int)radius;
- (void)removeObstacleAtX:(int)x y:(int)y radius:(int)radius;

// Audio source control (for click-to-toggle)
- (void)toggleAudioSourcePlayback:(int)index;

// Acoustic presets (0=Realistic, 1=Visualization, 2=Anechoic)
- (void)applyDampingPreset:(int)presetType;

// Simulation update (call once per frame)
- (void)updateWithDeltaTime:(float)dt;

// Rendering data access
- (const float *)getPressureFieldData;
- (const uint8_t *)getObstacleFieldData;
- (NSArray<AudioSourceInfo *> *)getAudioSources;

@end

NS_ASSUME_NONNULL_END
