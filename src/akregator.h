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
#include <kparts/browserinterface.h>
#include <kparts/browserextension.h>

class KToggleAction;
class KSqueezedTextLabel;
class KProgress;
class KParts::BrowserExtension;
class aKregator;

namespace Akregator
{
    class TrayIcon;
}

class BrowserInterface : public KParts::BrowserInterface
{
    Q_OBJECT
    Q_PROPERTY( bool haveWindowLoaded READ haveWindowLoaded )
    
public:
        BrowserInterface( aKregator *shell, const char *name );
        bool haveWindowLoaded() const;
public slots:
        void updateUnread( int );
private:
        aKregator *m_shell;
};


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
     
    bool loadPart();

    void loadLastOpenFile();

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

    virtual void fontChange(const QFont &);

    void updateUnread(int);
public slots:
    void slotSetStatusBarText(const QString & s);

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
    void optionsFetchOnStartup();

    void applyNewToolbarConfig();

    void quitProgram();
    void partChanged(KParts::ReadOnlyPart *p);
    void loadingProgress(int percent);
private:
    void setupAccel();
    void setupActions();

private:
    KParts::BrowserExtension *browserExtension(KParts::ReadOnlyPart *p);
    BrowserInterface *m_browserIface;
    KParts::ReadWritePart *m_part;
    KParts::ReadOnlyPart *m_activePart;
    KToggleAction *m_fetchStartupAction;
    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;
    KProgress *m_progressBar;
    KSqueezedTextLabel *m_statusLabel;
    Akregator::TrayIcon *m_icon;
    bool m_quit;
};

#endif // _AKREGATOR_H_
