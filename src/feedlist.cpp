/*
    This file is part of Akregator.

    Copyright (C) 2004 Frank Osterfeld <osterfeld@kde.org>

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

#include "feedlist.h"
#include "storage.h"
#include "article.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include "kernel.h"
#include "subscriptionlistjobs.h"

#include "akregator_debug.h"
#include <KLocalizedString>
#include <krandom.h>

#include <qdom.h>
#include <QHash>
#include <QSet>
#include <QTime>

#include <cassert>

namespace Akregator
{

class Q_DECL_HIDDEN FeedList::Private
{
    FeedList *const q;

public:
    Private(Backend::Storage *st, FeedList *qq);

    Akregator::Backend::Storage *storage;
    QList<TreeNode *> flatList;
    Folder *rootNode;
    QHash<int, TreeNode *> idMap;
    AddNodeVisitor *addNodeVisitor;
    RemoveNodeVisitor *removeNodeVisitor;
    QHash<QString, QList<Feed *> > urlMap;
    mutable int unreadCache;
};

class FeedList::AddNodeVisitor : public TreeNodeVisitor
{
public:
    AddNodeVisitor(FeedList *list) : m_list(list) {}

    bool visitFeed(Feed *node) Q_DECL_OVERRIDE
    {
        m_list->d->idMap.insert(node->id(), node);
        m_list->d->flatList.append(node);
        m_list->d->urlMap[node->xmlUrl()].append(node);
        connect(node, SIGNAL(fetchStarted(Akregator::Feed*)),
                m_list, SIGNAL(fetchStarted(Akregator::Feed*)));
        connect(node, SIGNAL(fetched(Akregator::Feed*)),
                m_list, SIGNAL(fetched(Akregator::Feed*)));
        connect(node, SIGNAL(fetchAborted(Akregator::Feed*)),
                m_list, SIGNAL(fetchAborted(Akregator::Feed*)));
        connect(node, SIGNAL(fetchError(Akregator::Feed*)),
                m_list, SIGNAL(fetchError(Akregator::Feed*)));
        connect(node, SIGNAL(fetchDiscovery(Akregator::Feed*)),
                m_list, SIGNAL(fetchDiscovery(Akregator::Feed*)));

        visitTreeNode(node);
        return true;
    }

    void visit2(TreeNode *node, bool preserveID)
    {
        m_preserveID = preserveID;
        TreeNodeVisitor::visit(node);
    }

    bool visitTreeNode(TreeNode *node) Q_DECL_OVERRIDE
    {
        if (!m_preserveID) {
            node->setId(m_list->generateID());
        }
        m_list->d->idMap[node->id()] = node;
        m_list->d->flatList.append(node);

        connect(node, SIGNAL(signalDestroyed(Akregator::TreeNode*)), m_list, SLOT(slotNodeDestroyed(Akregator::TreeNode*)));
        connect(node, SIGNAL(signalChanged(Akregator::TreeNode*)), m_list, SIGNAL(signalNodeChanged(Akregator::TreeNode*)));
        Q_EMIT m_list->signalNodeAdded(node);

        return true;
    }

    bool visitFolder(Folder *node) Q_DECL_OVERRIDE
    {
        connect(node, SIGNAL(signalChildAdded(Akregator::TreeNode*)), m_list, SLOT(slotNodeAdded(Akregator::TreeNode*)));
        connect(node, SIGNAL(signalAboutToRemoveChild(Akregator::TreeNode*)), m_list, SIGNAL(signalAboutToRemoveNode(Akregator::TreeNode*)));
        connect(node, SIGNAL(signalChildRemoved(Akregator::Folder*,Akregator::TreeNode*)), m_list, SLOT(slotNodeRemoved(Akregator::Folder*,Akregator::TreeNode*)));

        visitTreeNode(node);

        for (TreeNode *i = node->firstChild(); i && i != node; i = i->next()) {
            m_list->slotNodeAdded(i);
        }

        return true;
    }

private:
    FeedList *m_list;
    bool m_preserveID;
};

class FeedList::RemoveNodeVisitor : public TreeNodeVisitor
{
public:
    RemoveNodeVisitor(FeedList *list) : m_list(list) {}

    bool visitFeed(Feed *node) Q_DECL_OVERRIDE
    {
        visitTreeNode(node);
        m_list->d->urlMap[node->xmlUrl()].removeAll(node);
        return true;
    }

    bool visitTreeNode(TreeNode *node) Q_DECL_OVERRIDE
    {
        m_list->d->idMap.remove(node->id());
        m_list->d->flatList.removeAll(node);
        m_list->disconnect(node);
        return true;
    }

    bool visitFolder(Folder *node) Q_DECL_OVERRIDE
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
    , rootNode(0)
    , addNodeVisitor(new AddNodeVisitor(q))
    , removeNodeVisitor(new RemoveNodeVisitor(q))
    , unreadCache(-1)
{
    Q_ASSERT(storage);
}

FeedList::FeedList(Backend::Storage *storage)
    : QObject(0), d(new Private(storage, this))
{
    Folder *rootNode = new Folder(i18n("All Feeds"));
    rootNode->setId(1);
    setRootNode(rootNode);
    addNode(rootNode, true);
}

QVector<int> FeedList::feedIds() const
{
    QVector<int> ids;
    Q_FOREACH (const Feed *const i, feeds()) {
        ids += i->id();
    }
    return ids;
}

QVector<const Feed *> FeedList::feeds() const
{
    QVector<const Feed *> constList;
    Q_FOREACH (const Feed *const i, d->rootNode->feeds()) {
        constList.append(i);
    }
    return constList;
}

QVector<Feed *> FeedList::feeds()
{
    return d->rootNode->feeds();
}

QVector<const Folder *> FeedList::folders() const
{
    QVector<const Folder *> constList;
    Q_FOREACH (const Folder *const i, d->rootNode->folders()) {
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
        //QString title = e.hasAttribute("text") ? e.attribute("text") : e.attribute("title");

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
    QTime spent;
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

    for (TreeNode *i = allFeedsFolder()->firstChild(); i && i != allFeedsFolder(); i = i->next())
        if (i->id() == 0) {
            uint id = generateID();
            i->setId(id);
            d->idMap.insert(id, i);
        }

    qCDebug(AKREGATOR_LOG) << "measuring startup time: STOP," << spent.elapsed() << "ms";
    qCDebug(AKREGATOR_LOG) << "Number of articles loaded:" << allFeedsFolder()->totalCount();
    return true;
}

FeedList::~FeedList()
{
    Q_EMIT signalDestroyed(this);
    setRootNode(0);
    delete d->addNodeVisitor;
    delete d->removeNodeVisitor;
    delete d;
}

const Feed *FeedList::findByURL(const QString &feedURL) const
{
    if (!d->urlMap.contains(feedURL)) {
        return 0;
    }
    const QList<Feed *> &v = d->urlMap[feedURL];
    return !v.isEmpty() ? v.front() : 0;
}

Feed *FeedList::findByURL(const QString &feedURL)
{
    if (!d->urlMap.contains(feedURL)) {
        return 0;
    }
    const QList<Feed *> &v = d->urlMap[feedURL];
    return !v.isEmpty() ? v.front() : 0;
}

const Article FeedList::findArticle(const QString &feedURL, const QString &guid) const
{
    const Feed *feed = findByURL(feedURL);
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

    foreach (const TreeNode *const i, allFeedsFolder()->children()) {
        body.appendChild(i->toOPML(body, doc));
    }

    return doc;
}

const TreeNode *FeedList::findByID(int id) const
{
    return d->idMap[id];
}

TreeNode *FeedList::findByID(int id)
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
    return d->rootNode->firstChild() == 0;
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

int FeedList::generateID() const
{
    return KRandom::random();
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
    return d->rootNode ? d->rootNode->createMarkAsReadJob() : 0;
}

FeedListManagementImpl::FeedListManagementImpl(const QSharedPointer<FeedList> &list) : m_feedList(list)
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
    Q_FOREACH (const Folder *const i, m_feedList->folders()) {
        cats.append(path_of_folder(i));
    }
    return cats;
}

QStringList FeedListManagementImpl::feeds(const QString &catId) const
{
    if (!m_feedList) {
        return QStringList();
    }

    uint lastcatid = catId.split(QLatin1Char('/'), QString::SkipEmptyParts).last().toUInt();

    QSet<QString> urls;
    Q_FOREACH (const Feed *const i, m_feedList->feeds()) {
        if (lastcatid == i->parent()->id()) {
            urls.insert(i->xmlUrl());
        }
    }
    return urls.toList();
}

void FeedListManagementImpl::addFeed(const QString &url, const QString &catId)
{
    if (!m_feedList) {
        return;
    }

    qCDebug(AKREGATOR_LOG) << "Name:" << url.left(20) << "Cat:" << catId;
    uint folder_id = catId.split(QLatin1Char('/'), QString::SkipEmptyParts).last().toUInt();

    // Get the folder
    Folder *m_folder = 0;
    QVector<Folder *> vector = m_feedList->folders();
    for (int i = 0; i < vector.size(); ++i) {
        if (vector.at(i)->id() == folder_id) {
            m_folder = vector.at(i);
            i = vector.size();
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

    uint lastcatid = catId.split(QLatin1Char('/'), QString::SkipEmptyParts).last().toUInt();

    Q_FOREACH (const Feed *const i, m_feedList->feeds()) {
        if (lastcatid == i->parent()->id()) {
            if (i->xmlUrl().compare(url) == 0) {
                qCDebug(AKREGATOR_LOG) << "id:" << i->id();
                DeleteSubscriptionJob *job = new DeleteSubscriptionJob;
                job->setSubscriptionId(i->id());
                job->start();
            }
        }
    }
}

QString FeedListManagementImpl::addCategory(const QString &name, const QString &parentId) const
{
    Q_UNUSED(parentId)

    if (!m_feedList) {
        return QStringLiteral("");
    }

    Folder *m_folder = new Folder(name);
    m_feedList->allFeedsFolder()->appendChild(m_folder);

    return QString::number(m_folder->id());
}

QString FeedListManagementImpl::getCategoryName(const QString &catId) const
{
    QString catname;

    if (!m_feedList) {
        return catname;
    }

    QStringList list = catId.split(QLatin1Char('/'), QString::SkipEmptyParts);
    for (int i = 0; i < list.size(); ++i) {
        int index = list.at(i).toInt();
        catname += m_feedList->findByID(index)->title() + QLatin1Char('/');
    }

    return catname;
}

} // namespace Akregator

