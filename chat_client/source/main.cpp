#include <QApplication>
#include <QDialog>
#include "mainwindow.h"
#include "logindialog.h"
#include "network.h"

network conn;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginDialog dlg;
    if(dlg.exec() == QDialog::Accepted)
    {
        MainWindow w;
        w.show();
        return a.exec();
    }
    else
    {
        return 0;
    }

}
