#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mServer = new QTcpServer();

    if( mServer->listen(QHostAddress::Any, 1234) )
    {
        connect(this, SIGNAL(newMessage(QString)), this, SLOT(displayMessage(QString)));
        /*
         *  lub można to zapisać jako:
         *  connect(this, &MainWindow::newMessage, this, &MainWindow::displayMessage);
        */
        connect(mServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
        /*
         *  lub można to zapisać jako:
         *   connect(m_server, &QTcpServer::newConnection, this, &MainWindow::newConnection);
        */
        ui->statusBar->showMessage("Server is listening...");// wyświetlanie
    }
    else
    {
        QMessageBox::critical(this, "QTCPServer", QString("Unable to start the server: %1.").arg(mServer->errorString())); // Wyświetlanie Erroru
        exit(EXIT_FAILURE);
    }
}

MainWindow::~MainWindow()
{
    // zamykanie socketów i serwera
    foreach(QTcpSocket *socket, connectionSet)
    {
        socket->close();
        socket->deleteLater();
    }
    mServer->close();
    mServer->deleteLater();

    delete ui;
}


void MainWindow::newConnection()
{
    while ( mServer->hasPendingConnections())
        appendToSocketList(mServer->nextPendingConnection()); // Dodawanie kolejnych połączeń do listy socketów
}

void MainWindow::appendToSocketList(QTcpSocket *socket)
{
    connectionSet.insert(socket); // Dodanie socketu do tabeli
    connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket())); // Połączenie sygnału gotowości na odczyt z klasą odczytującą socket
    connect(socket, SIGNAL(disconnected()), this, SLOT(discardSocket())); // Połączenie sygnału rozłączenia z klasą zamykającą socket
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError))); // Połączenie sygnału błędu z wyświetlaniem błędów

    ui->comboBox->addItem(QString::number(socket->socketDescriptor())); // Dodawanie numeru socketu do listy w comboBoxie
    displayMessage(QString("INFO :: Client with socketID: %1 joined").arg(socket->socketDescriptor()));
}

void MainWindow::readSocket()
{
    QTcpSocket  *socket = reinterpret_cast<QTcpSocket*>(sender()); // Przypisanie do wskaźnik kto wysyła wiadomość

    QByteArray  buffer;

    QDataStream socketStream(socket); // Tworzenie socketStreama
    socketStream.startTransaction();
    socketStream >> buffer;

    if(!socketStream.commitTransaction())
    {
        QString message =   QString("Client nr.%1 :: Waiting for more data to come...").arg(socket->socketDescriptor());
        emit newMessage(message);
        return;
    }

    QString header   =  buffer.mid(0,128); // odczyt bufferu
    QString fileType =  header.split(",")[0].split(":")[1]; // rozdzielenie "fileType:message,filename:null,filesize:%1;"


    buffer  =   buffer.mid(128);

    qDebug() << buffer.mid(128);

    if(fileType=="message")
    {
        QString message =   QString("%1 :: %2").arg(socket->socketDescriptor()).arg(QString::fromStdString(buffer.toStdString()));
        emit newMessage(message);
    }
    //else można dorobić żeby przesyłać pliki ( filetype==attachment )

}

void MainWindow::discardSocket()
{
    QTcpSocket* socket  =   reinterpret_cast<QTcpSocket*>(sender());
    QSet<QTcpSocket*>::iterator it  =   connectionSet.find(socket);
    if  (it !=  connectionSet.end())
    {
        displayMessage(QString("INFO :: A client nr.%1 has left the room").arg(socket->socketDescriptor()));
        connectionSet.remove(*it);
    }
    refreshComboBox();

    socket->deleteLater();
}

void MainWindow::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, "QTCPServer", "The host was not found. Please check the host name and port settings.");
            break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, "QTCPServer", "The connection was refused by the peer. Make sure QTCPServer is running, and check that the host name and port settings are correct.");
        break;
        default:
            QTcpSocket* socket  =   qobject_cast<QTcpSocket*>(sender());
            QMessageBox::information(this, "QTCPServer", QString("The following error occured: %1.").arg(socket->errorString()));
        break;
    }
}

void MainWindow::displayMessage(const QString& str)
{
    ui->textBrowser->append(str); // dodanie kolejnej wiadomości
}

void MainWindow::sendMessage(QTcpSocket *socket)
{
    if(socket)
    {
        if(socket->isOpen())
        {
            QString str =   ui->lineEdit->text();

            QDataStream socketStream(socket);
            socketStream.setVersion(QDataStream::Qt_5_2);

            QByteArray header;
            header.prepend(QString("fileType:message,filename:null,filesize:%1;").arg(str.size()).toUtf8());
            header.resize(128);

            QByteArray byteArray    =   str.toUtf8();
            byteArray.prepend(header);

            socketStream.setVersion(QDataStream::Qt_5_2);
            socketStream << byteArray;

        }
        else
            QMessageBox::critical(this,"QTCPServer","Socket doesn't seem to be opened");
    }
    else
        QMessageBox::critical(this,"QTCPServer","Not connected");
}

void MainWindow::on_sendButton_clicked()
{
    QString receiver    =   ui->comboBox->currentText();

    if(receiver ==  "Broadcast")
    {
        foreach(QTcpSocket* socket, connectionSet)
        {
            sendMessage(socket);
        }
    }
    else
    {
        foreach (QTcpSocket* socket, connectionSet)
        {
            if(socket->socketDescriptor() == receiver.toLongLong())
            {
                sendMessage(socket);
                break;
            }
        }
    }
    ui->lineEdit->clear();
}

void MainWindow::refreshComboBox(){
    ui->comboBox->clear();
    ui->comboBox->addItem("Broadcast");
    foreach(QTcpSocket* socket, connectionSet)
        ui->comboBox->addItem(QString::number(socket->socketDescriptor()));
}
