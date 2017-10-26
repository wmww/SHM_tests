#include "wmww_shm.h"
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
	std::cout << "creating buffer" << std::endl;
	ShmBuffer buffer("/shm_test");
	std::cout << "opening buffer" << std::endl;
	buffer.open();
	std::this_thread::sleep_for(std::chrono::milliseconds(4000));
	std::cout << "closing buffer" << std::endl;
}

