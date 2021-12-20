#ifndef CWS_MM_H_
#define CWS_MM_H_

#include <cws/types.h>

/**
 * Ticks all the grid entries. 
 */
void cwsGridEntryTick(void);

/**
 * Returns zero if the cwsGridEntry at position is not found in the buffer. So check for NULL, otherwise you will get
 * nasty suprises derefencing!
 */
struct cwsGridEntry* cwsGetGridEntryAt(int x, int y, int z);

/**
 * Returns the pointer to the cwsGridEntry at index in the entry buffer. If you provide an invalid index, this function
 * will complain to stderr and return a null pointer.
 */
struct cwsGridEntry* cwsGetGridEntryForIndex(unsigned int index);

/**
 * Returns the index for the given position. If none exists, allocates a new cwsGridEntry and returns the index in the array.
 */
unsigned int cwsAllocateNewGridEntry(struct cwsGridEntry* base, int xOffset, int yOffset, int zOffset, char flags);

/**
 * This function will perform the next step of the garbage collection. The intricacies of the function will not
 * be explained here as they are not relevant to the person using the API. If, for whatever reason, you need the
 * garbage collection to run at once instead of incrementally, you will need to use cwsCompleteGarbageCollection().
 */
void cwsGarbageCollection(void);

/**
 * This function is similar to cwsGarbageCollection(), but runs at once instead of letting the library break up the
 * garbage collection process. This is slow and discouraged, but may be needed in certain situations. Use at your 
 * own risk.
 */
void cwsCompleteGarbageCollection(void);

/**
 * Don't call this manually, this is invoked by cwsInitialize().
 */
void cwsInitializeMemoryManager(unsigned int min, unsigned int max, unsigned int cacheSize);

/**
 * Call this when you are done, otherwise you will have memory leaks (which are not fun at all.)
 */
void cwsCleanUp(void);

#endif
