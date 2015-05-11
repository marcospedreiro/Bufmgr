// Implementation of lru.h
// Marcos Pedreiro, Jenny Zou | 10/17/16

#include "db.h"
#include "replacer.h"
#include "lru.h"

/** Constructor for LRU
	Input: None
	Output: None
*/
LRU::LRU() 
{
	this->lruVector;
}

/** Destructor for LRU
	Input: None
	Output: None
*/
LRU::~LRU() 
{
	this->lruVector.clear();
}

/** Returns true if LRU is empty
	Input: None
	Output: None
*/
bool LRU::isEmpty() 
{
	return this->lruVector.empty();
}

/** Returns true if frame exists inside the LRU
	false otherwise
	Input: frame
	Output: bool
*/
bool LRU::exists(int frame)
{
	return std::find(this->lruVector.begin(), this->lruVector.end(), frame) != this->lruVector.end();
}

/** Chooses a frame to have contents replaced 
	Returns -1 if LRU is empty
	Input: None
	Output: int (frameNumber)
*/
int LRU::PickVictim()
{
	if(this->isEmpty()) {
		return -1;
	}

	// remove head of vector
	int frameNum = this->lruVector.at(0);
	this->lruVector.erase(this->lruVector.begin());
	return frameNum;

}

/** Add a frame to the LRU
	Input: Frame number to add
	Output: None
*/
void LRU::AddFrame(int f)
{
	// add frame to end of vector
	this->lruVector.push_back(f);
}

/** Remove the first frame whose value is equal to f from LRU
	Does nothing if frame not found
	Input: Frame number to remove
	Output: None
*/
void LRU::RemoveFrame(int f)
{
	std::vector<int >::iterator position = std::find(this->lruVector.begin(), this->lruVector.end(), f);

	// if position == end(), element not found
	if(position != this->lruVector.end()) {
		this->lruVector.erase(position);
	}
}