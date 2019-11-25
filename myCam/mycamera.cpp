#include "mycamera.h"
#include <iostream>

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include <QDebug>
#include <QImage>

myCamera::myCamera()
{
    // store rgb data
    rgbData = new unsigned char [640*480*3];

    // camera init
    nbuf = 4;
    myCameraInit();
    myCameraStart();

    // flag bit
    picFlag = false;
    picIndex = 0;
    exitFlag = false;
}

int myCamera::m_width = 640;
int myCamera::m_height = 480;

myCamera::~myCamera()
{
    myCameraClose();
}

int myCamera::myCameraOpen()
{
    camFd = ::open("/dev/video7", O_RDWR);
    if (camFd == -1)
    {
        std::cout << "open camera failed" << std::endl;
        return -1;
    }

    return camFd;
}

int myCamera::myCameraInit()
{
    myCameraOpen();
    mySetCameraFmt();
    myRequestCache();

    struct v4l2_buffer mybuffer[nbuf];

    int i;
    for (i=0; i<nbuf; i++)
    {
        bzero(&mybuffer[i], sizeof(mybuffer[i]));

        mybuffer[i].type 	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
        mybuffer[i].memory 	= V4L2_MEMORY_MMAP;
        mybuffer[i].index 	= i;

        ioctl(camFd, VIDIOC_QUERYBUF, &mybuffer[i]);

        len[i] 		= mybuffer[i].length;
        startdata[i] 	= (unsigned char *)mmap(NULL, len[i],  PROT_READ|PROT_WRITE, MAP_SHARED, camFd, mybuffer[i].m.offset);

        ioctl(camFd, VIDIOC_QBUF, &mybuffer[i]);
    }

    qDebug() << "init finish";
    return 0;
}

int myCamera::myCameraStart()
{
    enum v4l2_buf_type vtype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(camFd, VIDIOC_STREAMON, &vtype);

    return 0;
}

int myCamera::myCameraClose()
{
    for (int i=0; i<nbuf; i++)
            munmap(startdata[i], len[i]);
    ::close(camFd);

    qDebug() << "camera close";
    return 0;
}

int myCamera::mySetCameraFmt()
{
    struct v4l2_format myfmt;
    bzero(&myfmt, sizeof(myfmt));

    myfmt.type 					= V4L2_BUF_TYPE_VIDEO_CAPTURE;
    myfmt.fmt.pix.width 		= m_width;
    myfmt.fmt.pix.height 		= m_height;
    myfmt.fmt.pix.pixelformat 	= V4L2_PIX_FMT_YUYV;
    myfmt.fmt.pix.field 		= V4L2_FIELD_INTERLACED;

    ioctl(camFd, VIDIOC_S_FMT, &myfmt);

    return 0;
}

int myCamera::myRequestCache()
{
    struct v4l2_requestbuffers myrequest;
    bzero(&myrequest, sizeof(myrequest));

    myrequest.count 	= nbuf;
    myrequest.type 		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
    myrequest.memory 	= V4L2_MEMORY_MMAP;

    ioctl(camFd,  VIDIOC_REQBUFS, &myrequest);

    return 0;
}

// get video data
int myCamera::myGetCameraData()
{
    qDebug() << "start get camera data";
    struct v4l2_buffer mybuf1;
    bzero(&mybuf1, sizeof(mybuf1));

    mybuf1.type 	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
    mybuf1.memory 	= V4L2_MEMORY_MMAP;

    int i = 0;
    while(1)
    {
        mylock.lock();

        mybuf1.index = i;
        ioctl(camFd, VIDIOC_DQBUF, &mybuf1);	//fo

        myGetRGBData(startdata[i]);

        mybuf1.index = i;
        ioctl(camFd, VIDIOC_QBUF, &mybuf1);		//fi

        i++;
        i = (i > 3) ? 0 : i;

        if (exitFlag == true)
            break;

        mylock.unlock();
    }

    return 0;
}

// get RGB data
int myCamera::myGetRGBData(unsigned char *yuvdata)
{
    int i = 0;
    unsigned int num = 0;
    int j = 0;
    unsigned char *p = (unsigned char *)&num;
    while(i < m_width*m_height*2)
    {
        num = myYUVtoRGB(yuvdata[i], yuvdata[i+1], yuvdata[i+3]);
        rgbData[j] = *(p+2);
        rgbData[j+1] = *(p+1);
        rgbData[j+2] = *p;
        num = myYUVtoRGB(yuvdata[i+2], yuvdata[i+1], yuvdata[i+3]);
        rgbData[j+3] = *(p+2);
        rgbData[j+4] = *(p+1);
        rgbData[j+5] = *p;

        i += 4;
        j += 6;
    }

    // emit signal to transfer data
    emit mySendData(rgbData);

    // call capture function
    myCameraCapture();

    return 0;
}

// yuv to rgb data
unsigned int myCamera::myYUVtoRGB(int y, int u, int v)
{
    int r, g, b;
    b = 1164*(y - 16)/1000 + 2018*(u - 128)/1000;
    g = 1164*(y - 16)/1000 - 813*(v - 128)/1000 - 391*(u - 128)/1000;
    r = 1164*(y - 16)/1000 + 1596*(v - 128)/1000;

    b = (b > 255) ? 255 : ((b < 0) ? 0 : b);
    g = (g > 255) ? 255 : ((g < 0) ? 0 : g);
    r = (r > 255) ? 255 : ((r < 0) ? 0 : r);

    unsigned int pix = (r<<16 | g<<8 | b);
    return pix;
}

// virtual thread run to capture picture
void myCamera::run()
{
    myGetCameraData();
}

// pause thread with lock
void myCamera::pauseThread()
{
    mylock.lock();
}

// unlock make thread continue
void myCamera::continueThread()
{
    mylock.unlock();
}

// external call function change standard bit
void myCamera::myCapture()
{
    picFlag = true;
}

void myCamera::myExitThread()
{
    exitFlag = true;
}

int myCamera::myCameraCapture()
{
    // capture picture
    if (picFlag == true)
    {
        // clean picFlag
        picFlag = false;

        // convert rgb data to jpg image
        QImage img(rgbData, m_width, m_height, m_width*3, QImage::Format_RGB888);
        QString filename = QString("./image/%1.jpg").arg(picIndex);
        img.save(filename, 0, 90);

        //index add
        picIndex++;
        qDebug() << "capture a picture";
    }

    return 0;
}
