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
#include <kio/job.h>

class KActionCollection;
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
		void newArticle(const QString&, const QPixmap&, const QString&);

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

    /** Loads standard feed file */
    void loadStandardFile();

    /**
     * Default Destructor
     */
    virtual ~aKregator();

    /**
     * Use this method to load whatever file/URL you have.
     */
    void load(const KURL& url);

    /**
      * Add a feed to a group.
      * @param url The URL of the feed to add.
      * @param group The name of the folder into which the feed is added.
      *
      * If the group does not exist, it is created.  The feed is added as the last member
      * of the group.
      */
    void addFeedToGroup(const QString& url, const QString& group);
    
    virtual void fontChange(const QFont &);
    virtual void setCaption(const QString &);

    void updateUnread(int);
    void newArticle(const QString&, const QPixmap&, const QString&);

public slots:
    void slotSetStatusBarText(const QString & s);
    void slotActionStatusText(const QString &s);
    void slotClearStatusText();

signals:
    void markAllFeedsRead();

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
    /** 
     *  reimplemented to save settings 
     */
    virtual bool queryExit();
      
    virtual bool queryClose(); 
     
private slots:
    void fileNew();
    void fileOpen();
    void optionsShowToolbar();
    void optionsShowStatusbar();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();
    void showOptions();

    void applyNewToolbarConfig();

    void quitProgram();
    void partChanged(KParts::ReadOnlyPart *p);
    void loadingProgress(int percent);
    void slotStop();
    void slotStarted(KIO::Job *);
    void slotCanceled(const QString &);
    void slotCompleted();

private:
    void callObjectSlot( QObject *obj, const char *name, const QVariant &argument );

    void setupAccel();
    void setupActions();
    void connectActionCollection(KActionCollection *coll);
    void disconnectActionCollection(KActionCollection *coll);

    KParts::BrowserExtension *browserExtension(KParts::ReadOnlyPart *p);
    BrowserInterface *m_browserIface;
    KParts::ReadWritePart *m_part;
    KParts::ReadOnlyPart *m_activePart;
    KToggleAction *m_fetchStartupAction;
    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;
    KAction *m_stopAction;
    KProgress *m_progressBar;
    KSqueezedTextLabel *m_statusLabel;
    Akregator::TrayIcon *m_icon;
    QString m_permStatusText;
};

#endif // _AKREGATOR_H_
