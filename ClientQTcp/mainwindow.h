#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractSocket>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QHostAddress>
#include <QMessageBox>
#include <QMetaType>
#include <QString>
#include <QStandardPaths>
#include <QTcpSocket>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

   QString *username;

signals:
    void newMessage(QString);

private slots:

    void connectSignals();

    void readSocket();

    void discardSocket();
    void displayError(QAbstractSocket::SocketError socketError);

    void displayMessage(const QString& str);

    void on_sendButton_clicked();

    void on_connectButton_toggled(bool checked);

private:
    Ui::MainWindow *ui;

    QTcpSocket* socket;
    QByteArray* header;
};

#endif // MAINWINDOW_H
