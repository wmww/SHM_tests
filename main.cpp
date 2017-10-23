#include "wmww_shm.h"
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
	std::cout << "opening buffer" << std::endl;
	ShmBuffer buffer("/shm_test", 1000);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	std::cout << "closing buffer" << std::endl;
}

