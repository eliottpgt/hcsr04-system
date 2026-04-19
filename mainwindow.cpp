#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // UI SETUP
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);


    // Window
    setCentralWidget(centralWidget);
    setWindowTitle("STM32MP1 Sensor Monitor");
    resize(400, 250);
}

MainWindow::~MainWindow()
{
}