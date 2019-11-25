#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QPixmap>
#include <QImage>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    myCam = new myCamera;
    connect(myCam, SIGNAL(mySendData(unsigned char *)), this, SLOT(myRecvData(unsigned char*)));

    myCam->pauseThread();
    myCam->start();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete myCam;
}

// start
void MainWindow::on_pushButton_clicked()
{
    myCam->continueThread();
}

void MainWindow::myRecvData(unsigned char *data)
{
    QPixmap pixmap = QPixmap::fromImage(QImage(data, 640, 480, 640*3, QImage::Format_RGB888));

    pixmap.scaled(ui->label->size(), Qt::KeepAspectRatio);

    ui->label->setAutoFillBackground(true);
    ui->label->setPixmap(pixmap);
}

// pause
void MainWindow::on_pushButton_2_clicked()
{
    myCam->pauseThread();
}

// close
void MainWindow::on_pushButton_3_clicked()
{
    myCam->myExitThread();
    myCam->continueThread();
    myCam->quit();
    myCam->wait();
}

// capture
void MainWindow::on_pushButton_4_clicked()
{
    myCam->myCapture();
}
