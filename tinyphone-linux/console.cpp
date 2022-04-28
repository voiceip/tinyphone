// console.cpp : Defines the entry point for the console application.

#include <stdio.h>
#include <ctime>
#include <algorithm>
#include <vector>
#include <iostream>

#include "server.h"
#include "utils.h"
#include "net.h"
#include "consts.h"
#include "config.h"
#include "log.h"
#include "app.hpp"
#include "tpendpoint.h"

#include <qapplication.h>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QCloseEvent>
#include <QtCore/QDebug>

using namespace std;
using namespace pj;
using namespace tp;


void showIcon(){
    // icn=QIcon("icon.png");
    // icon.setIcon(icn);
    // icon.show();

    // QMessageBox::StandardButton reply;
    //   reply = QMessageBox::question("Test", "Quit?",
    //                                 QMessageBox::Yes|QMessageBox::No);
    //   if (reply == QMessageBox::Yes) {
    //     qDebug() << "Yes was clicked";
    //     QApplication::quit();
    //   } else {
    //     qDebug() << "Yes was *not* clicked";
    //   }

    auto exitAction = new QAction("&Exit");
    // connect(exitAction, &QAction::triggered, [this]()
    // {
    //     // closing = true;
    //     // close();
    // });

    auto trayIconMenu = new QMenu();
    trayIconMenu->addAction(exitAction);


    auto sysTrayIcon = new QSystemTrayIcon();
    sysTrayIcon->setContextMenu(trayIconMenu);
    sysTrayIcon->setIcon(QIcon("tinyphone.ico"));
    sysTrayIcon->show();

}

void Start(){
   tp::StartApp();
   exit(0);
}

void Stop(){
    tp::StopApp();
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QPixmap oPixmap(32,32);
    oPixmap.load ("icon.png");
    QIcon oIcon(oPixmap);


    auto exitAction = new QAction("&Exit");
    connect(exitAction, &QAction::triggered, [](){
        // closing = true;
        // close();
    });

    auto trayIconMenu = new QMenu();
    trayIconMenu->addAction(exitAction);

    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(oIcon);
    qDebug() << trayIcon->isSystemTrayAvailable();
    trayIcon->setContextMenu( trayIconMenu);
    trayIcon->setIcon(oIcon);
    trayIcon->setVisible(true);
    trayIcon->showMessage("Test Message", "Text", QSystemTrayIcon::Information, 1000);


    std::cout << "Hello World!\n";
    // Start();
    return app.exec();
}