#include "mylcd.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <iostream>
MyLcd::MyLcd()
{

}

int MyLcd::myLcdOpen()
{
    lcdFd = ::open("/dev/fb0", O_RDWR);
    if (lcdFd == -1)
    {
        std::cout << "open lcd failed" << std::endl;
        return -1;
    }
    return lcdFd;
}

int MyLcd::myLcdMmap()
{
    lcdMem = (unsigned int *)mmap(NULL, 800*480*4, PROT_READ|PROT_WRITE, MAP_SHARED, lcdFd, 0);

    return 0;
}

int MyLcd::myLcdClose()
{
    munmap(lcdMem, 800*480*4);
    ::close(lcdFd);

    return 0;
}
