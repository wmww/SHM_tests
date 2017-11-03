#include <chrono>
#include <thread>
#include "ShmBuffer.h"

const std::string shmName = "MySharedMemory";

int main(int argc, char ** argv)
{
	shm_helper::Buffer buffer;
	shm_helper::Block<char, shm_helper::DoubleBuffer> block;
	block.setupFrom(&buffer);
	buffer.open(shmName);
	
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	
	buffer.close();
	
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	
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
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		char c = '+';
		if (!block.readData(&c))
		{
			std::cout << "read failed" << std::endl;
		}
		std::cout << c << std::endl;
		
	}
}

