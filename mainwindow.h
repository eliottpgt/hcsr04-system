#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTcpSocket>
#include <QTimer>
#include <QProgressBar>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void toggleConnection();
    void onConnected();
    void onDisconnected();
    void togglePolling();
    void requestValue();
    void readResponse();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QLineEdit *ipInput;
    QPushButton *btnConnect;
    QPushButton *btnFetch;
    QLabel *distanceLabel;
    QTcpSocket *socket;
    QTimer *timer;
    QProgressBar *progressBar;
};