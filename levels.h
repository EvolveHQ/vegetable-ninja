// Level definitions for the saga structure.
// All 20 stages live in the table in levels.c; gameplay code consumes a
// LevelDef generically and never branches on a concrete level id.
#ifndef LEVELS_H
#define LEVELS_H

typedef enum {
    OBJ_SLICE_COUNT,   // slice objTarget vegetables before timeLimit
    OBJ_SURVIVE_TIME,  // survive timeLimit with lives remaining
    OBJ_SCORE_TARGET,  // reach objTarget score before timeLimit
    OBJ_NO_BOMB,       // reach timeLimit without slicing a bomb
    OBJ_COMBO          // land a combo of objTarget in one swipe before timeLimit
} ObjectiveType;

typedef enum {
    METRIC_SCORE,      // star thresholds grade the run's score
    METRIC_ACCURACY    // star thresholds grade sliced/spawned (bombs excluded)
} StarMetric;

// Bits match the VegType enum order in main.c.
#define VEGBIT_CARROT   0x01u
#define VEGBIT_TOMATO   0x02u
#define VEGBIT_BROCCOLI 0x04u
#define VEGBIT_EGGPLANT 0x08u
#define VEGBIT_CORN     0x10u
#define VEGBIT_PUMPKIN  0x20u
#define VEGBIT_ONION    0x40u
#define VEGBIT_CUCUMBER 0x80u
#define VEGBIT_ALL      0xFFu

typedef struct {
    int   id;             // world*100 + stage (101..110, 201..210)
    int   world, stage;   // 1-based
    const char *name;     // display name shown on map/HUD
    ObjectiveType objective;
    int   objTarget;      // slices / score / combo size (unused: survive, no-bomb)
    float timeLimit;      // stage's bounded window, seconds
    float spawnInterval;  // base seconds between waves at stage start
    float spawnRamp;      // interval multiplier reached by stage end (1.0 = flat)
    int   maxWave;        // max entities per wave
    unsigned vegMask;     // allowed vegetable types
    float bombChance;     // probability per spawned entity
    int   lives;          // allowed misses
    StarMetric starMetric;
    float star2, star3;   // thresholds (score points, or accuracy 0..1)
} LevelDef;

// Fixed capacity shared by consumers that size per-level arrays statically
// (progress module); levels.c static-asserts the table fits.
#define LEVELS_MAX_COUNT 64

int             Levels_Count(void);
const LevelDef *Levels_ByIndex(int index);       // NULL if out of range
const LevelDef *Levels_ById(int id);             // NULL if unknown
int             Levels_IndexOfId(int id);        // -1 if unknown
int             Levels_FirstInvalid(void);       // -1 if every row is sane

#endif // LEVELS_H
