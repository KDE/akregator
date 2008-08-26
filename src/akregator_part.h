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

#include <kurl.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>

class KConfigGroup;
class KUrl;
class KCMultiDialog;

class QDomDocument;
class QTimer;

namespace Akregator {

namespace Backend
{
    class Storage;
}

typedef KParts::ReadOnlyPart MyBasePart;

class ActionManagerImpl;
class Feed;
class MainWidget;
class Part;
class TrayIcon;

class BrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT

    public:
        explicit BrowserExtension(Part *p, const char *name=0);
    public slots:
        void saveSettings();
    private:
        Part *m_part;
};

/**
    This is a RSS Aggregator "Part". It does all the real work.
    It is also embeddable into other applications (e.g. for use in Kontact).
    */
class Part : public MyBasePart
{
    Q_OBJECT
    public:
        typedef MyBasePart inherited;

        /** Default constructor.*/
        Part(QWidget *parentWidget, QObject *parent, const QVariantList&);

	   /** Destructor. */
        ~Part();

        /**
            Opens feedlist
            @param url URL to feedlist
            */
        bool openUrl(const KUrl& url);

        /** Opens standard feedlist */
        void openStandardFeedList();

        void fetchFeedUrl(const QString&);

        /** Fetch all feeds in the feed tree */
        void fetchAllFeeds();

        /**
            Add a feed to a group.
            @param url The URL of the feed to add.
            @param group The name of the folder into which the feed is added.
            If the group does not exist, it is created.  The feed is added as the last member
            of the group.
            */
        void addFeedsToGroup(const QStringList& urls, const QString& group);

        void addFeed();

        /**
            This method is called when this app is restored.  The KConfig
            object points to the session management config file that was saved
            with @ref saveProperties
            Calls Akregator MainWidget's saveProperties.
            */
        virtual void readProperties(const KConfigGroup & config);

        /** This method is called when it is time for the app to save its
            properties for session management purposes.
            Calls Akregator MainWidget's readProperties. */
        virtual void saveProperties(KConfigGroup & config);

        void exportFile(const KUrl& url);

    public slots:
        /** Used to save settings after changing them from configuration dialog. Calls AkregatorPart's saveSettings. */
        void saveSettings();

        /** Saves the standard feed list to it's default location */
        void slotSaveFeedList();

        void fileImport();
        void fileExport();

        /** Shows configuration dialog */
        void showOptions();
        void showKNotifyOptions();

    signals:
        void showPart();
        void signalSettingsChanged();


    private:

    /** @return Whether the tray icon is enabled or not */
        bool isTrayIconEnabled() const;

        /** loads all Akregator storage plugins */
        void loadStoragePlugins();

        /** This must be implemented by each part */
        bool openFile();

        void importFile(const KUrl& url);

        /** FIXME: hack to get the tray icon working */
        QWidget* getMainWindow();

        KParts::Part *hitTest(QWidget *widget, const QPoint &globalPos);

    private slots:
        void slotStarted();

        void slotOnShutdown();
        void slotSettingsChanged();

    private: // methods

        bool copyFile(const QString& backup);

        /** fills the font settings with system fonts, if fonts are not set */
        void initFonts();

        /** creates an OPML file containing the initial feeds (KDE feeds) */
        static QDomDocument createDefaultFeedList();

    private: // attributes

        class ApplyFiltersInterceptor;
        ApplyFiltersInterceptor* m_applyFiltersInterceptor;
        QString m_standardFeedList;
        bool m_standardListLoaded;
        bool m_shuttingDown;

        KParts::BrowserExtension *m_extension;

        QTimer* m_autosaveTimer;
        /** did we backup the feed list already? */
        bool m_backedUpList;
        MainWidget* m_mainWidget;
        Backend::Storage* m_storage;
        ActionManagerImpl* m_actionManager;
        KCMultiDialog* m_dialog;
};

} // namespace Akregator

#endif // AKREGATOR_PART_H
