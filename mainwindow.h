#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "caseymgmtsrv.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    CaseyMgmtSrv *caseyMgmtClients;
    CaseyMgmtSrv *caseyMgmtServer;
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
