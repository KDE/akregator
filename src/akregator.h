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

namespace KPIM
{
    class StatusbarProgressWidget;
}

namespace Akregator
{

class Part;
class TrayIcon;
class MainWindow;    

class BrowserInterface : public KParts::BrowserInterface
{
    Q_OBJECT
    Q_PROPERTY( bool haveWindowLoaded READ haveWindowLoaded )
    
public:
    BrowserInterface(Akregator::MainWindow *shell, const char *name );
    bool haveWindowLoaded() const;

private:
    Akregator::MainWindow *m_shell;
};


/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 */
class MainWindow : public KParts::MainWindow
{
    Q_OBJECT
public:

    MainWindow();
    virtual ~MainWindow();
    
    /**
     * Creates the progress widget in the status bar and the ProgressDialog
     * and connects them. 
     */
    void setupProgressWidgets();

    virtual void setCaption(const QString &);

   /**
    Loads the part
    @return Whether the part has been successfully created or not.
    */
    bool loadPart();
    
public slots:
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

private:
     void setupActions();
    void connectActionCollection(KActionCollection *coll);
    
    KParts::BrowserExtension *browserExtension(KParts::ReadOnlyPart *p);

private slots:
  
    void optionsConfigureKeys();
    void optionsConfigureToolbars();
    
    void applyNewToolbarConfig();

private:
    BrowserInterface *m_browserIface;
  
    Akregator::Part *m_part;
    KPIM::StatusbarProgressWidget *m_progressBar;
    KSqueezedTextLabel *m_statusLabel;
};

} // namespace Akregator

#endif // _AKREGATOR_H_
