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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QLineEdit *ipInput;
    QPushButton *btnConnect;
    QPushButton *btnFetch;
    QLabel *distanceLabel;
    QTcpSocket *socket;
    QTimer *timer;
};