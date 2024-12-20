#include "caseymgmtsrv.h"
#include <QTcpSocket>
#include <QDirIterator>
#include <QCryptographicHash>
#include <QSettings>
#include <QThread>
#include <QApplication>

//useful tutorial:  http://www.bogotobogo.com/Qt/Qt5_QTcpServer_Multithreaded_Client_Server.php

CaseyMgmtSrv::CaseyMgmtSrv(QObject *parent)
{
    //qsrand(QDateTime::currentMSecsSinceEpoch() / 1000);
}
//=================================================================
void CaseyMgmtSrv::Init(uint caseyMgmtServerPort, bool direction_fromSrvToCli)
{
    m_caseyMgmtServerPort=caseyMgmtServerPort;
    m_direction_fromSrvToCli=direction_fromSrvToCli;
}
//=================================================================
void CaseyMgmtSrv::StartCaseyMgmtServer()
{

    if(!this->listen(QHostAddress::Any,m_caseyMgmtServerPort))
    {
        //qDebug() << "Could not start server";
        emit workerMessage(0,"Could not start server");
    }
    else
    {
        //qDebug() << "Listening to port " << m_pHalInputServerPort << "...";
        emit workerMessage(0,"CaseyMgmtSrv started..."+QString::number(m_caseyMgmtServerPort) + "...");
        emit workerMessage(0,"Listening to port "+QString::number(m_caseyMgmtServerPort) + "...");
    }

}
//=================================================================
void CaseyMgmtSrv::StopCaseyMgmtServer()
{
    foreach(Worker *wrk, workersHash.values())
    {
        wrk->pClient->close();
    }

    if (this->isListening())
    {
        this->close();
    }

}
//=================================================================
void CaseyMgmtSrv::incomingConnection(qintptr socketDescriptor)
{
    // We have a new connection

    /* // for thread handler
    qDebug() << socketDescriptor << " Connecting workerthread...";

    WorkerThread *thread = new WorkerThread(socketDescriptor, this);
    connect(thread,SIGNAL(workerError(qintptr,QTcpSocket::SocketError)),this,SLOT(WorkerError(qintptr,QTcpSocket::SocketError)));
    connect(thread,SIGNAL(workerDisconnected(qintptr)),this,SLOT(WorkerDisconnected(qintptr)));
    connect(thread,SIGNAL(workerMessage(qintptr,QString)),this,SLOT(WorkerMessage(qintptr,QString)));
    connect(thread,SIGNAL(workerPacket(qintptr,CaseyMgmtProtHead,QByteArray)),this,SLOT(WorkerPacket(qintptr,CaseyMgmtProtHead,QByteArray));
    // connect signal/slot
    // once a thread is not needed, it will be beleted later
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();

    */

    qDebug() << socketDescriptor << " Connecting worker...";

    Worker *worker = new Worker(socketDescriptor, this);
    connect(worker,&Worker::workerError,this,&CaseyMgmtSrv::WorkerError);
    connect(worker,&Worker::workerDisconnected,this,&CaseyMgmtSrv::WorkerDisconnected);
    connect(worker,&Worker::workerMessage,this,&CaseyMgmtSrv::WorkerMessage);
    connect(worker,&Worker::workerPacket,this,&CaseyMgmtSrv::WorkerPacket);
    connect(worker,&Worker::workerDataUpdated,this,&CaseyMgmtSrv::WorkerDataUpdated);

    connect(worker,&Worker::workerDataReceived,this,&CaseyMgmtSrv::WorkerDataReceived);




    workersHash.insert(socketDescriptor,worker);
    emit workersListChanged();

    if ( workersHash.size()==1) emit hasConnectedClients(); //first client connected

}
//=================================================================
//=================================================================
CaseyMgmtSrv::~CaseyMgmtSrv()
{

}
//=================================================================
void CaseyMgmtSrv::SendDataToAllWorkers(QByteArray data)
{
    foreach(Worker *wrk, workersHash.values())
    {
        wrk->pClient->write(data);
        //wrk->wa
    }

}
//=================================================================
void CaseyMgmtSrv::SendDataToOneWorker(QByteArray data, uint32_t session_id)
{
    foreach(Worker *wrk, workersHash.values())
    {
        if (session_id==wrk->m_session_id) wrk->pClient->write(data);
        //wrk->wa
    }

}
//=================================================================
void CaseyMgmtSrv::WorkerDataUpdated(qintptr id)
{
    emit workersListChanged();
}
//====================================
void CaseyMgmtSrv::WorkerDataReceived(QByteArray data, uint32_t session_id)
{
    emit workerDataReceived(data, session_id);
}
//=================================================================
void CaseyMgmtSrv::WorkerMessage(qintptr id, QString message)
{
    emit workerMessage(id, "Worker: "+QString::number(id)+": "+message);
}
//=================================================================
void CaseyMgmtSrv::WorkerPacket(qintptr id, CaseyMgmtProtHead head, QByteArray data)
{
    qDebug() << "CaseyMgmtSrv packet:" << head.cmd << " " << head.param;
    emit workerPacket(id, head, data);
}
//=================================================================
void CaseyMgmtSrv::WorkerDisconnected(qintptr id)
{
    workersHash.remove(id);
    emit workerDisconnected(id);

    qDebug() << "server disconnrcted, workersHash.size()="+QString::number(workersHash.size());

    if ( workersHash.size()==0) emit allClientsDisconnected();
}
//=================================================================
void CaseyMgmtSrv::WorkerError(qintptr id,QTcpSocket::SocketError socketerror)
{
    emit workerError(id,socketerror);
}
//=================================================================






//====================================================================================
Worker::Worker(qintptr ID, bool direction_fromSrvToCli, QObject *parent)
{
    this->id = ID;
    m_direction_fromSrvToCli=direction_fromSrvToCli;
    qDebug() << "worker " << ID;


    pClient=new QTcpSocket();


    // set the ID
    if(!pClient->setSocketDescriptor(this->id))
    {
        // something's wrong, we just emit a signal
        emit workerError(id, pClient->error());
        return;
    }

    //pClient->setSocketOption(QAbstractSocket:: ReceiveBufferSizeSocketOption, 64 * 1024 * 1024);
    pClient->setReadBufferSize(1024*1024);

    connect(pClient, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
    connect(pClient, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(pClient, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(error(QAbstractSocket::SocketError)));

    // We'll have multiple clients, we want to know which is which
    qDebug() <<  id << "Worker Client connected";


    //ID="+QString::number(socketDescriptor)
    emit workerMessage(id, "new connection IP=" + pClient->peerAddress().toString());
    // make this thread a loop,
    // thread will stay alive so that signal/slot to function properly
    // not dropped out in the middle when thread dies



    //resolve name
    //QHostInfo::lookupHost(pClient->peerAddress().toString(),
    //                      this, SLOT(lookedUpHost(QHostInfo)));



}
//====================================================================================
Worker::~Worker()
{

}

//====================================================================================
void Worker::error(QAbstractSocket::SocketError err)
{
    qDebug() << "worker Error:" << err;
    emit workerError(id, err);
}
//====================================================================================
void Worker::lookedUpHost(const QHostInfo &host)
{
    if (host.error() != QHostInfo::NoError) {
        //qDebug() << "Lookup failed:" << host.errorString();
        emit workerMessage(id, "resolve IP "+host.hostName()+" error");
        return;
    }

    const auto addresses = host.addresses();
    for (const QHostAddress &address : addresses)
    {
       // qDebug() << "Found address:" << address.toString();
       emit workerMessage(id, "resolve IP "+address.toString()+" === "+ host.hostName());
    }
}
//====================================================================================

//====================================================================================
void Worker::readyRead()
{

    emit workerDataReceived(pClient->readAll()); //- work!!! but without logic based on session id


/*
    QByteArray ba=pClient->readAll();



    if (!m_direction_fromSrvToCli)   //data from client to server
    {

        qDebug() << "worker data direction cli to serv" << m_session_id;
        if(!initCommandReceived)   //first packet from client - save session_id
        {


            // read the init data from the socket
            if (ba.size() == sizeof(CaseyMgmtProtHead) + sizeof(CmdInitData))
            {
                CaseyMgmtProtHead head;
                //CmdInitData initData;

                //QByteArray data;

                memcpy(&head,ba.data(),sizeof(CaseyMgmtProtHead));

                if (head.protid==CMD_CLI_TO_SRV && head.datalen==sizeof(CmdInitData) && head.cmd==CaseyMgmtCmds::Init)
                {
                    initCommandReceived=true;
                    m_session_id=head.session_id;
                    qDebug() << "worker init received " << m_session_id;
                    emit workerDataReceived(ba);
                }
                else
                {
                    pClient->disconnectFromHost(); //ne nash client
                    return;
                }

            }
        }
        else
        {
            emit workerDataReceived(ba); //if initCommandReceived and session_id known - send all data
        }

    }
    else
    {
        qDebug() << "worker data direction serv to cli" << m_session_id;
        emit workerDataReceived(ba);
    }



    if (m_direction_fromSrvToCli)   //command from server which we need to send only to client with yhis session_id
    {
        // read the init data from the socket
        if (ba.size() >= sizeof(CaseyMgmtProtHead))
        {
            CaseyMgmtProtHead head;
            //CmdInitData initData;

            //QByteArray data;

            memcpy(&head,ba.data(),sizeof(CaseyMgmtProtHead));

            if (head.protid==CMD_SRV_TO_CLI)
            {

                emit workerDataReceived(ba,head.session_id); // to send to only this client
            }


        }
    }

*/

}
//====================================================================================
void Worker::disconnected()
{
    qDebug() << id << " Disconnected";
    emit workerMessage(id, pClient->peerAddress().toString()+" disconnnected");
    emit workerDisconnected(id);

    pClient->deleteLater();
    //exit(0);  // stop event loop and stop thread
}
//====================================================================================





