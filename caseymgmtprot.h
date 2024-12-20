#ifndef CASEYMGMTPROT_H
#define CASEYMGMTPROT_H

#include <QMetaType>

#define CMD_SRV_TO_CLI 0xCA5E452C
#define CMD_CLI_TO_SRV 0xCA5E4C25


enum CaseyMgmtCmds: uint32_t
{
    Init=1,
    Message,
    Close,
    Update,
    KeepAlive,
    GetNumScrs,
    GetScrOnce, // param=i - loop get i screen
    GetScrLoop, // param=i - loop get i screen
    ExecCmd, // param=0 data=command
    ExecCmdPS, // param=0 data=command in powershell
    GetFile, //  param=0 data=fullpath
    GetFileWeb, // param=0  data=url
    SetMousePos, // param=i - numscreen data = 2 ints x and y
    MouseLeftClick, // param=0
    MouseRightClick, // param=0
    MouseLeftDblCli
};
Q_DECLARE_METATYPE(CaseyMgmtCmds)


// not crypted
struct CaseyMgmtProtHead  //20 bytes
{
    uint32_t protid;  // 0xCA5E4C25  - > cli 2 srv    0xCA5E452C -> srv 2 cli
    uint32_t session_id; //random 32 bit number also use as cryptobase
    uint32_t datalen;
    CaseyMgmtCmds cmd;
    int32_t param;   //for small params or reserved

};
Q_DECLARE_METATYPE(CaseyMgmtProtHead)

// commands:
// init
// message
// update
// keepAlive
// getNumScrs - get num screens
// getScrOnce param=i - loop get i screen
// getScrLoop param=i - loop get i screen
// execCmd param=0 data=command
// execCmdPS param=0 data=command in powershell
// getFile  param=0 data=fullpath
// getFileWeb param=0  data=url
// setMousePos param=i - numscreen data = 2 ints x and y
// mouseLeftClick param=0
// mouseRightClick param=0
// mouseLeftDblCli



struct CmdInitData
{
    char host[32];
    char user[32];
    char desc[32];
};

struct CurrDownloadingData
{
    CaseyMgmtProtHead head={0};
    QByteArray data;
    bool inProcess=false;
};



//======================================================================

#endif // CASEYMGMTPROT_H
