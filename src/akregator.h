/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef _AKREGATOR_H_
#define _AKREGATOR_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kparts/mainwindow.h>

class KToggleAction;

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 */
class aKregator : public KParts::MainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    aKregator();

    /**
     * Default Destructor
     */
    virtual ~aKregator();

    /**
     * Use this method to load whatever file/URL you have.
     */
    void load(const KURL& url);

    /**
     * Reimplemented from QWidget to prevent from closing when docked.
     */
    virtual void closeEvent(QCloseEvent* e);

protected:
    /**
     * This method is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties(KConfig *);

    /**
     * This method is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties(KConfig *);
    
private slots:
    void fileNew();
    void fileOpen();
    void optionsShowToolbar();
    void optionsShowStatusbar();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();

    void applyNewToolbarConfig();

    void quitProgram();
    void partChanged(KParts::Part *p);

private:
    void setupAccel();
    void setupActions();

private:
    KParts::ReadWritePart *m_part;

    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;

    bool m_quit;
};

#endif // _AKREGATOR_H_
