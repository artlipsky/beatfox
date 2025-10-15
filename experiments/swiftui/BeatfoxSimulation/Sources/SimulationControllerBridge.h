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

// Audio info
@property (nonatomic, assign) BOOL isAudioInitialized;
@property (nonatomic, assign) BOOL isMuted;
@property (nonatomic, assign) float volume;
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
- (void)clearWaves;
- (void)setListenerPositionX:(int)x y:(int)y;
- (void)toggleListener;
- (void)setTimeScale:(float)scale;

// Audio operations
- (void)toggleMute;
- (void)setVolume:(float)volume;

// Acoustic presets (0=Realistic, 1=Visualization, 2=Anechoic)
- (void)applyDampingPreset:(int)presetType;

// Simulation update (call once per frame)
- (void)updateWithDeltaTime:(float)dt;

@end

NS_ASSUME_NONNULL_END
