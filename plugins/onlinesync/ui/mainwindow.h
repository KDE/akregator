#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QActionGroup;
class QLabel;
class QMenu;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private:
    void createActions();
    void createMenus();

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *syncMenu;
    QActionGroup *alignmentGroup;
    QAction *exitAct;
    QAction *undoAct;
    QAction *redoAct;
    QAction *googleReaderSyncAct;
    QAction *opmlSyncAct;
    QAction *manageAct;
};

#endif
