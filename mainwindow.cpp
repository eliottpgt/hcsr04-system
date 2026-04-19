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

    // Progress Bar
    progressBar = new QProgressBar(this);
    progressBar->setRange(1, 150);
    progressBar->setEnabled(false);

    // Check Box
    checkStats = new QCheckBox("Stats Mesure",this);
    checkStats->setEnabled(false);
    checkStats->setCheckState(Qt::Unchecked);

    // Display
    distanceLabel = new QLabel("Disconnected", this);
    distanceLabel->setAlignment(Qt::AlignCenter);
    distanceLabel->setStyleSheet("font-size: 35px; font-weight: bold; color: #7f8c8d;");

    sdLabel = new QLabel(this);
    sdLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(ipLayout);
    mainLayout->addWidget(btnFetch);
    mainLayout->addWidget(checkStats);
    mainLayout->addWidget(distanceLabel);
    mainLayout->addWidget(sdLabel);
    mainLayout->addWidget(progressBar);

    // Window
    setCentralWidget(centralWidget);
    setWindowTitle("STM32MP1 Sensor Monitor");
    resize(400, 250);

    // --- NETWORK & TIMER SETUP ---
    socket = new QTcpSocket(this);
    timer = new QTimer(this);

    // Init buffer
    buffer = std::make_unique<CircularBuffer<20>>();

    // Signals Connections
    connect(btnConnect, &QPushButton::clicked, this, &MainWindow::toggleConnection);
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(btnFetch, &QPushButton::clicked, this, &MainWindow::togglePolling);
    connect(timer, &QTimer::timeout, this, &MainWindow::requestValue);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readResponse);
    connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::onError);
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
    checkStats->setEnabled(true);
    progressBar->setEnabled(true);
    distanceLabel->setText("Connected");
    distanceLabel->setStyleSheet("font-size: 35px; font-weight: bold; color: #2980b9;");
}

void MainWindow::onDisconnected() {
    timer->stop();
    btnConnect->setText("Connect");
    btnConnect->setEnabled(true);
    btnFetch->setEnabled(false);
    checkStats->setEnabled(false);
    progressBar->setEnabled(false);
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

void MainWindow::readResponse() {
    QByteArray data = socket->readLine().trimmed();
    qDebug() << "Données reçues non valides :" << data;
    if (data.isEmpty()) return;

    bool ok;
    int value = data.toInt(&ok);
    
    if (ok) {
        updateUI(value);
    }
}

void MainWindow::updateUI(int value) {
    progressBar->setValue(value);

    if (checkStats->isChecked()) {
        buffer->add(static_cast<uint16_t>(value));
        
        distanceLabel->setText(QString("Mean: %1 cm").arg(buffer->getMean(), 0, 'f', 1));
        sdLabel->setText(QString("SD: %1").arg(buffer->getStandardDeviation(), 0, 'f', 2));
        
        distanceLabel->setStyleSheet("font-size: 30px; font-weight: bold; color: #575757;");
    } else {
        distanceLabel->setText(QString::number(value) + " cm");
        sdLabel->clear();
        distanceLabel->setStyleSheet("font-size: 35px; font-weight: bold; color: #2ecc71;");
    }
}

void MainWindow::onError(QAbstractSocket::SocketError socketError) {
    QString errorMsg = socket->errorString();
    distanceLabel->setText("Error: " + errorMsg);

    qDebug() << "Socket Error:" << errorMsg;

    btnConnect->setText("Connect");
    btnConnect->setEnabled(true);
    distanceLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #e74c3c;");
}

MainWindow::~MainWindow()
{
}