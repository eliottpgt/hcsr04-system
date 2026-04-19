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

    // --- NETWORK & TIMER SETUP ---
    socket = new QTcpSocket(this);
    timer = new QTimer(this);

    // Signals Connections
    connect(btnConnect, &QPushButton::clicked, this, &MainWindow::toggleConnection);
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(btnFetch, &QPushButton::clicked, this, &MainWindow::togglePolling);
    connect(timer, &QTimer::timeout, this, &MainWindow::requestValue);
}

void MainWindow::toggleConnection(){
    if (socket->state() == QAbstractSocket::UnconnectedState) {
        QString ip = ipInput->text().trimmed();
        socket->connectToHost(ip, 8080);
        btnConnect->setText("Connecting...");
        btnConnect->setEnabled(false);
    } else {
        socket->disconnectFromHost();
    }
}

void MainWindow::onConnected() {
    btnConnect->setText("Disconnect");
    btnConnect->setEnabled(true);
    btnFetch->setEnabled(true);
    distanceLabel->setText("Connected");
    distanceLabel->setStyleSheet("font-size: 35px; font-weight: bold; color: #2980b9;");
}

void MainWindow::onDisconnected() {
    timer->stop();
    btnConnect->setText("Connect");
    btnConnect->setEnabled(true);
    btnFetch->setEnabled(false);
    btnFetch->setText("Start Measure");
    distanceLabel->setText("Disconnected");
    distanceLabel->setStyleSheet("font-size: 35px; font-weight: bold; color: #7f8c8d;");
}

void MainWindow::togglePolling() {
    if (timer->isActive()) {
        timer->stop();
        btnFetch->setText("Start Measure");
    } else {
        timer->start(50);
        btnFetch->setText("Stop Measure");
    }
}

void MainWindow::requestValue() {
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write("GET_VALUE\n");
    }
}

MainWindow::~MainWindow()
{
}