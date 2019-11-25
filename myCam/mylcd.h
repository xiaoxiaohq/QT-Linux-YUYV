#ifndef MYLCD_H
#define MYLCD_H


class MyLcd
{
public:
    MyLcd();

    int myLcdOpen(void);
    int myLcdMmap(void);
    int myLcdClose(void);

    int lcdFd;
    unsigned int *lcdMem;

};

#endif // MYLCD_H
