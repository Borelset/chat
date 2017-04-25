#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTextBrowser>
#include "network.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    int cpid = -1;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_sendBtn_clicked();
    int recv_process();

private:
    Ui::MainWindow *ui;
};

void* recv_process(void* para);


#endif // MAINWINDOW_H
