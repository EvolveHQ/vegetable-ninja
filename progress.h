// Session progression: unlock flags and best stars per stage.
// In-memory only by design — no storage API is used. This interface is
// the seam where a persistence backend can be swapped in later without
// touching callers.
#ifndef PROGRESS_H
#define PROGRESS_H

#include <stdbool.h>

typedef struct {
    int levelCount;
    const bool *unlocked;    // by level index (Levels_ByIndex order)
    const int  *bestStars;   // 0..3 by level index; 0 = never completed
} ProgressView;

void         Progress_Reset(void);                  // fresh session state
ProgressView Progress_Get(void);                    // read-only snapshot
void         Progress_CompleteLevel(int id, int stars); // record + unlock next
bool         Progress_IsUnlocked(int id);
int          Progress_BestStars(int id);

#endif // PROGRESS_H
