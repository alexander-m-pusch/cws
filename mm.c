//CWS memory manager
//written by Alexander Pusch
//Copyright (C) 2021
//Available under the GNU General Public License version 3.

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include <cws/mm.h>
#include <cws/types.h>

static struct cwsGridEntry* bufferA;
static struct cwsGridEntry* bufferB;

static unsigned int bufferSize;
static unsigned int minBufferSize;
static unsigned int maxBufferSize;
static unsigned int bufferPointer;

static unsigned int garbageCollectionStep;
static unsigned int garbageCollected;

//internal function
static unsigned int cwsGetIndexForEntry(int x, int y, int z) {
	for(int i = 0; i < bufferSize; i++) {
		if(bufferA[i].x == x && bufferA[i].y == y && bufferA[i].z == z) return i;
	}
	return UINT_MAX; //this is a magic number
}

void cwsGridEntryTick(void) {
	//TODO how this will work:
	//generally create a threadpool and submit the function that does the meteorological number crunching
	//the function that does the magic takes the i-th gridentry from bufferA and writes the processed results
	//into bufferB[i], so the order is preserver
	//finally, swap bufferA and bufferB so bufferA is always the "main" one and bufferB is only really used
	//to swap to 

	struct cwsGridEntry* temp = bufferA;
	bufferA = bufferB;
	bufferB = temp;
}

struct cwsGridEntry* cwsGetGridEntryAt(int x, int y, int z) {
	unsigned int index = cwsGetIndexForEntry(x, y, z);

	if(index != UINT_MAX) return &bufferA[index];

	return 0;
}

struct cwsGridEntry* cwsGetGridEntryForIndex(unsigned int index) {
	if(index > bufferSize) return 0; //so we don't do nonsense
	return &bufferA[index];
}

unsigned int cwsAllocateNewGridEntry(struct cwsGridEntry* base, int xOffset, int yOffset, int zOffset, char flags) {
	unsigned int index = cwsGetIndexForEntry(base->x + xOffset, base->y + yOffset, base->z + zOffset);
	
	if(index != UINT_MAX) return index;
	
	//if not, first check if we have allocated space that we can use
	
	if(bufferPointer < bufferSize) {
		bufferA[bufferSize - 1] = *base;
		bufferA[bufferPointer].x += xOffset;
		bufferA[bufferPointer].y += yOffset;
		bufferA[bufferPointer].z += zOffset;
		bufferA[bufferPointer].flags = flags;
		
		return bufferPointer;
	}

	//if not, grow the buffer

	bufferSize++;
	if(bufferSize > maxBufferSize) {
		fprintf(stderr, "Buffer exceeds the maximum buffer space!\n");
		exit(-1);
	}

	bufferA = realloc(bufferA, bufferSize * sizeof(struct cwsGridEntry));
	bufferB = realloc(bufferB, bufferSize * sizeof(struct cwsGridEntry));
	if(!bufferA || !bufferB) {
		fprintf(stderr, "Ran out of memory, or realloc failed for another reason\n");
		exit(-1);
	}
	bufferA[bufferSize - 1] = *base;
	bufferA[bufferSize - 1].x += xOffset;
	bufferA[bufferSize - 1].y += yOffset;
	bufferA[bufferSize - 1].z += zOffset;
	bufferA[bufferSize - 1].flags = flags;

	return bufferSize;
}

static unsigned int bufferSizePre;
void cwsGarbageCollection(void) {
	if(bufferPointer < minBufferSize) return; //why bother collect the garbage if there is none to collect?
	if(bufferSizePre < bufferSize) { //sanity check: in case the array we are trying to garbage-collect is modified in the meantime, start over
		garbageCollectionStep = 2;
	}
	if(garbageCollectionStep == 2) {
		bufferSizePre = bufferSize;
	}
	garbageCollected = 0;
	
	for(unsigned int baseIndex = 0; baseIndex < bufferSizePre; baseIndex += garbageCollectionStep) {
		unsigned int pivotIndex = baseIndex + (garbageCollectionStep / 2);
		
		//first: count how many elements before the pivot are used
		unsigned int uppermostOccupied = baseIndex;
		for(int i = baseIndex; i < baseIndex + pivotIndex; i++) {
			uppermostOccupied++;
			if(bufferA[i].flags == 0x00) break; //I hate using break in nested loops but if I use goto and a label here, the no-goto crowd will lynch me, I can guarantee you
		}

		//second: calculate how many indices the elements right of the pivot can be moved left before they override existing elements
		unsigned int freeIndices = (baseIndex + pivotIndex) - uppermostOccupied;
		
		//third: move elements right of the pivot n indices to the left and update references
		if(freeIndices) { //for performance reasons, let's check if we actually need to. In case we don't, ignore because memory access is slow
			for(int i = baseIndex + pivotIndex; i < baseIndex + garbageCollectionStep; i++) {
				if(bufferA[i].flags == 0x00) break; //we have reached of the elements to process
				bufferA[i - freeIndices] = bufferA[i];
				bufferA[bufferA[i - freeIndices].plusXAdjacentEntry].minusXAdjacentEntry = i - freeIndices;
				bufferA[bufferA[i - freeIndices].minusXAdjacentEntry].plusXAdjacentEntry = i - freeIndices;
				bufferA[bufferA[i - freeIndices].plusYAdjacentEntry].minusYAdjacentEntry = i - freeIndices;
				bufferA[bufferA[i - freeIndices].minusYAdjacentEntry].plusYAdjacentEntry = i - freeIndices;
				bufferA[bufferA[i - freeIndices].plusZAdjacentEntry].minusZAdjacentEntry = i - freeIndices;
				bufferA[bufferA[i - freeIndices].minusZAdjacentEntry].plusZAdjacentEntry = i - freeIndices;
			}
		}
	}
	
	garbageCollectionStep *= 2;

	if(garbageCollectionStep > bufferSizePre) {
	       	garbageCollected = 1;
		garbageCollectionStep = 2;
		//now is the time for the glorious realloc
		
		unsigned int newBufferSize;
		for(unsigned int i = 0; i < bufferSizePre; i++) {
			newBufferSize++;
			if(bufferA[i].flags == 0x00) break; //after the first element that has the NULL flag, we are guaranteed to have all in-use blocks below index i 
		}

		bufferPointer = newBufferSize;
		if(newBufferSize < minBufferSize) newBufferSize = minBufferSize;
		
		bufferA = realloc(bufferA, newBufferSize * sizeof(struct cwsGridEntry));
		bufferB = realloc(bufferB, newBufferSize * sizeof(struct cwsGridEntry));
	}
}

void cwsCompleteGarbageCollection(void) {
	do {
		cwsGarbageCollection();
	} while(!garbageCollected);
}

void cwsInitializeMemoryManager(unsigned int min, unsigned int max, unsigned int cacheSize) {

	bufferPointer = 0;
	bufferSize = min;
	maxBufferSize = max;
	minBufferSize = min;
	//I use calloc here because I require the "flags" element of the structs to be initialized to zero, otherwise the garbage collection will produce undefined behaviour (it will most likely consider the array elements from 0 to minBufferSize - 1 as occupied instead of free, which is undesirable)
	bufferA = calloc(sizeof(struct cwsGridEntry), minBufferSize);
	bufferB = calloc(sizeof(struct cwsGridEntry), minBufferSize);
	garbageCollectionStep = 2;
	garbageCollected = 0;

	if(!bufferA || !bufferB) {
		fprintf(stderr, "Failed to create buffers! Do we have enough memory?\n");
		exit(-1);
	}
}

void cwsCleanUp(void) {
	free(bufferA);
	free(bufferB);
}
