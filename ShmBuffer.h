#include <string>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <functional>

class ShmBuffer
{
public:
	ShmBuffer();
	~ShmBuffer();
	
	// reserves a block of the given size that will be available at the returned offset once open is called
	// this should not be called after open
	// essentially all this does is increase the total size by the given size and returns the previous total size
	int addBlock(int size);
	
	// opens the SHM buffer
	void open(std::string name);
	
	// must be called after opened, returns a pointer to the data
	void * getData();
	
	// closes the SHM buffer, called automatically by the destructor if not called explicitly before
	void close();
	
	bool getIsOpen() { return isOpen; }
	
private:	
	// if the buffer is open
	bool isOpen = false;
	
	std::string name;
	
	// size of the entire buffer
	int size = 0;
	
	boost::interprocess::shared_memory_object shm;
	boost::interprocess::mapped_region region;
};

// a block (part of a SHM buffer) that will simply fail if you try to read and write to it at the same time
template <typename T>
class SingleBufferBlock
{
public:
	SingleBufferBlock () {}
	
	// must be called before the buffer is opened
	void setupFrom(ShmBuffer * bufferIn)
	{
		if (isReady)
		{
			throw std::runtime_error("SingleBufferBlock::setupFrom called more then once\n");
		}
		buffer = bufferIn;
		offset = buffer->addBlock(sizeof(Block));
		isReady = true;
	}
	
	// write and read data to and from the buffer, returns true if successful and false if failed
	bool writeData(T * data)
	{
		return readOrWriteData(data, true);
	}
	
	bool readData(T * data)
	{
		return readOrWriteData(data, false);
	}
	
	bool getIsReady() { return isReady; }
	
private:
	
	struct Block
	{
		boost::interprocess::interprocess_mutex mutex;
		void * data;
	};
	
	bool readOrWriteData(T * data, bool write)
	{
		if (!isReady || !buffer->getIsOpen())
		{
			throw std::runtime_error("SingleBufferBlock::readOrWriteData called before everything is set up\n");
		}
		
		Block * block = static_cast<Block *>(buffer->getData());
		boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
			lock(block->mutex, boost::interprocess::try_to_lock);
		if (lock)
		{
			if (write)
			{
				memcpy(&block->data, data, sizeof(T));
			}
			else
			{
				memcpy(data, &block->data, sizeof(T));
			}
			
			return true;
		}
		else
		{
			return false;
		}
	}
	
	// if this object has been set up
	bool isReady = false;
	
	ShmBuffer * buffer = nullptr;
	
	// the offset of this block from the base pointer in the buffer
	int offset = 0;
};
