#include "mainwindow.h"
#include "controlsystem.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ControlSystem CtrlSys;
    return a.exec();
}
