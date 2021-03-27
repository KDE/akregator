/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "feedlist.h"
#include "article.h"
#include "feed.h"
#include "folder.h"
#include "storage.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include "akregator_debug.h"
#include "kernel.h"
#include "subscriptionlistjobs.h"
#include <KLocalizedString>
#include <limits>
#include <memory>

#include <QElapsedTimer>
#include <QHash>
#include <QRandomGenerator>
#include <QSet>
#include <qdom.h>

using namespace Akregator;
class Q_DECL_HIDDEN FeedList::Private
{
    FeedList *const q;

public:
    Private(Backend::Storage *st, FeedList *qq);

    Akregator::Backend::Storage *storage;
    QList<TreeNode *> flatList;
    Folder *rootNode;
    QHash<uint, TreeNode *> idMap;
    AddNodeVisitor *addNodeVisitor;
    RemoveNodeVisitor *removeNodeVisitor;
    QHash<QString, QList<Feed *>> urlMap;
    mutable int unreadCache;
};

class FeedList::AddNodeVisitor : public TreeNodeVisitor
{
public:
    AddNodeVisitor(FeedList *list)
        : m_list(list)
    {
    }

    bool visitFeed(Feed *node) override
    {
        m_list->d->idMap.insert(node->id(), node);
        m_list->d->flatList.append(node);
        m_list->d->urlMap[node->xmlUrl()].append(node);
        connect(node, &Feed::fetchStarted, m_list, &FeedList::fetchStarted);
        connect(node, &Feed::fetched, m_list, &FeedList::fetched);
        connect(node, &Feed::fetchAborted, m_list, &FeedList::fetchAborted);
        connect(node, &Feed::fetchError, m_list, &FeedList::fetchError);
        connect(node, &Feed::fetchDiscovery, m_list, &FeedList::fetchDiscovery);

        visitTreeNode(node);
        return true;
    }

    void visit2(TreeNode *node, bool preserveID)
    {
        m_preserveID = preserveID;
        TreeNodeVisitor::visit(node);
    }

    bool visitTreeNode(TreeNode *node) override
    {
        if (!m_preserveID) {
            node->setId(m_list->generateID());
        }
        m_list->d->idMap[node->id()] = node;
        m_list->d->flatList.append(node);

        connect(node, &TreeNode::signalDestroyed, m_list, &FeedList::slotNodeDestroyed);
        connect(node, &TreeNode::signalChanged, m_list, &FeedList::signalNodeChanged);
        Q_EMIT m_list->signalNodeAdded(node);

        return true;
    }

    bool visitFolder(Folder *node) override
    {
        connect(node, &Folder::signalChildAdded, m_list, &FeedList::slotNodeAdded);
        connect(node, &Folder::signalAboutToRemoveChild, m_list, &FeedList::signalAboutToRemoveNode);
        connect(node, &Folder::signalChildRemoved, m_list, &FeedList::slotNodeRemoved);

        visitTreeNode(node);

        for (TreeNode *i = node->firstChild(); i && i != node; i = i->next()) {
            m_list->slotNodeAdded(i);
        }

        return true;
    }

private:
    FeedList *m_list = nullptr;
    bool m_preserveID = false;
};

class FeedList::RemoveNodeVisitor : public TreeNodeVisitor
{
public:
    RemoveNodeVisitor(FeedList *list)
        : m_list(list)
    {
    }

    bool visitFeed(Feed *node) override
    {
        visitTreeNode(node);
        m_list->d->urlMap[node->xmlUrl()].removeAll(node);
        return true;
    }

    bool visitTreeNode(TreeNode *node) override
    {
        m_list->d->idMap.remove(node->id());
        m_list->d->flatList.removeAll(node);
        m_list->disconnect(node);
        return true;
    }

    bool visitFolder(Folder *node) override
    {
        visitTreeNode(node);

        return true;
    }

private:
    FeedList *m_list;
};

FeedList::Private::Private(Backend::Storage *st, FeedList *qq)
    : q(qq)
    , storage(st)
    , rootNode(nullptr)
    , addNodeVisitor(new AddNodeVisitor(q))
    , removeNodeVisitor(new RemoveNodeVisitor(q))
    , unreadCache(-1)
{
    Q_ASSERT(storage);
}

FeedList::FeedList(Backend::Storage *storage)
    : QObject(nullptr)
    , d(new Private(storage, this))
{
    Folder *rootNode = new Folder(i18n("All Feeds"));
    rootNode->setId(1);
    setRootNode(rootNode);
    addNode(rootNode, true);
}

QVector<uint> FeedList::feedIds() const
{
    QVector<uint> ids;
    const auto f = feeds();
    for (const Feed *const i : f) {
        ids += i->id();
    }
    return ids;
}

QVector<const Akregator::Feed *> FeedList::feeds() const
{
    QVector<const Akregator::Feed *> constList;
    const auto rootNodeFeeds = d->rootNode->feeds();
    for (const Akregator::Feed *const i : rootNodeFeeds) {
        constList.append(i);
    }
    return constList;
}

QVector<Akregator::Feed *> FeedList::feeds()
{
    return d->rootNode->feeds();
}

QVector<const Folder *> FeedList::folders() const
{
    QVector<const Folder *> constList;
    const auto nodeFolders = d->rootNode->folders();
    for (const Folder *const i : nodeFolders) {
        constList.append(i);
    }
    return constList;
}

QVector<Folder *> FeedList::folders()
{
    return d->rootNode->folders();
}

void FeedList::addNode(TreeNode *node, bool preserveID)
{
    d->addNodeVisitor->visit2(node, preserveID);
}

void FeedList::removeNode(TreeNode *node)
{
    d->removeNodeVisitor->visit(node);
}

void FeedList::parseChildNodes(QDomNode &node, Folder *parent)
{
    QDomElement e = node.toElement(); // try to convert the node to an element.

    if (!e.isNull()) {
        // QString title = e.hasAttribute("text") ? e.attribute("text") : e.attribute("title");

        if (e.hasAttribute(QStringLiteral("xmlUrl")) || e.hasAttribute(QStringLiteral("xmlurl")) || e.hasAttribute(QStringLiteral("xmlURL"))) {
            Feed *feed = Feed::fromOPML(e, d->storage);
            if (feed) {
                if (!d->urlMap[feed->xmlUrl()].contains(feed)) {
                    d->urlMap[feed->xmlUrl()].append(feed);
                }
                parent->appendChild(feed);
            }
        } else {
            Folder *fg = Folder::fromOPML(e);
            parent->appendChild(fg);

            if (e.hasChildNodes()) {
                QDomNode child = e.firstChild();
                while (!child.isNull()) {
                    parseChildNodes(child, fg);
                    child = child.nextSibling();
                }
            }
        }
    }
}

bool FeedList::readFromOpml(const QDomDocument &doc)
{
    QDomElement root = doc.documentElement();

    qCDebug(AKREGATOR_LOG) << "loading OPML feed" << root.tagName().toLower();

    qCDebug(AKREGATOR_LOG) << "measuring startup time: START";
    QElapsedTimer spent;
    spent.start();

    if (root.tagName().toLower() != QLatin1String("opml")) {
        return false;
    }
    QDomNode bodyNode = root.firstChild();

    while (!bodyNode.isNull() && bodyNode.toElement().tagName().toLower() != QLatin1String("body")) {
        bodyNode = bodyNode.nextSibling();
    }

    if (bodyNode.isNull()) {
        qCDebug(AKREGATOR_LOG) << "Failed to acquire body node, markup broken?";
        return false;
    }

    QDomElement body = bodyNode.toElement();

    QDomNode i = body.firstChild();

    while (!i.isNull()) {
        parseChildNodes(i, allFeedsFolder());
        i = i.nextSibling();
    }

    for (TreeNode *i = allFeedsFolder()->firstChild(); i && i != allFeedsFolder(); i = i->next()) {
        if (i->id() == 0) {
            uint id = generateID();
            i->setId(id);
            d->idMap.insert(id, i);
        }
    }

    qCDebug(AKREGATOR_LOG) << "measuring startup time: STOP," << spent.elapsed() << "ms";
    qCDebug(AKREGATOR_LOG) << "Number of articles loaded:" << allFeedsFolder()->totalCount();
    return true;
}

FeedList::~FeedList()
{
    Q_EMIT signalDestroyed(this);
    setRootNode(nullptr);
    delete d->addNodeVisitor;
    delete d->removeNodeVisitor;
    delete d;
}

const Akregator::Feed *FeedList::findByURL(const QString &feedURL) const
{
    if (!d->urlMap.contains(feedURL)) {
        return nullptr;
    }
    const QList<Feed *> &v = d->urlMap[feedURL];
    return !v.isEmpty() ? v.front() : nullptr;
}

Akregator::Feed *FeedList::findByURL(const QString &feedURL)
{
    if (!d->urlMap.contains(feedURL)) {
        return nullptr;
    }
    const QList<Akregator::Feed *> &v = d->urlMap[feedURL];
    return !v.isEmpty() ? v.front() : nullptr;
}

const Article FeedList::findArticle(const QString &feedURL, const QString &guid) const
{
    const Akregator::Feed *feed = findByURL(feedURL);
    return feed ? feed->findArticle(guid) : Article();
}

void FeedList::append(FeedList *list, Folder *parent, TreeNode *after)
{
    if (list == this) {
        return;
    }

    if (!d->flatList.contains(parent)) {
        parent = allFeedsFolder();
    }

    QList<TreeNode *> children = list->allFeedsFolder()->children();

    QList<TreeNode *>::ConstIterator end(children.constEnd());
    for (QList<TreeNode *>::ConstIterator it = children.constBegin(); it != end; ++it) {
        list->allFeedsFolder()->removeChild(*it);
        parent->insertChild(*it, after);
        after = *it;
    }
}

QDomDocument FeedList::toOpml() const
{
    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")));

    QDomElement root = doc.createElement(QStringLiteral("opml"));
    root.setAttribute(QStringLiteral("version"), QStringLiteral("1.0"));
    doc.appendChild(root);

    QDomElement head = doc.createElement(QStringLiteral("head"));
    root.appendChild(head);

    QDomElement ti = doc.createElement(QStringLiteral("text"));
    head.appendChild(ti);

    QDomElement body = doc.createElement(QStringLiteral("body"));
    root.appendChild(body);

    const auto children = allFeedsFolder()->children();
    for (const TreeNode *const i : children) {
        body.appendChild(i->toOPML(body, doc));
    }

    return doc;
}

const TreeNode *FeedList::findByID(uint id) const
{
    return d->idMap[id];
}

TreeNode *FeedList::findByID(uint id)
{
    return d->idMap[id];
}

QList<const TreeNode *> FeedList::findByTitle(const QString &title) const
{
    return allFeedsFolder()->namedChildren(title);
}

QList<TreeNode *> FeedList::findByTitle(const QString &title)
{
    return allFeedsFolder()->namedChildren(title);
}

const Folder *FeedList::allFeedsFolder() const
{
    return d->rootNode;
}

Folder *FeedList::allFeedsFolder()
{
    return d->rootNode;
}

bool FeedList::isEmpty() const
{
    return d->rootNode->firstChild() == nullptr;
}

void FeedList::rootNodeChanged()
{
    Q_ASSERT(d->rootNode);
    const int newUnread = d->rootNode->unread();
    if (newUnread == d->unreadCache) {
        return;
    }
    d->unreadCache = newUnread;
    Q_EMIT unreadCountChanged(newUnread);
}

void FeedList::setRootNode(Folder *folder)
{
    if (folder == d->rootNode) {
        return;
    }

    delete d->rootNode;
    d->rootNode = folder;
    d->unreadCache = -1;

    if (d->rootNode) {
        d->rootNode->setOpen(true);
        connect(d->rootNode, &Folder::signalChildAdded, this, &FeedList::slotNodeAdded);
        connect(d->rootNode, &Folder::signalAboutToRemoveChild, this, &FeedList::signalAboutToRemoveNode);
        connect(d->rootNode, &Folder::signalChildRemoved, this, &FeedList::slotNodeRemoved);
        connect(d->rootNode, &Folder::signalChanged, this, &FeedList::signalNodeChanged);
        connect(d->rootNode, &Folder::signalChanged, this, &FeedList::rootNodeChanged);
    }
}

uint FeedList::generateID() const
{
    // The values 0 and 1 are reserved, see TreeNode::id()
    return QRandomGenerator::global()->bounded(2u, std::numeric_limits<quint32>::max());
}

void FeedList::slotNodeAdded(TreeNode *node)
{
    if (!node) {
        return;
    }

    Folder *parent = node->parent();
    if (!parent || !d->flatList.contains(parent) || d->flatList.contains(node)) {
        return;
    }

    addNode(node, false);
}

void FeedList::slotNodeDestroyed(TreeNode *node)
{
    if (!node || !d->flatList.contains(node)) {
        return;
    }
    removeNode(node);
}

void FeedList::slotNodeRemoved(Folder * /*parent*/, TreeNode *node)
{
    if (!node || !d->flatList.contains(node)) {
        return;
    }
    removeNode(node);
    Q_EMIT signalNodeRemoved(node);
}

int FeedList::unread() const
{
    if (d->unreadCache == -1) {
        d->unreadCache = d->rootNode ? d->rootNode->unread() : 0;
    }
    return d->unreadCache;
}

void FeedList::addToFetchQueue(FetchQueue *qu, bool intervalOnly)
{
    if (d->rootNode) {
        d->rootNode->slotAddToFetchQueue(qu, intervalOnly);
    }
}

KJob *FeedList::createMarkAsReadJob()
{
    return d->rootNode ? d->rootNode->createMarkAsReadJob() : nullptr;
}

FeedListManagementImpl::FeedListManagementImpl(const QSharedPointer<FeedList> &list)
    : m_feedList(list)
{
}

void FeedListManagementImpl::setFeedList(const QSharedPointer<FeedList> &list)
{
    m_feedList = list;
}

static QString path_of_folder(const Folder *fol)
{
    Q_ASSERT(fol);
    QString path;
    const Folder *i = fol;
    while (i) {
        path = QString::number(i->id()) + QLatin1Char('/') + path;
        i = i->parent();
    }
    return path;
}

QStringList FeedListManagementImpl::categories() const
{
    if (!m_feedList) {
        return QStringList();
    }
    QStringList cats;
    const auto folders = m_feedList->folders();
    for (const Folder *const i : folders) {
        cats.append(path_of_folder(i));
    }
    return cats;
}

QStringList FeedListManagementImpl::feeds(const QString &catId) const
{
    if (!m_feedList) {
        return QStringList();
    }

    const uint lastcatid = catId.split(QLatin1Char('/'), Qt::SkipEmptyParts).last().toUInt();

    QSet<QString> urls;
    const auto feeds = m_feedList->feeds();
    for (const Feed *const i : feeds) {
        if (lastcatid == i->parent()->id()) {
            urls.insert(i->xmlUrl());
        }
    }
    return urls.values();
}

void FeedListManagementImpl::addFeed(const QString &url, const QString &catId)
{
    if (!m_feedList) {
        return;
    }

    qCDebug(AKREGATOR_LOG) << "Name:" << url.left(20) << "Cat:" << catId;
    const uint folder_id = catId.split(QLatin1Char('/'), Qt::SkipEmptyParts).last().toUInt();

    // Get the folder
    Folder *m_folder = nullptr;
    const QVector<Folder *> vector = m_feedList->folders();
    for (int i = 0; i < vector.size(); ++i) {
        if (vector.at(i)->id() == folder_id) {
            m_folder = vector.at(i);
            break;
        }
    }

    // Create new feed
    QScopedPointer<FeedList> new_feedlist(new FeedList(Kernel::self()->storage()));
    Feed *new_feed = new Feed(Kernel::self()->storage());
    new_feed->setXmlUrl(url);
    // new_feed->setTitle(url);
    new_feedlist->allFeedsFolder()->appendChild(new_feed);

    // Get last in the folder
    TreeNode *m_last = m_folder->childAt(m_folder->totalCount());

    // Add the feed
    m_feedList->append(new_feedlist.data(), m_folder, m_last);
}

void FeedListManagementImpl::removeFeed(const QString &url, const QString &catId)
{
    qCDebug(AKREGATOR_LOG) << "Name:" << url.left(20) << "Cat:" << catId;

    uint lastcatid = catId.split(QLatin1Char('/'), Qt::SkipEmptyParts).last().toUInt();

    const auto feeds = m_feedList->feeds();
    for (const Feed *const i : feeds) {
        if (lastcatid == i->parent()->id()) {
            if (i->xmlUrl().compare(url) == 0) {
                qCDebug(AKREGATOR_LOG) << "id:" << i->id();
                auto job = new DeleteSubscriptionJob;
                job->setSubscriptionId(i->id());
                job->start();
            }
        }
    }
}

QString FeedListManagementImpl::getCategoryName(const QString &catId) const
{
    QString catname;

    if (!m_feedList) {
        return catname;
    }

    const QStringList list = catId.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (int i = 0; i < list.size(); ++i) {
        int index = list.at(i).toInt();
        catname += m_feedList->findByID(index)->title() + QLatin1Char('/');
    }

    return catname;
}
