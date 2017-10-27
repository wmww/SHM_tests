#include "ShmBuffer.h"
#include <iostream>

using namespace boost::interprocess;

#define debug_off(msg)
#define debug_on(msg) std::cout << msg << std::endl

// switch between debug_on and debug_off to toggle debugging
#define debug debug_on

ShmBuffer::ShmBuffer()
{}

ShmBuffer::~ShmBuffer()
{
	if (isOpen)
	{
		close();
	}
}

int ShmBuffer::addBlock(int sizeIn)
{
	if (isOpen)
	{
		throw std::runtime_error("ShmBuffer::addBlock() called after buffer was opened\n");
	}
	int offset = size;
	size += sizeIn;
	return offset;
}

void ShmBuffer::open(std::string nameIn)
{
	if (isOpen)
	{
		close();
	}
	name = nameIn;
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
	}
	region = mapped_region(shm, read_write);
	isOpen = true;
}

void * ShmBuffer::getData()
{
	if (!isOpen)
	{
		throw std::runtime_error("ShmBuffer::getData() called before buffer was opened\n");
	}
	return region.get_address();
}

void ShmBuffer::close()
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
	isOpen = false;
}

