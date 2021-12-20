#include <stdio.h>

#include <cws/cws.h>
#include <cws/mm.h>

static struct cwsSettings* globalSettings;

void cwsInitialize(struct cwsSettings* settings) {
	globalSettings = settings;
	cwsInitializeMemoryManager(settings->minimumBufferSize, settings->maximumBufferSize, settings->cacheSize);
}
