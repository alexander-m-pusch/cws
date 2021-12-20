#ifndef CWS_CWS_H_
#define CWS_CWS_H_
//header guards are cool i guess

#include <stdlib.h>
#include <limits.h>

struct cwsSettings {
	unsigned int minimumBufferSize;
	unsigned int maximumBufferSize;
	unsigned int cacheSize;
};

void cwsInitialize(struct cwsSettings* settings);

static inline struct cwsSettings* cwsDefaultSettings(void) {
	struct cwsSettings* defs = malloc(sizeof(struct cwsSettings));
	
	defs->minimumBufferSize = 512;
	defs->maximumBufferSize = UINT_MAX;
	defs->cacheSize = 10;

	return defs;
}

#endif
