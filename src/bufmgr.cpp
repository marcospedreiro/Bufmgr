
#include "bufmgr.h"
#include "frame.h"
#include "lru.h"
#include "mru.h"

//--------------------------------------------------------------------
// Constructor for BufMgr
//
// Input   : bufSize  - number of frames(pages) in the this buffer manager
//           replacementPolicy - a replacement policy, either LRU or MRU            
// Output  : None
// PostCond: All frames are empty.
//           the "replacer" is initiated to LRU or MRU according to the
//           replacement policy.
//--------------------------------------------------------------------
BufMgr::BufMgr(int bufSize, const char* replacementPolicy)
{
	//TODO: add your code here
	// set stats
	this->ResetStat();
	this->numFrames = bufSize;

	// set replacement policy
	if(replacementPolicy[0] == 'L') {
		this->replacer = new LRU();
	} else {// if(replacementPolicy[0] == 'M') {
		this->replacer = new MRU();
	} 

	// allocate and empty frames in memory
	this->frames = new Frame[bufSize];
	for(int k=0; k < bufSize; k++) {
		this->frames[k].EmptyIt();
	}

}


//--------------------------------------------------------------------
// Destructor for BufMgr
//
// Input   : None
// Output  : None
//--------------------------------------------------------------------
BufMgr::~BufMgr()
{   
	//TODO: add your code here

	// Write all dirty pages to disk
	this->FlushAllPages();

	// clean up
	this->numFrames = 0;
	delete [] this->frames;
		this->frames = NULL;
	delete this->replacer;
		this->replacer = NULL;
}

//--------------------------------------------------------------------
// BufMgr::PinPage
//
// Input    : pid     - page id of a particular page 
//            isEmpty - (optional, default to false) if true indicate
//                      that the page to be pinned is an empty page.
// Output   : page - a pointer to a page in the buffer pool. (NULL
//            if fail)
// Purpose  : Pin the page with page id = pid to the buffer.  
//            Read the page from disk unless isEmpty is true or unless
//            the page is already in the buffer.
// Condition: Either the page is already in the buffer, or there is at
//            least one frame available in the buffer pool for the 
//            page.
// PostCond : The page with page id = pid resides in the buffer and 
//            is pinned. The number of pin on the page increase by
//            one.
// Return   : OK if operation is successful.  FAIL otherwise.
//--------------------------------------------------------------------
Status BufMgr::PinPage(PageID pid, Page*& page, bool isEmpty)
{
	int frameNum = this->FindFrame(pid);

	// check if frame is pinned in buffer
	if(frameNum == INVALID_PAGE) { 
		// not in buffer
		// check if there's space for new page
		if(this->GetNumOfUnpinnedFrames() == 0) {
			// no space
			page = NULL;
			return FAIL;
		}
		
		/*// find frame | REPLACED BY getAvailableFrame()
		int iter = 0;
		bool foundEmptyFrame = false;
		while(iter<this->numFrames && !foundEmptyFrame) {
			if(!this->frames[iter].IsValid()) {
				foundEmptyFrame = true;
				frameNum = iter;
			}
			iter += 1;
		}
				
		if(!foundEmptyFrame) { // pick victim
			frameNum = this->replacer->PickVictim();
			Status pageFlush = this->FlushPage(this->frames[frameNum].GetPageID());
			if(pageFlush == OK){
				this->frames[frameNum].EmptyIt();
			} else {
				frameNum = -1;
			}
		} */
		frameNum = this->getAvailableFrame();
		this->frames[frameNum].Read(pid, isEmpty);
	} else {
		// frame is in buffer
		this->totalHit += 1;
	}

	page = this->frames[frameNum].GetPage(); 
	this->frames[frameNum].Pin();
	// If first pin, need to account for it in replacer
	if (this->frames[frameNum].GetPinCount() == 1) { 
		this->replacer->RemoveFrame(frameNum);
	}

	this->totalCall += 1;
	return OK;
} 

//--------------------------------------------------------------------
// BufMgr::UnpinPage
//
// Input    : pid     - page id of a particular page 
//            dirty   - indicate whether the page with page id = pid
//                      is dirty or not. (Optional, default to false)
// Output   : None
// Purpose  : Unpin the page with page id = pid in the buffer. Mark 
//            the page dirty if dirty is true.  
// Condition: The page is already in the buffer and is pinned.
// PostCond : The page is unpinned and the number of pin on the
//            page decrease by one. 
// Return   : OK if operation is successful.  FAIL otherwise.
//--------------------------------------------------------------------
Status BufMgr::UnpinPage(PageID pid, bool dirty)
{
	//TODO: add your code here
	Status pageUnpinned = FAIL;

	int frameNum = this->FindFrame(pid);

	if((frameNum != INVALID_FRAME) && (this->frames[frameNum].GetPinCount() > 0)) {
		this->frames[frameNum].Unpin();
		if(dirty) {
			this->frames[frameNum].DirtyIt();
		}

		// If pin count is 0, need to account for it in replacer
		if(this->frames[frameNum].GetPinCount() == 0) {
			this->replacer->RemoveFrame(frameNum);
			this->replacer->AddFrame(frameNum);
		}

		pageUnpinned = OK;
	}

	return pageUnpinned; 
}

//--------------------------------------------------------------------
// BufMgr::NewPage
//
// Input    : howMany - (optional, default to 1) how many pages to 
//                      allocate.
// Output   : firstPid  - the page id of the first page (as output by
//                   DB::AllocatePage) allocated.
//            firstPage - a pointer to the page in memory.
// Purpose  : Allocate howMany number of pages, and pin the first page
//            into the buffer. 
// Condition: howMany > 0 and there is at least one free buffer space
//            to hold a page.
// PostCond : The page with page id = pid is pinned into the buffer.
// Return   : OK if operation is successful.  FAIL otherwise.
// Note     : You can call DB::AllocatePage() to allocate a page.  
//            You should call DB:DeallocatePage() to deallocate the
//            pages you allocate if you failed to pin the page in the
//            buffer.
//--------------------------------------------------------------------
Status BufMgr::NewPage (PageID& firstPid, Page*& firstPage, int howMany)
{
	
	//TODO: add your code here
	if (howMany < 1 || this->GetNumOfUnpinnedFrames() < 1) {
		return FAIL;
	}

	MINIBASE_DB->AllocatePage(firstPid, howMany);

	/*// find free frame | REPLACED BY getAvailableFrame()
	int iter = 0;
	int freeFrameIndex = -1;
	bool foundEmptyFrame = false;
	while(iter<this->numFrames && !foundEmptyFrame) {
		if(!this->frames[iter].IsValid()) {
			freeFrameIndex = iter;
			foundEmptyFrame = true;
		}
		iter += 1;
	}

	if(!foundEmptyFrame) {
		freeFrameIndex = this->replacer->PickVictim();
		this->frames[freeFrameIndex].EmptyIt();
	} */

	int freeFrameIndex = this->getAvailableFrame();

	if(freeFrameIndex == -1) {
		return FAIL;
	}

	this->frames[freeFrameIndex].Read(firstPid, true); 
	firstPage = this->frames[freeFrameIndex].GetPage();

	this->frames[freeFrameIndex].Pin();

	return OK; 

}

//--------------------------------------------------------------------
// BufMgr::FreePage
//
// Input    : pid     - page id of a particular page 
// Output   : None
// Purpose  : Free the memory allocated for the page with 
//            page id = pid  
// Condition: Either the page is already in the buffer and is pinned
//            no more than once, or the page is not in the buffer.
// PostCond : The page is unpinned, and the frame where it resides in
//            the buffer pool is freed.  Also the page is deallocated
//            from the database. 
// Return   : OK if operation is successful.  FAIL otherwise.
// Note     : You can call MINIBASE_DB->DeallocatePage(pid) to
//            deallocate a page.
//--------------------------------------------------------------------
Status BufMgr::FreePage(PageID pid)
{
	//TODO: add your code here
	int frameNum = this->FindFrame(pid);

	if(frameNum != INVALID_FRAME) {
		if(this->frames[frameNum].GetPinCount() > 1) {
			return FAIL;
		}
		this->frames[frameNum].Unpin();
		this->frames[frameNum].EmptyIt();
		this->replacer->RemoveFrame(frameNum);
	}

	MINIBASE_DB->DeallocatePage(pid);
	return OK; 
}


//--------------------------------------------------------------------
// BufMgr::FlushPage
//
// Input    : pid  - page id of a particular page 
// Output   : None
// Purpose  : Flush the page with the given pid to disk.
// Condition: The page with page id = pid must be in the buffer,
//            and is not pinned. pid cannot be INVALID_PAGE.
// PostCond : The page with page id = pid is written to disk if it's dirty. 
//            The frame where the page resides is empty.
// Return   : OK if operation is successful.  FAIL otherwise.
//--------------------------------------------------------------------
Status BufMgr::FlushPage(PageID pid)
{
	//TODO: add your code here
	Status didFlushPage = FAIL;

	int frameNum = this->FindFrame(pid);

	// check if page can/should be flushed
	if((frameNum != INVALID_FRAME) && (this->frames[frameNum].NotPinned()) && (this->frames[frameNum].IsDirty())) {

		this->replacer->RemoveFrame(frameNum);

		didFlushPage = this->frames[frameNum].Write();
		
		this->frames[frameNum].EmptyIt(); // SET DIRTY TO FALSE AFTER FLUSH
		this->numDirtyPageWrites += 1;
	}

	return didFlushPage;	
} 

//--------------------------------------------------------------------
// BufMgr::FlushAllPages
//
// Input    : None
// Output   : None
// Purpose  : Flush all pages in this buffer pool to disk.
// Condition: All pages in the buffer pool must not be pinned.
// PostCond : All dirty pages in the buffer pool are written to 
//            disk (even if some pages are pinned). All frames are empty.
// Return   : OK if operation is successful.  FAIL otherwise.
//--------------------------------------------------------------------

Status BufMgr::FlushAllPages()
{
	//TODO: add your code here

	// Check condition, fail if pinned page found
	Status flushResult = OK;
	for(int k=0; k < this->numFrames; k++) {

		if(!(this->frames[k].NotPinned())) {
			flushResult = FAIL;
		}

		// Flush page and increment stat
		if(this->frames[k].IsDirty()) {

			this->frames[k].Write();
			this->frames[k].EmptyIt();
			this->numDirtyPageWrites += 1;
		}

		// remove from replacer
		this->frames[k].EmptyIt();
		this->replacer->RemoveFrame(k);
	}

	return flushResult;
}


//--------------------------------------------------------------------
// BufMgr::GetNumOfUnpinnedFrames
//
// Input    : None
// Output   : None
// Purpose  : Find out how many unpinned locations are in the buffer
//            pool.
// Condition: None
// PostCond : None
// Return   : The number of unpinned buffers in the buffer pool.
//--------------------------------------------------------------------
unsigned int BufMgr::GetNumOfUnpinnedFrames()
{
	//TODO: add your code here
	int unpinnedCount = 0;
	for(int k=0; k<this->numFrames; k++) {
		if(this->frames[k].NotPinned()) {
			unpinnedCount += 1;
		}
	}

	return unpinnedCount;
}

//--------------------------------------------------------------------
// BufMgr::FindFrame
//
// Input    : pid - a page id 
// Output   : None
// Purpose  : Look for the page in the buffer pool, return the frame
//            number if found.
// PreCond  : None
// PostCond : None
// Return   : the frame number if found. INVALID_FRAME otherwise.
//--------------------------------------------------------------------
int BufMgr::FindFrame( PageID pid )
{
	//TODO: add your code here

	int frameNum = INVALID_FRAME; // will never occur if found

	for(int k=0; k<this->numFrames; k++) {
		if(this->frames[k].GetPageID() == pid) {
			frameNum = k;
		}
	}

	// If found, this constitutes a frame access, need to account for that in 
	// replacer if page is unpinned
	if(frameNum != INVALID_FRAME) {
		if(this->frames[frameNum].GetPinCount() == 0) {
			this->replacer->RemoveFrame(frameNum);
			this->replacer->AddFrame(frameNum);
		}
	}

	return frameNum;
}

void BufMgr::ResetStat() { 
	totalHit = 0; 
	totalCall = 0; 
	numDirtyPageWrites = 0;
}

void  BufMgr::PrintStat() {
	cout<<"**Buffer Manager Statistics**"<<endl;
	cout<<"Number of Dirty Pages Written to Disk: "<<numDirtyPageWrites<<endl;
	cout<<"Number of Pin Page Requests: "<<totalCall<<endl;
	cout<<"Number of Pin Page Request Misses "<<totalCall-totalHit<<endl;
}

/** Returns the frame number of a free frame, or one chosen by pickVictim
	Returns -1 if no frame available
	Function used to abstract out repeat code
	Input: None
	Output: int (frame number)
*/
int BufMgr::getAvailableFrame() {
	// if no frames are unpinned, no frames available
	if (this->GetNumOfUnpinnedFrames() == 0) {
		return -1;
	}

	// find free frame
	int frameNumber;
	for(int k = 0; k < this->numFrames; k++) {
		if (!this->frames[k].IsValid()) {
			frameNumber = k;
			return frameNumber;
		}
	}

	// No free frames, pick victim
	frameNumber = this->replacer->PickVictim();
	
	if(this->FlushPage(this->frames[frameNumber].GetPageID()) == OK) {
		this->frames[frameNumber].EmptyIt();
	}// else {
		//return -1;
	//} 

	return frameNumber;
} 