#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPixmap>
#include <QDataStream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(videoSocket, &QTcpSocket::readyRead, this, &MainWindow::readVideo);
    connect(dataSocket, &QTcpSocket::readyRead, this, &MainWindow::readData);
    connect(ui->networkConnect, &QPushButton::clicked, [=](){
        QString ip = ui->networkIp->text();
        dataSocket->connectToHost(ip, PORT_DATA);
        videoSocket->connectToHost(ip, PORT_VIDEO);
        ui->serialList->clear();
        ui->cameraList->clear();
    });
    connect(ui->paramsSelect, &QPushButton::clicked, [=](){
        QString s = "INFO:";
        dataSocket->write(s.toUtf8() + ui->serialList->currentText().toUtf8());
        videoSocket->write(s.toUtf8() + ui->cameraList->currentText().toUtf8());
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readVideo()
{
    incomingVideoData.setDevice(videoSocket);
    incomingVideoData.setVersion(QDataStream::Qt_5_9);
    videoSocket->waitForBytesWritten();
    QByteArray data;
    incomingVideoData.startTransaction();
    incomingVideoData >> data;

    qDebug() << "[RECEIVED PART] " << data << endl;

    if (!incomingVideoData.commitTransaction())
        return;
    qDebug() << "[RECEIVE] " << data << endl;

//    videoSocket->waitForBytesWritten();
//    QString message;
//    incomingVideoData.startTransaction();
//    incomingVideoData >> message;

//    qDebug() << "[RECEIVED PART] " << message << endl;

//    if (!incomingVideoData.commitTransaction())
//        return;
//    qDebug() << "[RECEIVE] " << message << endl;
//    if(m_HasCompetition) {
//    //    emit newDataAvailable(m_Index, message);
//    } else {
//        readMessage(message);
//    }
//    QByteArray data = videoSocket->readAll();

    QStringList dataList = QString(data).split(":");
    if (dataList.first() == "INFO"){
        QStringList list = dataList.last().split(";");
        ui->cameraList->addItems(list);
    } else{
        QPixmap myPicture;
        myPicture.loadFromData(data, "JPG");
        if (myPicture.isNull()){
            qDebug() << "Bad image";
            return;
        }
        myPicture = myPicture.scaled(640, 360);
        ui->camera->setPixmap(myPicture);
    }
}

void MainWindow::readData()
{
    incomingSerialData.setDevice(dataSocket);
    incomingSerialData.setVersion(QDataStream::Qt_5_9);
    dataSocket->waitForBytesWritten();
    QByteArray data;
    incomingSerialData.startTransaction();
    incomingSerialData >> data;

    qDebug() << "[RECEIVED PART] " << data << endl;

    if (!incomingSerialData.commitTransaction())
        return;
    qDebug() << "[RECEIVE] " << data << endl;

    QStringList dataList = QString(data).split(":");
    if (dataList.first() == "INFO"){
        QStringList list = dataList.last().split(";");
        ui->serialList->addItems(list);
    } else{
        dataList = dataList.last().split(" ");
        dataList = dataList.last().split(";");
        if (dataList.size()<4)
            return;
        ui->gpsLat->setText(dataList.at(LATITUDE));
        ui->gpsLon->setText(dataList.at(LONGITUDE));
        ui->gpsAlt->setText(dataList.at(ALTITUDE));
        ui->gpsSat->setText(dataList.at(SATELLITES));
    }
}
