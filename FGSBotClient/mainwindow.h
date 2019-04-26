#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

#define PORT_DATA   22022
#define PORT_VIDEO  22023

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

enum SERIAL
    {
        SATELLITES,
        LONGITUDE,
        LATITUDE,
        ALTITUDE
    };

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTcpSocket *videoSocket = new QTcpSocket();
    QTcpSocket *dataSocket = new QTcpSocket();
    quint16     m_nNextBlockSize = 0;
    QByteArray  videoData;
    int         videoDataSize = 0;
    QDataStream incomingVideoData;
    QDataStream incomingSerialData;

private slots:
    void readVideo();
    void readData();
};

#endif // MAINWINDOW_H
