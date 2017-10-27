#include <chrono>
#include <thread>
#include "ShmBuffer.h"

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>

const std::string shmName = "MySharedMemory";
/*
using namespace boost::interprocess;

template <typename T>
struct DoubleBuffer
{
	bool hasSetUp = true;
	struct Elem
	{
		boost::interprocess::interprocess_mutex mutex;
		T data;		
	} data[2];
};
* */

int main(int argc, char ** argv)
{
	ShmBuffer buffer;
	BufferBlock<char, DoubleBuffer> block;
	block.setupFrom(&buffer);
	buffer.open(shmName);
	
	for (int i = 0; i < 20; i++)
	{
		if (argc > 1)
		{
			if (!block.writeData(&argv[1][0]))
			{
				std::cout << "write failed" << std::endl;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		char c = '+';
		if (!block.readData(&c))
		{
			std::cout << "read failed" << std::endl;
		}
		std::cout << c << std::endl;
		
	}
	
	/*
	//Remove shared memory on construction and destruction
	struct shm_remove
	{
		void remove() { shared_memory_object::remove(shmName.c_str()); }
		//shm_remove() { remove(); }
		~shm_remove() { remove(); }
	} remover;

	//Create a shared memory object.
	shared_memory_object shm (open_or_create, shmName.c_str(), read_write);

	//Set size
	shm.truncate(1000);

	//Map the whole shared memory in this process
	mapped_region region(shm, read_write);
	
	typedef DoubleBuffer<char[16]> Buffer;
	
	Buffer * data = static_cast<Buffer *>(region.get_address());
	
	if (!data->hasSetUp)
	{
		std::cout << "setting up" << std::endl;
		new (region.get_address()) Buffer;
	}
	
	//Write all the memory to 1
	//std::memset(region.get_address(), 1, region.get_size());
	
	for (int i = 0; i < 20; i++)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		std::cout << "locking..." << std::endl;
		//scoped_lock<interprocess_mutex> lock(data->data[0].mutex);
		data->data[0].mutex.lock();
		if (true)
		{
			std::cout << "lock worked" << std::endl;
			if (argc > 1)
			{
				std::cout << "writing..." << std::endl;
				data->data[0].data[0] = argv[1][0];
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		else
		{
			std::cout << "lock failed" << std::endl;
		}
		std::cout << "buffer: " << data->data[0].data[0] << std::endl;
		std::cout << "unlocking..." << std::endl;
		data->data[0].mutex.unlock();
	}
	*/
	/*
	std::cout << "creating buffer" << std::endl;
	ShmBuffer buffer("/shm_test");
	std::cout << "opening buffer" << std::endl;
	buffer.open();
	std::cout << "closing buffer" << std::endl;
	*/
}

