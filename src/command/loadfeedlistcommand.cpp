/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "loadfeedlistcommand.h"

#include "feedlist.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <QDateTime>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QRandomGenerator>
#include <QString>
#include <QTimer>

#include <cassert>

using namespace Akregator;
using namespace Akregator::Backend;

class Akregator::LoadFeedListCommandPrivate
{
    LoadFeedListCommand *const q;

public:
    explicit LoadFeedListCommandPrivate(LoadFeedListCommand *qq)
        : q(qq)
    {
    }

    void handleDocument(const QDomDocument &doc);
    [[nodiscard]] QString createBackup(const QString &path, bool *ok);
    void emitResult(const QSharedPointer<FeedList> &list);
    void doLoad();

    QString fileName;
    QDomDocument defaultFeedList;
    Storage *storage = nullptr;
};

void LoadFeedListCommandPrivate::emitResult(const QSharedPointer<FeedList> &list)
{
    Q_EMIT q->result(list);
    q->done();
}

void LoadFeedListCommandPrivate::handleDocument(const QDomDocument &doc)
{
    QSharedPointer<FeedList> feedList(new FeedList(storage));
    if (!feedList->readFromOpml(doc)) {
        bool backupCreated;
        const QString backupFile = createBackup(fileName, &backupCreated);
        const QString msg = backupCreated ? i18n(
                                "<qt>The standard feed list is corrupted (invalid OPML). "
                                "A backup was created:<p><b>%1</b></p></qt>",
                                backupFile)
                                          : i18n(
                                              "<qt>The standard feed list is corrupted (invalid OPML). "
                                              "Could not create a backup.</qt>");

        QPointer<QObject> that(q);
        KMessageBox::error(q->parentWidget(), msg, i18nc("@title:window", "OPML Parsing Error"));
        if (!that) {
            return;
        }
        feedList.reset();
    }
    emitResult(feedList);
}

QString LoadFeedListCommandPrivate::createBackup(const QString &path, bool *ok)
{
    const QString backup = path + QLatin1String("-backup.") + QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());

    const bool copied = QFile::copy(path, backup);
    if (ok) {
        *ok = copied;
    }
    return backup;
}

LoadFeedListCommand::LoadFeedListCommand(QObject *parent)
    : Command(parent)
    , d(new LoadFeedListCommandPrivate(this))
{
}

LoadFeedListCommand::~LoadFeedListCommand() = default;

void LoadFeedListCommand::setFileName(const QString &fileName)
{
    d->fileName = fileName;
}

void LoadFeedListCommand::setDefaultFeedList(const QDomDocument &doc)
{
    d->defaultFeedList = doc;
}

void LoadFeedListCommand::setStorage(Backend::Storage *s)
{
    d->storage = s;
}

void LoadFeedListCommand::doStart()
{
    QTimer::singleShot(QRandomGenerator::global()->bounded(400), this, [this]() {
        d->doLoad();
    });
}

void LoadFeedListCommand::doAbort()
{
}

void LoadFeedListCommandPrivate::doLoad()
{
    Q_ASSERT(storage);
    Q_ASSERT(!fileName.isNull());
    Q_EMIT q->progress(0, i18n("Opening Feed List..."));

    QDomDocument doc;

    if (!QFileInfo::exists(fileName)) {
        handleDocument(defaultFeedList);
        return;
    }

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        QPointer<QObject> that(q);
        KMessageBox::error(q->parentWidget(),
                           i18n("<qt>Could not open feed list (%1) for reading.</qt>", file.fileName()),
                           i18nc("@title:window", "Read Error"));
        if (that) {
            handleDocument(defaultFeedList);
        }
        return;
    }

    QString errMsg;
    int errLine = 0;
    int errCol = 0;
    if (!doc.setContent(&file, true, &errMsg, &errLine, &errCol)) {
        bool backupCreated = false;
        const QString backupFile = createBackup(fileName, &backupCreated);
        const QString title = i18nc("error message window caption", "XML Parsing Error");
        const QString details = xi18n(
            "<qt><p>XML parsing error in line %1, "
            "column %2 of %3:</p><p>%4</p></qt>",
            QString::number(errLine),
            QString::number(errCol),
            fileName,
            errMsg);
        const QString msg = backupCreated ? i18n(
                                "<qt>The standard feed list is corrupted (invalid XML). "
                                "A backup was created:<p><b>%1</b></p></qt>",
                                backupFile)
                                          : i18n(
                                              "<qt>The standard feed list is corrupted (invalid XML). "
                                              "Could not create a backup.</qt>");

        QPointer<QObject> that(q);

        KMessageBox::detailedError(q->parentWidget(), msg, details, title);

        if (that) {
            handleDocument(defaultFeedList);
        }
        return;
    }

    handleDocument(doc);
}

#include "moc_loadfeedlistcommand.cpp"
