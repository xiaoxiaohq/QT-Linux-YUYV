#ifndef MYCAMERA_H
#define MYCAMERA_H

#include "mylcd.h"

#include <QMutex>
#include <QThread>

class myCamera : public QThread
{
    Q_OBJECT

public:
    myCamera();
    ~myCamera();

    int myCameraInit(void);
    int myCameraClose(void);
    void run();
    void pauseThread(void);
    void continueThread(void);
    void myCapture(void);
    void myExitThread(void);

signals:
        void mySendData(unsigned char *);

private:
    int camFd;
    QMutex mylock;
    bool exitFlag;

    unsigned char *rgbData;
    int nbuf;
    unsigned int len[4];
    unsigned char *startdata[4];
    int picIndex;
    bool picFlag;

    static int m_width;
    static int m_height;

    MyLcd m_lcd;

    int myCameraOpen(void);
    int mySetCameraFmt(void);
    int myRequestCache(void);
    int myGetRGBData(unsigned char *yuvdata);
    unsigned int myYUVtoRGB(int y, int u, int v);
    int myCameraStart(void);
    int myGetCameraData(void);
    int myCameraCapture(void);
};

#endif // MYCAMERA_H
