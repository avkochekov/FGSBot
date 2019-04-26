#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QTimer>
#include <QBuffer>
#include <QCamera>
#include <QCameraInfo>
#include <QCameraImageCapture>

#define PORT_SYS    22021
#define PORT_DATA   22022
#define PORT_VIDEO  22023

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QString getSerialInfo();
    QString getVideoInfo();
    void initSerial(QString name);
    void initVideo(QString name);
    void sendData();

private:
    Ui::MainWindow *ui;

    QSerialPort *serial = nullptr;

    QCamera *camera = nullptr;
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QTimer *captureTimer = new QTimer();
    QCameraImageCapture *imageCapture;

    QTcpServer *sysServer = new QTcpServer;
    QTcpServer *dataServer = new QTcpServer;
    QTcpServer *videoServer = new QTcpServer;
    QTcpSocket *sysSocket = new QTcpSocket;
    QTcpSocket *dataSocket = new QTcpSocket;
    QTcpSocket *videoSocket = new QTcpSocket;

    void sendData(QTcpSocket *socket, QByteArray data);

signals:
    void serailDataAvailable(QByteArray data);
    void videoDataAvailable(QByteArray data);
};

#endif // MAINWINDOW_H
