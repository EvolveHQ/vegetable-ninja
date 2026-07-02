// The single data-driven level table. One row per stage; the difficulty
// curve (spawn/bomb/target ramps, boss stages 110 and 210) is expressed
// entirely in this data.
#include "levels.h"
#include <stddef.h>

// convenience masks — variety grows across World 1
#define VEG3 (VEGBIT_CARROT | VEGBIT_TOMATO | VEGBIT_CUCUMBER)
#define VEG4 (VEG3 | VEGBIT_BROCCOLI)
#define VEG5 (VEG4 | VEGBIT_CORN)
#define VEG6 (VEG5 | VEGBIT_PUMPKIN)
#define VEG7 (VEG6 | VEGBIT_ONION)
#define VEG8 VEGBIT_ALL

static const LevelDef LEVELS[] = {
    // id, world, stage, name,               objective,        target, time, spawn, ramp, wave, veg,  bomb,  lives, metric,          2*,   3*
    { 101, 1,  1, "First Chops",         OBJ_SLICE_COUNT,   10, 45, 1.60f, 1.00f, 2, VEG3, 0.00f, 3, METRIC_ACCURACY, 0.70f, 0.90f },
    { 102, 1,  2, "Warm-Up Salad",       OBJ_SLICE_COUNT,   15, 45, 1.50f, 1.00f, 2, VEG4, 0.02f, 3, METRIC_ACCURACY, 0.70f, 0.90f },
    { 103, 1,  3, "Keep Chopping",       OBJ_SURVIVE_TIME,   0, 30, 1.40f, 1.00f, 3, VEG4, 0.04f, 3, METRIC_ACCURACY, 0.60f, 0.85f },
    { 104, 1,  4, "Score Starter",       OBJ_SCORE_TARGET, 300, 50, 1.40f, 1.00f, 3, VEG5, 0.05f, 3, METRIC_SCORE,   400.f, 500.f },
    { 105, 1,  5, "Twin Blades",         OBJ_COMBO,          3, 40, 1.30f, 1.00f, 3, VEG5, 0.05f, 3, METRIC_ACCURACY, 0.65f, 0.85f },
    { 106, 1,  6, "Mind the Bomb",       OBJ_NO_BOMB,        0, 30, 1.30f, 1.00f, 3, VEG5, 0.10f, 3, METRIC_ACCURACY, 0.60f, 0.85f },
    { 107, 1,  7, "Veggie Flurry",       OBJ_SLICE_COUNT,   30, 50, 1.10f, 1.00f, 3, VEG6, 0.07f, 3, METRIC_ACCURACY, 0.70f, 0.88f },
    { 108, 1,  8, "High Steaks",         OBJ_SCORE_TARGET, 600, 55, 1.10f, 1.00f, 4, VEG6, 0.08f, 3, METRIC_SCORE,   750.f, 900.f },
    { 109, 1,  9, "Chain Reaction",      OBJ_COMBO,          4, 45, 1.00f, 1.00f, 4, VEG7, 0.08f, 3, METRIC_ACCURACY, 0.70f, 0.88f },
    { 110, 1, 10, "Garden Gauntlet",     OBJ_SURVIVE_TIME,   0, 60, 0.90f, 0.80f, 4, VEG8, 0.12f, 3, METRIC_ACCURACY, 0.65f, 0.85f },

    { 201, 2,  1, "Second Course",       OBJ_SLICE_COUNT,   35, 50, 1.00f, 1.00f, 4, VEG6, 0.10f, 3, METRIC_ACCURACY, 0.70f, 0.88f },
    { 202, 2,  2, "Bomb Squad",          OBJ_NO_BOMB,        0, 40, 0.95f, 1.00f, 4, VEG6, 0.14f, 3, METRIC_ACCURACY, 0.65f, 0.85f },
    { 203, 2,  3, "Point Harvest",       OBJ_SCORE_TARGET, 900, 55, 0.95f, 1.00f, 4, VEG7, 0.10f, 3, METRIC_SCORE,  1100.f, 1300.f },
    { 204, 2,  4, "Fast Fingers",        OBJ_COMBO,          5, 45, 0.90f, 1.00f, 5, VEG7, 0.10f, 3, METRIC_ACCURACY, 0.70f, 0.88f },
    { 205, 2,  5, "Marathon Muncher",    OBJ_SURVIVE_TIME,   0, 50, 0.90f, 0.85f, 4, VEG7, 0.12f, 3, METRIC_ACCURACY, 0.60f, 0.82f },
    { 206, 2,  6, "Chop till you Drop",  OBJ_SLICE_COUNT,   50, 60, 0.85f, 1.00f, 5, VEG8, 0.12f, 3, METRIC_ACCURACY, 0.72f, 0.90f },
    { 207, 2,  7, "Minefield",           OBJ_NO_BOMB,        0, 45, 0.85f, 1.00f, 5, VEG8, 0.18f, 3, METRIC_ACCURACY, 0.65f, 0.85f },
    { 208, 2,  8, "Grand Score",         OBJ_SCORE_TARGET,1400, 60, 0.80f, 1.00f, 5, VEG8, 0.12f, 3, METRIC_SCORE,  1700.f, 2000.f },
    { 209, 2,  9, "Combo Cyclone",       OBJ_COMBO,          6, 50, 0.75f, 1.00f, 6, VEG8, 0.12f, 3, METRIC_ACCURACY, 0.72f, 0.90f },
    { 210, 2, 10, "The Full Harvest",    OBJ_SCORE_TARGET,1800, 75, 0.70f, 0.75f, 6, VEG8, 0.20f, 3, METRIC_SCORE,  2200.f, 2600.f },
};

#define LEVEL_COUNT ((int)(sizeof(LEVELS)/sizeof(LEVELS[0])))

int Levels_Count(void) { return LEVEL_COUNT; }

const LevelDef *Levels_ByIndex(int index) {
    if (index < 0 || index >= LEVEL_COUNT) return NULL;
    return &LEVELS[index];
}

int Levels_IndexOfId(int id) {
    for (int i = 0; i < LEVEL_COUNT; i++)
        if (LEVELS[i].id == id) return i;
    return -1;
}

const LevelDef *Levels_ById(int id) {
    int i = Levels_IndexOfId(id);
    return i < 0 ? NULL : &LEVELS[i];
}
