#ifndef _MRU_H
#define _MRU_H

#include "db.h"

#include "replacer.h"
#include <vector>
#include <algorithm>

// MRU Buffer Replacement
class MRU : public Replacer {
private:

	std::vector<int > mruVector;

	bool isEmpty();
	bool exists(int frame);

public:
	MRU();
	virtual ~MRU();

	virtual int PickVictim();
	virtual void AddFrame(int f);
	virtual void RemoveFrame(int f);
};

#endif // MRU