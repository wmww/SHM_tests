#include "wmww_shm.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

ShmBuffer::ShmBuffer(std::string nameIn, int dataSizeIn):
	name(nameIn),
	dataSize(dataSizeIn)
{
	fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
	if (fd < 0)
	{
		std::cerr << "opening SHM file '" << name << "' failed: " << strerror(errno) << std::endl;
		return;
	}
	ftruncate(fd, dataSize);
	data = mmap(0, dataSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED)
	{
		std::cerr << "mapping SHM buffer '" << nameIn << "' failed: " << strerror(errno) << std::endl;
		return;
	}
	isValid = true;
}

ShmBuffer::~ShmBuffer()
{
	if (data)
	{
		if (munmap(data, dataSize) < 0)
		{
			std::cerr << "unmapping SHM buffer failed: " << strerror(errno) << std::endl;
		}
		data = nullptr;
	}
	
	if (fd >= 0)
	{
		if (close(fd) < 0)
		{
			std::cerr << "closing SHM fd for '" << name << "' failed: " << strerror(errno) << std::endl;
		}
		fd = -1;
		if (shm_unlink(name.c_str()) < 0)
		{
			std::cerr << "unlinking SHM '" << name << "' failed: " << strerror(errno) << std::endl;
		}
	}
}

