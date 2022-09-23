#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMetaType>
#include <QSet>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void newMessage(QString);

private slots:
    void newConnection();
    void appendToSocketList(QTcpSocket *socket);

    void readSocket();
    void discardSocket();
    void displayError(QAbstractSocket::SocketError socketError);

    void displayMessage(const QString& str);
    void sendMessage(QTcpSocket *socket);
//    void sendAttachment(QTcpSocket *socket, QString filePath);
    void refreshComboBox();

    void on_sendButton_clicked();

private:
    Ui::MainWindow *ui;

    QTcpServer *mServer;
    QSet<QTcpSocket*> connectionSet;

    QDataStream* socketStream;
};

#endif // MAINWINDOW_H
