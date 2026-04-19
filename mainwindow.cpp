#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // UI SETUP
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // IP Address Input Row
    QHBoxLayout *ipLayout = new QHBoxLayout();
    ipInput = new QLineEdit("192.168.2.XX", this);
    btnConnect = new QPushButton("Connect", this);
    ipLayout->addWidget(new QLabel("STM32 IP:"));
    ipLayout->addWidget(ipInput);
    ipLayout->addWidget(btnConnect);

    // Display
    mainLayout->addLayout(ipLayout);

    // Window
    setCentralWidget(centralWidget);
    setWindowTitle("STM32MP1 Sensor Monitor");
    resize(400, 250);
}

MainWindow::~MainWindow()
{
}