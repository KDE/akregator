/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_PART_H
#define AKREGATOR_PART_H

#include <QPointer>
#include <QVector>

#include <qurl.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>

#include <kparts/readonlypart.h>

class KConfigGroup;
class KCMultiDialog;

class QTimer;

namespace Akregator
{

namespace Backend
{
class Storage;
}

class Article;
class ActionManagerImpl;
class Feed;
class FeedList;
class LoadFeedListCommand;
class MainWidget;
class Part;
class TrayIcon;

class BrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT

public:
    explicit BrowserExtension(Part *p, const char *name = 0);
public Q_SLOTS:
    void saveSettings();
private:
    Part *m_part;
};

/**
    This is a RSS Aggregator "Part". It does all the real work.
    It is also embeddable into other applications (e.g. for use in Kontact).
    */
class Part : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    typedef KParts::ReadOnlyPart inherited;

    /** Default constructor.*/
    Part(QWidget *parentWidget, QObject *parent, const QVariantList &);

    /** Destructor. */
    ~Part();

    /**
        Opens feedlist
        @param url URL to feedlist
        */
    bool openUrl(const QUrl &url);

    /** Opens standard feedlist */
    void openStandardFeedList();

    void fetchFeedUrl(const QString &);

    /** Fetch all feeds in the feed tree */
    void fetchAllFeeds();

    /**
        Add a feed to a group.
        @param url The URL of the feed to add.
        @param group The name of the folder into which the feed is added.
        If the group does not exist, it is created.  The feed is added as the last member
        of the group.
        */
    void addFeedsToGroup(const QStringList &urls, const QString &group);

    void addFeed();

    /**
        This method is called when this app is restored.  The KConfig
        object points to the session management config file that was saved
        with @ref saveProperties
        Calls Akregator MainWidget's readProperties.
        */
    virtual void readProperties(const KConfigGroup &config);

    /** This method is called when it is time for the app to save its
        properties for session management purposes.
        Calls Akregator MainWidget's saveProperties. */
    virtual void saveProperties(KConfigGroup &config);

    void exportFile(const QUrl &url);

    bool handleCommandLine();

public Q_SLOTS:
    /** Used to save settings after changing them from configuration dialog. Calls AkregatorPart's saveSettings. */
    void saveSettings();

    /** Saves the standard feed list to it's default location */
    void slotSaveFeedList();

    void fileImport();
    void fileExport();

    /** Shows configuration dialog */
    void showOptions();
    void showNotificationOptions();

    /** Call to auto save */
    void slotAutoSave();

Q_SIGNALS:
    void signalSettingsChanged();
    void signalArticlesSelected(const QList<Akregator::Article> &);

private:

    /** @return Whether the tray icon is enabled or not */
    bool isTrayIconEnabled() const;

    /** loads all plugins of a given type
     * @param type The category of plugins to load, currently one of "storage" and "extension"
     */
    void loadPlugins(const QString &type);

    /** This must be implemented by each part */
    bool openFile();

    void importFile(const QUrl &url);

    KParts::Part *hitTest(QWidget *widget, const QPoint &globalPos);

private Q_SLOTS:
    void slotStarted();

    void slotOnShutdown();
    void slotSettingsChanged();
    void slotSetStatusText(const QString &statusText);

    void feedListLoaded(const QSharedPointer<Akregator::FeedList> &list);

    void flushAddFeedRequests();

private: // methods

    /** fills the font settings with system fonts, if fonts are not set */
    void initFonts();

    bool writeToTextFile(const QString &data, const QString &fname) const;

    /**
     * This function ist called by the MainWindow upon restore
     */
    void autoReadProperties();

    /**
     * This is called when exiting akregator in order to be able to restore
     * its state next time it starts.
     */
    void autoSaveProperties();

    /**
     * Saves the session in a special file, so akregator can restore the session in
     * case of a crash.
     */
    void saveCrashProperties();

    /**
     * Tries to restore the session, if akregator crashed.
     */
    bool readCrashProperties();

    /**
     * Clears the crash data.
     */
    void clearCrashProperties();

private: // attributes

    class ApplyFiltersInterceptor;
    ApplyFiltersInterceptor *m_applyFiltersInterceptor;
    QString m_standardFeedList;
    bool m_standardListLoaded;
    bool m_shuttingDown;
    bool m_doCrashSave;

    KParts::BrowserExtension *m_extension;

    QTimer *m_autosaveTimer;
    /** did we backup the feed list already? */
    bool m_backedUpList;
    MainWidget *m_mainWidget;
    Backend::Storage *m_storage;
    ActionManagerImpl *m_actionManager;
    KCMultiDialog *m_dialog;
    struct AddFeedRequest {
        QStringList urls;
        QString group;
    };
    QPointer<LoadFeedListCommand> m_loadFeedListCommand;
    QVector<AddFeedRequest> m_requests;
};

} // namespace Akregator

#endif // AKREGATOR_PART_H
