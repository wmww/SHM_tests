#pragma once

#include <string>
#include <iostream>
#include <functional>
#include <vector>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#define debug_off(msg)
#define debug_on(msg) std::cout << msg << std::endl

// switch between debug_on and debug_off to toggle debugging
#define debug debug_on

using namespace boost::interprocess;

class ShmBuffer
{
public:
	ShmBuffer()
	{}
	
	~ShmBuffer()
	{
		if (isOpen)
			close();
	}
	
	// reserves a block of the given size that will be available at the returned offset once open is called
	// this should not be called after open
	// essentially all this does is increase the total size by the given size and returns the previous total size
	void addBlock(int sizeIn, std::function<void(void *, bool)> callback)
	{
		if (isOpen)
		{
			throw std::runtime_error("ShmBuffer::addBlock() called after buffer was opened\n");
		}
		size += sizeIn;
		blocks.push_back({sizeIn, callback});
	}
	
	// opens the SHM buffer
	void open(std::string nameIn)
	{
		if (isOpen)
		{
			close();
		}
		name = nameIn;
		bool created = false;
		try
		{
			debug("trying to open SHM");
			shm = shared_memory_object(open_only, name.c_str(), read_write);
		}
		catch (boost::interprocess::interprocess_exception ex)
		{
			debug("creating SHM because open failed");
			shm = shared_memory_object(create_only, name.c_str(), read_write);
			shm.truncate(size);
			created = true;
		}
		try
		{
			region = mapped_region(shm, read_write);
			isOpen = true;
		}
		catch(interprocess_exception &ex)
		{
			std::cerr << ex.what() << std::endl;
			return;
		}
		int offset = 0;
		void * ptr = region.get_address();
		for (auto i: blocks)
		{
			i.callback((char *)ptr + offset, created);
			offset += i.size;
		}
		blocks.clear();
	}
	
	// must be called after opened, returns a pointer to the data
	void * getData()
	{
		if (!isOpen)
		{
			throw std::runtime_error("ShmBuffer::getData() called before buffer was opened\n");
		}
		try
		{
			return region.get_address();
		}
		catch(interprocess_exception &ex)
		{
			std::cerr << ex.what() << std::endl;
		}
	}
	
	// closes the SHM buffer, called automatically by the destructor if not called explicitly before
	void close()
	{
		try
		{
			debug("closing SHM");
			if (!isOpen)
			{
				throw std::runtime_error("ShmBuffer::close() called but buffer was not open\n");
			}
			region = mapped_region();
			shm = shared_memory_object();
			shared_memory_object::remove(name.c_str());
			size = 0;
		}
		catch(interprocess_exception &ex)
		{
			std::cerr << ex.what() << std::endl;
		}
		isOpen = false;
	}
	
	bool getIsOpen() { return isOpen; }
	
private:	
	// if the buffer is open
	bool isOpen = false;
	
	std::string name;
	
	// size of the entire buffer
	int size = 0;
	
	boost::interprocess::shared_memory_object shm;
	boost::interprocess::mapped_region region;
	
	struct Block
	{
		int size;
		std::function<void(void *, bool)> callback;
	};
	
	std::vector<Block> blocks;
};

template <typename DataT, template<typename> typename BlockT>
class BufferBlock
{
public:
	
	// must be called before the buffer is opened
	void setupFrom(ShmBuffer * buffer)
	{
		if (isReady)
		{
			throw std::runtime_error("BufferBlock::setupFrom called more then once\n");
		}
		buffer->addBlock(sizeof(BlockT<DataT>),
			[this](void * ptr, bool created)
			{
				if (created)
				{
					new (static_cast<BlockT<DataT> *>(ptr)) BlockT<DataT>;
				}
				block = (BlockT<DataT> *)ptr;
			}
		);
		isReady = true;
	}
	
	// write and read data to and from the buffer, returns true if successful and false if failed
	bool writeData(DataT * data)
	{
		if (block == nullptr)
		{
			throw std::runtime_error("BufferBlock::writeData called before buffer opened\n");
		}
		
		try
		{
			return block->writeData(data);
		}
		catch(interprocess_exception &ex)
		{
			std::cerr << ex.what() << std::endl;
			return false;
		}
	}
	
	bool readData(DataT * data)
	{
		if (block == nullptr)
		{
			throw std::runtime_error("BufferBlock::readData called before buffer opened\n");
		}
		
		try
		{
			return block->readData(data);
		}
		catch(interprocess_exception &ex)
		{
			std::cerr << ex.what() << std::endl;
			return false;
		}
	}
	
	bool getIsReady() { return isReady; }
	
protected:
	
	// if this object has been set up
	bool isReady = false;
	
	BlockT<DataT> * block = nullptr;
};

// a block (part of a SHM buffer) that will simply fail if you try to read and write to it at the same time
template <typename T>
class SingleBuffer
{
public:
	interprocess_mutex mutex;
	T data;
	
	bool readData(T * dataIn)
	{
		scoped_lock<interprocess_mutex> lock(mutex, try_to_lock);
		
		if (lock)
		{
			memcpy(dataIn, &data, sizeof(T));
		}
		else
		{
			return false;
		}
	}
	
	bool writeData(T * dataIn)
	{
		scoped_lock<interprocess_mutex> lock(mutex, try_to_lock);
		
		if (lock)
		{
			memcpy(&data, dataIn, sizeof(T));
		}
		else
		{
			return false;
		}
	}
};

// a block (part of a SHM buffer) that will simply fail if you try to read and write to it at the same time
template <typename T>
class DoubleBuffer
{
public:
	static const int bufferCount = 2;
	
	struct Buffer
	{
		boost::interprocess::interprocess_mutex mutex;
		T data;
	};
	
	Buffer buffers[bufferCount];
	
	bool readData(T * dataIn)
	{
		// loop backwards so its in opposite direction of write
		for (int i = bufferCount - 1; i >= 0; i--)
		{
			scoped_lock<interprocess_mutex> lock(buffers[i].mutex, try_to_lock);
			if (lock)
			{
				memcpy(dataIn, &buffers[i].data, sizeof(T));
				return true;
			}
		}
		return false;
	}
	
	bool writeData(T * dataIn)
	{
		for (int i = 0; i < bufferCount; i++)
		{
			scoped_lock<interprocess_mutex> lock(buffers[i].mutex, try_to_lock);
			if (lock)
			{
				memcpy(&buffers[i].data, dataIn, sizeof(T));
			}
			else
			{
				return false;
			}
		}
		return true;
	}
};
