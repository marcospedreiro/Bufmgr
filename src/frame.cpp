// Implementation of frame.h
// Marcos Pedreiro, Jenny Zou | 10/17/16

#include "page.h"
#include "db.h"
#include "frame.h"

/** Constructor for Frame
	Input: None
	Output: None
*/
Frame::Frame() 
{
	this->pid = INVALID_PAGE;
	this->data = NULL;
	this->pinCount = 0;
	this->dirty = false;

}

/** Destructor for Frame
	Writes page to disk if dirty
	Input: None
	Output: None
*/
Frame::~Frame() 
{
	this->EmptyIt();
	delete this->data;
	this->data = NULL;
}

/** Increment the pinCounter for the frame
	Input: None
	Output: None
*/
void Frame::Pin() 
{
	this->pinCount += 1;
}

/** Decrement the pinCounter for the frame
	Input: None
	Output: None
*/
void Frame::Unpin()
{
	this->pinCount -= 1;
}

/** Returns the pinCount of the frame
	Input: None
	Output: int (pinCount)
*/
int Frame::GetPinCount()
{
	return this->pinCount;
}

/** Sets pid to INVALID_PAGE, resets pinCount and dirty
	Input: None
	Output: None
*/
void Frame::EmptyIt() 
{
	this->pid = INVALID_PAGE;
	this->pinCount = 0;
	this->dirty = false;
}

/** Marks the frames contents as dirty
	Input: None
	Output: None
*/
void Frame::DirtyIt() 
{
	this->dirty = true;
}

/** Sets the Frames PageID
	Input: A valid pid
	Output: None
*/
void Frame::SetPageID(PageID pid)
{
	this->pid = pid;
}

/** Returns the Frames dirty value
	Input: None
	Output: bool (dirty)
*/
bool Frame::IsDirty()
{
	return this->dirty;
}

/** Returns true if the frame contains a valid pid
	Input: None
	Output: bool
*/
bool Frame::IsValid()
{
	if(this->pid == INVALID_PAGE) {
		return false;
	} else {
		return true;
	}
}

/**	Writes the content of a Frame to disk
	Input: None
	Output: Status
*/
Status Frame::Write()
{
	return MINIBASE_DB->WritePage(this->pid, this->data);
}

/** Reads the pid from disk and puts it inside the Frame
	Fills Frame with new Page if initially empty
	Input: PageID, bool
	Output: Status
*/
Status Frame::Read(PageID pid, bool isEmpty)
{
	Status readStatus = FAIL;
	if(!(this->data)) { // if empty
		this->data = new Page();
	} 
	this->pid = pid;
	readStatus = OK;
	if (!isEmpty) {
		readStatus = MINIBASE_DB->ReadPage(this->pid, this->data);
	}

	return readStatus;
}

/** States whether a Frames contents is pinned or not
	Input: None
	Output: bool (pinCount == 0)
*/
bool Frame::NotPinned()
{
	return (this->pinCount == 0);
}

/** Returns the PageID of the page stored in the Frame
	Input: None
	Output: PageID (this->pid)
*/
PageID Frame::GetPageID()
{
	return this->pid;
}

/** Returns a pointer to the page stored in the Frame
	Input: None
	Output: Page* (this->data)
*/
Page* Frame::GetPage()
{
	return this->data;
}