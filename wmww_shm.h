#include <string>
#include <memory>

class ShmBuffer
{
public:
	ShmBuffer(std::string nameIn, int dataSizeIn);
	~ShmBuffer();
	void * getData() { return data; }
	
private:
	
	int fd = -1;
	
	std::string name;
	
	// the actual data pointer
	void * data = nullptr;
	
	// total size of the SHM buffer
	int dataSize = 0;
	
	// if this object created the buffer, this just opened it otherwise
	bool thisCreatedBuffer = false;
	
	// if the SHM buffer has been successfully opened
	// if false then all other data in this class is undefined
	bool isValid = false;
};

