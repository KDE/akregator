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
class KParts::PartManager;

class AkregatorMainWindow;

namespace KPIM
{
    class StatusbarProgressWidget;
}

namespace Akregator
{
    class aKregatorPart;
    class TrayIcon;
    
}

class BrowserInterface : public KParts::BrowserInterface
{
    Q_OBJECT
    Q_PROPERTY( bool haveWindowLoaded READ haveWindowLoaded )
    
public:
        BrowserInterface( AkregatorMainWindow *shell, const char *name );
        bool haveWindowLoaded() const;

private:
        AkregatorMainWindow *m_shell;
};


/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 */
class AkregatorMainWindow : public KParts::MainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    AkregatorMainWindow();
    
    /**
     * Loads the part
     * @return Whether the part has been successfully created or not.
     */
    bool loadPart();

    /**
     * Creates the progress widget in the status bar and the ProgressDialog
     * and connects them. 
     */
    void setupProgressWidgets();

    /** 
     * Loads standard feed list
    */
    void loadStandardFile();

    /**
     * Default Destructor
     */
    virtual ~AkregatorMainWindow();

    /**
     * Use this method to load whatever file/URL you have.
     */
    //void load(const KURL& url);

    /**
      * Add a feed to a group.
      * @param url The URL of the feed to add.
      * @param group The name of the folder into which the feed is added.
      *
      * If the group does not exist, it is created.  The feed is added as the last member
      * of the group.
      */
    void addFeedToGroup(const QString& url, const QString& group);
    
    virtual void setCaption(const QString &);


public slots:
    void slotSetStatusBarText(const QString & s);
    void slotActionStatusText(const QString &s);
    void slotClearStatusText();

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
     *  Reimplemented to save settings
     */
    virtual bool queryExit();
      
    /**
     * Reimplemented to say if app will be running in system tray if necessary
     */
    virtual bool queryClose(); 
     
private slots:
    void fileNew();
    void fileOpen();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();
    
    void applyNewToolbarConfig();

    //void partChanged(KParts::ReadOnlyPart *p);
    //void loadingProgress(int percent);

private:
    void callObjectSlot( QObject *obj, const char *name, const QVariant &argument );

    void setupActions();
    void connectActionCollection(KActionCollection *coll);
    void disconnectActionCollection(KActionCollection *coll);

    KParts::BrowserExtension *browserExtension(KParts::ReadOnlyPart *p);
    BrowserInterface *m_browserIface;
    /**
     * Our part
     */
    Akregator::aKregatorPart *m_part;
    KParts::PartManager* m_manager;
    //KParts::ReadOnlyPart *m_activePart;
    KPIM::StatusbarProgressWidget *m_progressBar;
    KSqueezedTextLabel *m_statusLabel;
    QString m_permStatusText;
};

#endif // _AKREGATOR_H_
