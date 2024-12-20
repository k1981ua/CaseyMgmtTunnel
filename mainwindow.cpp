#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType<CaseyMgmtProtHead>();

    //server serve clients


    //QSettings settings(qApp->applicationDirPath()+"/HalInputServer.ini",QSettings::IniFormat);
    //settings.beginGroup("Server");
    uint caseyMgmtPortForClients=7009;//8557;//settings.value("ServerPort",8555).toInt();
    uint caseyMgmtPortForServer=7008;
    //if (halInputServerPort==0) break;


    caseyMgmtClients=new CaseyMgmtSrv();
    caseyMgmtClients->Init(caseyMgmtPortForClients, false);

    caseyMgmtServer=new CaseyMgmtSrv();
    caseyMgmtServer->Init(caseyMgmtPortForServer, true);



    connect(caseyMgmtServer, &CaseyMgmtSrv::hasConnectedClients,caseyMgmtClients,&CaseyMgmtSrv::StartCaseyMgmtServer);
    connect(caseyMgmtServer, &CaseyMgmtSrv::allClientsDisconnected, caseyMgmtClients,&CaseyMgmtSrv::StopCaseyMgmtServer);

    connect(caseyMgmtClients,&CaseyMgmtSrv::workerDataReceived, caseyMgmtServer,  &CaseyMgmtSrv::SendDataToAllWorkers);  //To_DO from clients messages to all servers (which may be one)
    connect(caseyMgmtServer, &CaseyMgmtSrv::workerDataReceived, caseyMgmtClients, &CaseyMgmtSrv::SendDataToAllWorkers);  //TO DO: from server commands to one client with needed m_session_id

    caseyMgmtServer->StartCaseyMgmtServer();


}

MainWindow::~MainWindow()
{
    delete ui;
}
