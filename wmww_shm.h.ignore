#include <string>
#include <memory>

class ShmBuffer
{
public:
	ShmBuffer(std::string nameIn);
	// this function must only be called before buffer is opened
	// it increases the size this buffer will be, and returns the offset to the start of the new chunk
	int claimChunk(int chunkSize);
	void open();
	~ShmBuffer();
	void * getData() { return data; }
	
private:
	
	// file descriptor of the buffer
	int fd = -1;
	
	// name of the buffer
	std::string name;
	
	// the actual data pointer
	void * data = nullptr;
	
	// total size of the SHM buffer
	size_t dataSize = 0;
	
	// if this object created the buffer, this just opened it otherwise
	bool thisCreatedBuffer = false;
	
	// if the SHM buffer has been successfully opened
	// if false then all other data in this class is undefined
	bool isValid = false;
};
