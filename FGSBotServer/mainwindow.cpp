#include "mainwindow.h"
#include "ui_mainwindow.h"

#define serialDataListSize 4

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "[123]";
    connect(videoServer, &QTcpServer::newConnection, [=](){
        qDebug() << "[VIDEO SERVER CONNECTED]";
        videoSocket = videoServer->nextPendingConnection();
        connect(this, &MainWindow::videoDataAvailable, [=](QByteArray data){
            if (videoSocket->isWritable())
                sendData(videoSocket,data);
        });
        connect(videoSocket, &QTcpSocket::readyRead, [=](){
            QStringList data = QString(videoSocket->readAll()).split(":");
            if (data.first() == "INFO" && !data.last().isEmpty())
                this->initVideo(data.last());
        });
        sendData(videoSocket,this->getVideoInfo().toUtf8());
        connect(videoSocket, &QTcpSocket::disconnected, [=](){
            cameras.clear();
//            camera->stop();
        });
    });

    connect(dataServer, &QTcpServer::newConnection, [=](){
        qDebug() << "[DATA SERVER CONNECTED]";
        dataSocket = dataServer->nextPendingConnection();
        connect(this, &MainWindow::serailDataAvailable, [=](QByteArray data){
            if (dataSocket->isWritable()){
                qDebug() << "[NEW SERIAL DATA]: " << QString(data);
                sendData(dataSocket,data);
            }
        });
        connect(dataSocket, &QTcpSocket::readyRead, [=](){
            QStringList data = QString(dataSocket->readAll()).split(":");
            qDebug() << "[READ DATA]: " << data.first() << "\t" << data.last();
            if (data.first() == "INFO" && !data.last().isEmpty()){
                this->initSerial("/dev/"+data.last());
            } else if (data.first() == "MC" && !data.last().isEmpty()){
                serial->write(data.last().toUtf8());
            }
        });
        sendData(dataSocket,this->getSerialInfo().toUtf8());
    });

    dataServer->listen(QHostAddress::Any, PORT_DATA);
    videoServer->listen(QHostAddress::Any, PORT_VIDEO);

//    //////////////////////////////////////////////////////
//    ///
//    //////////////////////////////////////////////////////

//    QTcpSocket *dataTest = new QTcpSocket();
//    connect(dataTest, &QTcpSocket::readyRead, [=](){
//        QString data = dataTest->readAll();
//        dataTest->write(data.toUtf8());
//    });
//    dataTest->connectToHost("localhost",PORT_VIDEO);

//    //////////////////////////////////////////////////////
//    ///
//    //////////////////////////////////////////////////////
}

void MainWindow::initSerial(QString name)
{
//    serial->close();
//    serial->clear();
    serial = new QSerialPort(this);
    serial->setPortName(name);
    serial->setBaudRate(9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    if (serial->open(QIODevice::ReadWrite)){
        qDebug() << "[SERIAL OPENED]";
    } else {
        qDebug() << "[SERIAL NOT OPENED]";
    }

    connect(serial, &QSerialPort::readyRead, [=](){
        QString s;
        QStringList serialDataList;
        if (serial->canReadLine())
            s = QString(serial->readAll().simplified());
        if (s.isEmpty() || s.isNull()){
            for (int i = 0; i < serialDataListSize; i++)
                serialDataList.append("-----");
        }

        emit serailDataAvailable(s.toUtf8());
        qDebug() << "SENDING DATA: " + s;
    });
    emit serailDataAvailable(QString("-----;-----;-----;-----").toUtf8());
}

void MainWindow::initVideo(QString name)
{
//    camera->stop();
//    camera->disconnect();
    imageCapture = new QCameraImageCapture(nullptr);

    if (!cameras.size()){
        qDebug() << "[NO CAMERAS]";
        return;
    }
    foreach (QCameraInfo c, cameras) {
        if (c.deviceName() == name)
            camera = new QCamera(c);
    }
    if (camera == nullptr)
        return;
    imageCapture = new QCameraImageCapture(camera);
    camera->start();
    connect(imageCapture, &QCameraImageCapture::imageCaptured, [&](int id, QImage img){
        QPixmap pixmap = QPixmap::fromImage(img);
        pixmap = pixmap.scaled(320,180);
        QByteArray array;
        QBuffer buffer(&array);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "JPG");

        emit videoDataAvailable(array);
        qDebug() << "SENDING IMAGE";
    });
    connect(captureTimer, &QTimer::timeout, [&](){
        camera->searchAndLock();
        imageCapture->capture();
        camera->unlock();
    });
    captureTimer->start(100);
}

void MainWindow::sendData(QTcpSocket *socket, QByteArray data)
{
    if(socket->isWritable()) {
        qDebug() << "[SEND] " << socket;
        if (socket == dataSocket)
            qDebug() << data;
        qDebug() << endl;

        QByteArray block;
        QDataStream m_outcomingMessage(&block, QIODevice::WriteOnly);
        m_outcomingMessage.setVersion(QDataStream::Qt_5_9);
        m_outcomingMessage << data;
        socket->write(block);
        socket->flush();
    } else {
        qDebug() << "Socket disconnected, message added to queue";
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::getSerialInfo()
{
    const auto infos = QSerialPortInfo::availablePorts();
    QStringList serialNameList;
    for (const QSerialPortInfo &info : infos)
        serialNameList.append(info.portName());
    QString s = serialNameList.join(";");
    s.prepend("INFO:");
    return s;

}

QString MainWindow::getVideoInfo()
{
    QStringList cameraList;
    foreach (QCameraInfo c, cameras) {
        cameraList.append(c.deviceName());
    }
    QString s = cameraList.join(";");
    s.prepend("INFO:");
    return s;
}
