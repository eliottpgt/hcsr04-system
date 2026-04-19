#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
};