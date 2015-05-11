// Implementation of mru.h
// Marcos Pedreiro, Jenny Zou | 10/17/16

#include "db.h"
#include "replacer.h"
#include "mru.h"

MRU::MRU() 
{
	this->mruVector; 
}

/** Destructor for MRU
	Input: None
	Output: None
*/
MRU::~MRU() 
{
	this->mruVector.clear();
}

/** Returns true if MRU is empty
	Input: None
	Output: None
*/
bool MRU::isEmpty() 
{
	return this->mruVector.empty();
}

/** Returns true if frame exists inside the MRU
	false otherwise
	Input: frame
	Output: bool
*/
bool MRU::exists(int frame)
{
	return std::find(this->mruVector.begin(), this->mruVector.end(), frame) != this->mruVector.end();
}

/** Chooses a frame to have contents replaced 
	Returns -1 if MRU is empty
	Input: None
	Output: int (frameNumber)
*/
int MRU::PickVictim()
{
	if(this->isEmpty()) {
		return -1;
	}

	// remove tail of vector
	int frameNum = this->mruVector.at(this->mruVector.size()-1);
	this->mruVector.pop_back();
	return frameNum;

}

/** Add a frame to the MRU
	Input: Frame number to add
	Output: None
	NOTE: Differs from lru: adds frame to head of list, rather than tail
*/
void MRU::AddFrame(int f)
{
	// add frame to end of vector
	this->mruVector.push_back(f);
}

/** Remove the first frame whose value is equal to f from MRU
	Does nothing if frame not found
	Input: Frame number to remove
	Output: None
*/
void MRU::RemoveFrame(int f)
{
	std::vector<int >::iterator position = std::find(this->mruVector.begin(), this->mruVector.end(), f);

	// if position == end(), element not found
	if(position != this->mruVector.end()) {
		this->mruVector.erase(position);
	}

}













