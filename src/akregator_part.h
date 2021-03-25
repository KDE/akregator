/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <KSharedConfig>
#include <QPointer>
#include <QVector>

#include <QUrl>
#include <kparts/part.h>

#include "crashwidget/crashwidget.h"

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
class AkregatorCentralWidget;

/**
    This is a RSS Aggregator "Part". It does all the real work.
    It is also embeddable into other applications (e.g. for use in Kontact).
    */
class Part : public KParts::Part
{
    Q_OBJECT
public:
    /** Default constructor.*/
    Part(QWidget *parentWidget, QObject *parent, const QVariantList &);

    /** Destructor. */
    ~Part() override;

    /** Opens standard feedlist */
    void openStandardFeedList();

    void fetchFeedUrl(const QString &);

    /** Fetch all feeds in the feed tree */
    void fetchAllFeeds();

    /**
        Add a feed to a group.
        @param urls The URLs of the feeds to add.
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

    void exportFile(const QString &str); /* for the DBus adaptor */
    void exportFile(const QUrl &url);

    bool handleCommandLine(const QStringList &args);

    KSharedConfig::Ptr config();
    void updateQuickSearchLineText();
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
    void signalArticlesSelected(const QVector<Akregator::Article> &);

private:
    /** @return Whether the tray icon is enabled or not */
    bool isTrayIconEnabled() const;

    /** loads all plugins of a given type
     * @param type The category of plugins to load, currently one of "storage" and "extension"
     */
    void loadPlugins(const QString &type);

    void importFile(const QUrl &url);

    void openFile(const QString &filePath);

private Q_SLOTS:
    void slotOnShutdown();
    void slotSettingsChanged();
    void slotSetStatusText(const QString &statusText);

    void feedListLoaded(const QSharedPointer<Akregator::FeedList> &list);

    void flushAddFeedRequests();

    void slotRestoreSession(Akregator::CrashWidget::CrashAction type);

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
     * Clears the crash data.
     */
    void clearCrashProperties();

private: // attributes
    void initializeTrayIcon();

    class ApplyFiltersInterceptor;
    ApplyFiltersInterceptor *m_applyFiltersInterceptor = nullptr;
    QString m_standardFeedList;
    bool m_standardListLoaded = false;
    bool m_shuttingDown = false;
    bool m_doCrashSave = false;

    QTimer *m_autosaveTimer = nullptr;
    /** did we backup the feed list already? */
    bool m_backedUpList = false;
    Akregator::AkregatorCentralWidget *mCentralWidget = nullptr;
    QPointer<MainWidget> m_mainWidget;
    Backend::Storage *m_storage = nullptr;
    ActionManagerImpl *m_actionManager = nullptr;
    KCMultiDialog *m_dialog = nullptr;
    struct AddFeedRequest {
        QStringList urls;
        QString group;
    };
    QPointer<LoadFeedListCommand> m_loadFeedListCommand;
    QVector<AddFeedRequest> m_requests;
    KSharedConfig::Ptr mConfig;
};
} // namespace Akregator

