#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    QWidget *widget = new QWidget;
    setCentralWidget(widget);

    QWidget *topFiller = new QWidget;
    topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget *bottomFiller = new QWidget;
    bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    createActions();
    createMenus();

    setWindowTitle(tr("Menus"));
    setMinimumSize(160, 160);
    resize(480, 320);
}

void MainWindow::createActions()
{
    exitAct = new QAction(tr("E&xit"), this);
    undoAct = new QAction(tr("&Undo"), this);
    redoAct = new QAction(tr("&Redo"), this);
    googleReaderSyncAct = new QAction(tr("Google Reader - account_name"), this);
    opmlSyncAct = new QAction(tr("Opml file - http://website.com/..."), this);
    manageAct = new QAction(tr("Manage..."), this);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    syncMenu = editMenu->addMenu(tr("&Synchronize"));
    syncMenu->addSeparator();
    syncMenu->addAction(googleReaderSyncAct);
    syncMenu->addAction(opmlSyncAct);
    syncMenu->addSeparator();
    syncMenu->addAction(manageAct);
}
