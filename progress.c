#include "progress.h"
#include "levels.h"

#define MAX_LEVELS LEVELS_MAX_COUNT   // levels.c static-asserts the table fits

static bool sUnlocked[MAX_LEVELS];
static int  sBestStars[MAX_LEVELS];
static bool sInitialized = false;

static void EnsureInit(void) {
    if (!sInitialized) Progress_Reset();
}

void Progress_Reset(void) {
    for (int i = 0; i < MAX_LEVELS; i++) { sUnlocked[i] = false; sBestStars[i] = 0; }
    if (Levels_Count() > 0) sUnlocked[0] = true;   // only the first stage
    sInitialized = true;
}

ProgressView Progress_Get(void) {
    EnsureInit();
    ProgressView v = { Levels_Count(), sUnlocked, sBestStars };
    return v;
}

void Progress_CompleteLevel(int id, int stars) {
    EnsureInit();
    int i = Levels_IndexOfId(id);
    if (i < 0) return;
    if (stars < 1) stars = 1;
    if (stars > 3) stars = 3;
    if (stars > sBestStars[i]) sBestStars[i] = stars;   // monotonic best
    sUnlocked[i] = true;
    if (i + 1 < Levels_Count()) sUnlocked[i + 1] = true; // linear unlock
}

bool Progress_IsUnlocked(int id) {
    EnsureInit();
    int i = Levels_IndexOfId(id);
    return i >= 0 && sUnlocked[i];
}

int Progress_BestStars(int id) {
    EnsureInit();
    int i = Levels_IndexOfId(id);
    return i < 0 ? 0 : sBestStars[i];
}
