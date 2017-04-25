#include <QLineEdit>
#include <QString>
#include <QMessageBox>
#include "logindialog.h"
#include "ui_logindialog.h"
#include "network.h"

using namespace std;

extern network conn;

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_loginBtn_clicked()
{
    QString q_usrn = ui->usrEdit->text();
    QString q_pswd = ui->pswEdit->text();
    if(q_usrn.isEmpty() || q_pswd.isEmpty())
    {
        return;
    }
    string usrn = q_usrn.toStdString();
    string pswd = q_pswd.toStdString();

    int result = conn.login(usrn.c_str(), pswd.c_str());
    if(result == 1)
    {
        conn.confirm_port();
        accept();
    }
    else if(result == -1)
    {
        QMessageBox::warning(this, tr("log error"), tr("This user has already login ..."), QMessageBox::Yes);
    }
    else if(result == -2)
    {
        QMessageBox::warning(this, tr("log error"), tr("Wrong username or password ..."), QMessageBox::Yes);
    }
    else if(result == -90)
    {
        QMessageBox::warning(this, tr("log error"), tr("Cannot connect to server ..."), QMessageBox::Yes);
    }
    else
    {
        QMessageBox::warning(this, tr("log error"), tr("Unknown error ..."), QMessageBox::Yes);
    }
}

void LoginDialog::on_pushButton_clicked()
{
    QString q_usrn = ui->usrEdit->text();
    QString q_pswd = ui->pswEdit->text();
    if(q_usrn.isEmpty() || q_pswd.isEmpty())
    {
        return;
    }
    string usrn = q_usrn.toStdString();
    string pswd = q_pswd.toStdString();

    int result = conn.regist(usrn.c_str(), pswd.c_str());
    if(result == 1)
    {
        QMessageBox::warning(this, tr("regist"), tr("Success regist a new count ..."), QMessageBox::Yes);
    }
    else if(result == -1)
    {
        QMessageBox::warning(this, tr("regist error"), tr("Already used username ..."), QMessageBox::Yes);
    }
    else
    {
        QMessageBox::warning(this, tr("regist error"), tr("Unknown error ..."), QMessageBox::Yes);
    }
}
