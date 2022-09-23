#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->connectButton->setCheckable(true);

    connect(this, SIGNAL(newMessage(QString)), this, SLOT(displayMessage(QString)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectSignals()
{
    connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(discardSocket()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
}

void MainWindow::readSocket()
{
    QByteArray buffer;

    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_5_2);

    socketStream.startTransaction();
    socketStream >> buffer;

    if(!socketStream.commitTransaction())
    {
        QString message =   QString("Client %1 :: Waiting for more data to come...").arg(socket->socketDescriptor());
        emit newMessage(message);
        return;
    }

    QString header  =   buffer.mid(0,128);
    QString fileType    =   header.split(",")[0].split(":")[1];
    QString username    =   header.split(",")[1];

    qDebug()    <<  username;

    buffer  =   buffer.mid(128);
    if(fileType == "message")
    {
        QString message =   QString("%1 :: %2").arg(socket->socketDescriptor()).arg(QString::fromStdString(buffer.toStdString()));
        emit newMessage(message);
    }
}

void MainWindow::discardSocket()
{
    if(socket)
    {
        socket->deleteLater();
        socket  =   nullptr;

        ui->statusBar->showMessage("Disconnected!");
    }
}
void MainWindow::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
        break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, "QTCPClient", "The host was not found. Please check the host name and port settings.");
        break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, "QTCPClient", "The connection was refused by the peer. Make sure QTCPServer is running, and check that the host name and port settings are correct.");
        break;
        default:
            QMessageBox::information(this, "QTCPClient", QString("The following error occurred: %1.").arg(socket->errorString()));
        break;
    }
}

void MainWindow::displayMessage(const QString& str)
{
    ui->textBrowser->append(str);
}

void MainWindow::on_sendButton_clicked()
{
    if(socket)
    {
        if(socket->isOpen())
        {
            QString text =   ui->lineEdit->text();
            QString username    =   ui->username->text();
            QDataStream socketStream(socket);
            socketStream.setVersion(QDataStream::Qt_5_2);

            QString str =   QString("fileType:message,username:%1").arg(username);

            QByteArray  header;
            header  =   (str.toUtf8());
//            header.prepend(QString("fileType:message,username:%1,filename:null,filesize:%2;").arg(username).arg(str.size()).toUtf8());
            header.resize(128);

            QByteArray  byteArray   =   text.toUtf8();
            byteArray.prepend(header);

            socketStream    <<  byteArray;

            ui->lineEdit->clear();
        }
        else
        {
            QMessageBox::critical(this, "QTCPClient", "Socket doesn't seem to be opened");
        }
    }
    else
        QMessageBox::critical(this, "QTCPClient", "Not connected");
}

void MainWindow::on_connectButton_toggled(bool checked)
{
    if(checked == true)
    {
        socket  =   new QTcpSocket(this);
        connectSignals();
    }

    if(socket)
    {
        if(checked == true)
        {
            ui->connectButton->setText("Disconnect");

            socket->connectToHost(QHostAddress::LocalHost, 1234);

            if(socket->waitForConnected())
            {
                ui->statusBar->showMessage("Connected to server");

                QString username    =   ui->username->text();

                QString str = QString("fileType:message,username:%1,filename:null,fileSize:%2;").arg(username).arg(username.size());

                header = new QByteArray;

                header->prepend(str.toUtf8());
                // split potrzebny do wrzucenia przy nowym połączeniu do serwera
                QString string = header->split(',')[1].split(':')[1];

            }
            else
            {
                QDataStream socketStream(socket);
                socketStream.setVersion(QDataStream::Qt_5_2);
                QMessageBox::critical(this, "QTCPClient", QString("The following error occured: %1.").arg(socket->errorString()));

                ui->connectButton->setChecked(false);
                ui->connectButton->setText("Connect");
            }
        }
        if(checked==false)
        {
            ui->connectButton->setChecked(false);
            ui->connectButton->setText("Connect");
            socket->disconnected();
        }
    }
}
