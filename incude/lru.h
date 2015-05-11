#ifndef _LRU_H
#define _LRU_H

#include "db.h"

#include "replacer.h"
#include <vector>
#include <algorithm>

// LRU Buffer Replacement
class LRU : public Replacer {
private:

	std::vector<int > lruVector;

	bool isEmpty();
	bool exists(int frame);
	
public:
	LRU();
	virtual ~LRU();

	virtual int PickVictim();
	virtual void AddFrame(int f);
	virtual void RemoveFrame(int f); 
};

#endif // LRU