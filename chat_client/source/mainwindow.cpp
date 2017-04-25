#include <QString>
#include <QUdpSocket>
#include <string>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "network.h"

using namespace std;

extern network conn;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QString myname(conn.login_username);
    ui->username_label->setText(myname);
    connect(&conn.q_trans_sock, SIGNAL(readyRead()), this, SLOT(recv_process()));
}

int MainWindow::recv_process()
{
    QString msg(conn.recvmsg().c_str());
    ui->recvBrowser->append(msg);
    return 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sendBtn_clicked()
{
    QString q_twds = ui->twdsEdit->text();
    if(q_twds.isEmpty())
    {
        return;
    }
    string twds = q_twds.toStdString();

    QString q_msg = ui->msgEdit->text();
    if(q_msg.isEmpty())
    {
        return;
    }
    string msg = q_msg.toStdString();

    string display;
    display = display + conn.login_username + ">" + twds + " " + msg;
    QString q_display(display.c_str());
    ui->recvBrowser->append(q_display);
    conn.sendmsg(twds.c_str(), msg.c_str());

    ui->msgEdit->setText(NULL);
}
