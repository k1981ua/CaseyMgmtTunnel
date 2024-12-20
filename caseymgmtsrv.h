#ifndef CASEYMGMTSRV_H
#define CASEYMGMTSRV_H

#include <QObject>
#include <QTcpServer>
#include <QDateTime>
#include <QList>
#include <QThread>
#include <QTcpSocket>
#include <QHostInfo>
#include "caseymgmtprot.h"


class Worker;


//=======================================================
class CaseyMgmtSrv : public QTcpServer
{
    Q_OBJECT
public:
    explicit CaseyMgmtSrv(QObject *parent = 0);
    virtual ~CaseyMgmtSrv();
    void Init(uint caseyMgmtServerPort, bool direction_fromSrvToCli);

    QHash<qintptr,Worker*> workersHash;

    bool m_direction_fromSrvToCli=false;

    //void crypt_decrypt(CaseyMgmtProtHead &head, QByteArray data);


private:
    uint m_caseyMgmtServerPort;


protected:
    void incomingConnection(qintptr socketDescriptor) override;

signals:
    void workerMessage(qintptr id,QString message);
    void workerPacket(qintptr id,CaseyMgmtProtHead head, QByteArray data);
    void workerDisconnected(qintptr id);
    void workerError(qintptr id,QTcpSocket::SocketError socketerror);
    void workersListChanged();

    void workerDataReceived(QByteArray data, uint32_t session_id);

    void hasConnectedClients();
    void allClientsDisconnected();

public slots:

    void StartCaseyMgmtServer();
    void StopCaseyMgmtServer();


    void WorkerMessage(qintptr id,QString message);
    void WorkerPacket(qintptr id, CaseyMgmtProtHead head, QByteArray data);
    void WorkerDisconnected(qintptr id);
    void WorkerError(qintptr id,QTcpSocket::SocketError socketerror);
    void WorkerDataUpdated(qintptr id); //emit after init packet in which client send their info

    void WorkerDataReceived(QByteArray data, uint32_t session_id);

    void SendDataToAllWorkers(QByteArray data);
    void SendDataToOneWorker(QByteArray data, uint32_t session_id);
};


//======================================================================

class Worker : public QObject
{
    Q_OBJECT
public:
    Worker(qintptr ID, bool direction_fromSrvToCli, QObject *parent = 0);
    //Worker() = default;
    //Worker(const Worker&) = default;
    ~Worker();

    QTcpSocket *pClient;
    qintptr id;

    bool m_direction_fromSrvToCli=false;

    QString m_host;
    QString m_user;
    QString m_desc;
    uint32_t m_session_id;  //also cryptobase

    bool initCommandReceived=false;


signals:
    void workerError(qintptr id, QTcpSocket::SocketError socketerror);
    void workerDisconnected(qintptr id);
    void workerMessage(qintptr id, QString message);
    void workerPacket(qintptr id, CaseyMgmtProtHead head, QByteArray data);
    void workerDataUpdated(qintptr id);  //emit after init packet in which client send their info

    void workerDataReceived(QByteArray data, uint32_t session_id=0);


public slots:
    void readyRead();
    void disconnected();
    void error(QAbstractSocket::SocketError err);
    void lookedUpHost(const QHostInfo &host);



private:


};
//=====================================================================




#endif // CASEYMGMTSRV_H
