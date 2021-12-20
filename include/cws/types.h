#ifndef CWS_TYPES_H_
#define CWS_TYPES_H_

#define CWS_ENTRY_FLAG_SET 0x01
#define CWS_ENTRY_FLAG_ACTIVE 0x10

typedef double cwsVariable;
typedef double cwsVector[3];

struct cwsGridEntry {
	cwsVariable pressure; //in pa (=mbar/100)
	cwsVariable temperature; //in K, together with the pressure we can calculate our air density
	cwsVariable lwc; //liquid water content in kg
	cwsVariable wvc; //water vapour content in kg
	cwsVector wind; //each component in m/s

	//array offsets
	unsigned int plusXAdjacentEntry;
	unsigned int minusXAdjacentEntry;
	unsigned int plusYAdjacentEntry;
	unsigned int minusYAdjacentEntry;
	unsigned int plusZAdjacentEntry;
	unsigned int minusZAdjacentEntry;

	int x;
	int y;
	int z;

	char flags;
};

#endif
