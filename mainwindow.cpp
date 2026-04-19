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

    // Control Button
    btnFetch = new QPushButton("Start Measure", this);
    btnFetch->setEnabled(false); // Disabled until connected

    // Display
    distanceLabel = new QLabel("Disconnected", this);
    distanceLabel->setAlignment(Qt::AlignCenter);
    distanceLabel->setStyleSheet("font-size: 35px; font-weight: bold; color: #7f8c8d;");

    mainLayout->addLayout(ipLayout);
    mainLayout->addWidget(btnFetch);
    mainLayout->addWidget(distanceLabel);

    // Window
    setCentralWidget(centralWidget);
    setWindowTitle("STM32MP1 Sensor Monitor");
    resize(400, 250);
}

MainWindow::~MainWindow()
{
}