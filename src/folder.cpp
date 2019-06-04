/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2004-2005 Frank Osterfeld <osterfeld@kde.org>

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
#include "folder.h"
#include "article.h"
#include "articlejobs.h"

#include "feed.h"
#include "fetchqueue.h"
#include "treenodevisitor.h"

#include <qdom.h>
#include <QList>

#include <QIcon>
#include "akregator_debug.h"

#include <cassert>

using namespace Akregator;

// efficient alternative so we don't convert first to a temporary QList then to QVector
template<typename T>
static QVector<T> hashValuesToVector(const QHash<int, T> &hash)
{
    QVector<T> result;
    result.reserve(hash.count());
    for (auto it = hash.cbegin(), end = hash.cend(); it != end; ++it) {
        result.append(it.value());
    }

    return result;
}

class Folder::FolderPrivate
{
    Folder *const q;
public:
    explicit FolderPrivate(Folder *qq);
    ~FolderPrivate();

    /** List of children */
    QList<TreeNode *> children;
    /** caching unread count of children */
    mutable int unread;
    /** whether or not the folder is expanded */
    bool open;
};

Folder::FolderPrivate::FolderPrivate(Folder *qq) : q(qq)
    , unread(0)
    , open(false)
{
}

Folder::FolderPrivate::~FolderPrivate()
{
    while (!children.isEmpty()) {
        // child removes itself from list in its destructor
        delete children.first();
    }
    Q_EMIT q->emitSignalDestroyed();
}

bool Folder::accept(TreeNodeVisitor *visitor)
{
    if (visitor->visitFolder(this)) {
        return true;
    } else {
        return visitor->visitTreeNode(this);
    }
}

Folder *Folder::fromOPML(const QDomElement &e)
{
    Folder *fg = new Folder(e.hasAttribute(QStringLiteral("text")) ? e.attribute(QStringLiteral("text")) : e.attribute(QStringLiteral("title")));
    fg->setOpen(e.attribute(QStringLiteral("isOpen")) == QLatin1String("true"));
    fg->setId(e.attribute(QStringLiteral("id")).toUInt());
    return fg;
}

Folder::Folder(const QString &title) : TreeNode()
    , d(new FolderPrivate(this))
{
    setTitle(title);
}

Folder::~Folder()
{
    delete d;
    d = nullptr;
}

QVector<Article> Folder::articles()
{
    QVector<Article> seq;
    const auto f = feeds();
    for (Feed *const i : f) {
        seq += i->articles();
    }
    return seq;
}

QDomElement Folder::toOPML(QDomElement parent, QDomDocument document) const
{
    QDomElement el = document.createElement(QStringLiteral("outline"));
    el.setAttribute(QStringLiteral("text"), title());
    parent.appendChild(el);
    el.setAttribute(QStringLiteral("isOpen"), d->open ? QStringLiteral("true") : QStringLiteral("false"));
    el.setAttribute(QStringLiteral("id"), QString::number(id()));

    const auto children = d->children;
    for (const Akregator::TreeNode *i : children) {
        el.appendChild(i->toOPML(el, document));
    }
    return el;
}

QList<const TreeNode *> Folder::children() const
{
    QList<const TreeNode *> children;
    children.reserve(d->children.size());
    for (const TreeNode *i : qAsConst(d->children)) {
        children.append(i);
    }
    return children;
}

QList<TreeNode *> Folder::children()
{
    return d->children;
}

QVector<const Akregator::Feed *> Folder::feeds() const
{
    QHash<int, const Akregator::Feed *> feedsById;
    for (const TreeNode *i : qAsConst(d->children)) {
        const auto f = i->feeds();
        for (const Akregator::Feed *j : f) {
            feedsById.insert(j->id(), j);
        }
    }

    return hashValuesToVector<const Akregator::Feed *>(feedsById);
}

QVector<Akregator::Feed *> Folder::feeds()
{
    QHash<int, Akregator::Feed *> feedsById;
    for (TreeNode *i : qAsConst(d->children)) {
        const auto f = i->feeds();
        for (Akregator::Feed *j : f) {
            feedsById.insert(j->id(), j);
        }
    }

    return hashValuesToVector<Akregator::Feed *>(feedsById);
}

QVector<const Folder *> Folder::folders() const
{
    QHash<int, const Folder *> foldersById;
    foldersById.insert(id(), this);
    for (const TreeNode *i : qAsConst(d->children)) {
        const auto f = i->folders();
        for (const Folder *j : f) {
            foldersById.insert(j->id(), j);
        }
    }

    return hashValuesToVector<const Folder *>(foldersById);
}

QVector<Folder *> Folder::folders()
{
    QHash<int, Folder *> foldersById;
    foldersById.insert(id(), this);
    for (TreeNode *i : qAsConst(d->children)) {
        const auto f = i->folders();
        for (Folder *j : f) {
            foldersById.insert(j->id(), j);
        }
    }
    return hashValuesToVector<Folder *>(foldersById);
}

int Folder::indexOf(const TreeNode *node) const
{
    return children().indexOf(node);
}

void Folder::insertChild(TreeNode *node, TreeNode *after)
{
    int pos = d->children.indexOf(after);

    if (pos < 0) {
        prependChild(node);
    } else {
        insertChild(pos, node);
    }
}

QIcon Folder::icon() const
{
    return QIcon::fromTheme(QStringLiteral("folder"));
}

void Folder::insertChild(int index, TreeNode *node)
{
//    qCDebug(AKREGATOR_LOG) <<"enter Folder::insertChild(int, node)" << node->title();
    if (node) {
        if (index >= d->children.size()) {
            d->children.append(node);
        } else {
            d->children.insert(index, node);
        }
        node->setParent(this);
        connectToNode(node);
        updateUnreadCount();
        Q_EMIT signalChildAdded(node);
        articlesModified();
        nodeModified();
    }
//    qCDebug(AKREGATOR_LOG) <<"leave Folder::insertChild(int, node)" << node->title();
}

void Folder::appendChild(TreeNode *node)
{
//    qCDebug(AKREGATOR_LOG) <<"enter Folder::appendChild()" << node->title();
    if (node) {
        d->children.append(node);
        node->setParent(this);
        connectToNode(node);
        updateUnreadCount();
        Q_EMIT signalChildAdded(node);
        articlesModified();
        nodeModified();
    }
//    qCDebug(AKREGATOR_LOG) <<"leave Folder::appendChild()" << node->title();
}

void Folder::prependChild(TreeNode *node)
{
//    qCDebug(AKREGATOR_LOG) <<"enter Folder::prependChild()" << node->title();
    if (node) {
        d->children.prepend(node);
        node->setParent(this);
        connectToNode(node);
        updateUnreadCount();
        Q_EMIT signalChildAdded(node);
        articlesModified();
        nodeModified();
    }
//    qCDebug(AKREGATOR_LOG) <<"leave Folder::prependChild()" << node->title();
}

void Folder::removeChild(TreeNode *node)
{
    if (!node || !d->children.contains(node)) {
        return;
    }

    Q_EMIT signalAboutToRemoveChild(node);
    node->setParent(nullptr);
    d->children.removeOne(node);
    disconnectFromNode(node);
    updateUnreadCount();
    Q_EMIT signalChildRemoved(this, node);
    articlesModified(); // articles were removed, TODO: add guids to a list
    nodeModified();
}

TreeNode *Folder::firstChild()
{
    return d->children.isEmpty() ? nullptr : children().first();
}

const TreeNode *Folder::firstChild() const
{
    return d->children.isEmpty() ? nullptr : children().first();
}

TreeNode *Folder::lastChild()
{
    return d->children.isEmpty() ? nullptr : children().last();
}

const TreeNode *Folder::lastChild() const
{
    return d->children.isEmpty() ? nullptr : children().last();
}

bool Folder::isOpen() const
{
    return d->open;
}

void Folder::setOpen(bool open)
{
    d->open = open;
}

int Folder::unread() const
{
    return d->unread;
}

int Folder::totalCount() const
{
    int total = 0;
    const auto f = feeds();
    for (const Feed *const i : f) {
        total += i->totalCount();
    }
    return total;
}

void Folder::updateUnreadCount() const
{
    int unread = 0;
    const auto f = feeds();
    for (const Feed *const i : f) {
        unread += i->unread();
    }
    d->unread = unread;
}

KJob *Folder::createMarkAsReadJob()
{
    CompositeJob *job = new CompositeJob;
    const auto f = feeds();
    for (Feed *const i : f) {
        job->addSubjob(i->createMarkAsReadJob());
    }
    return job;
}

void Folder::slotChildChanged(TreeNode * /*node*/)
{
    updateUnreadCount();
    nodeModified();
}

void Folder::slotChildDestroyed(TreeNode *node)
{
    d->children.removeAll(node);
    updateUnreadCount();
    nodeModified();
}

bool Folder::subtreeContains(const TreeNode *node) const
{
    if (node == this) {
        return false;
    }
    const Folder *parent = node ? node->parent() : nullptr;
    while (parent) {
        if (parent == this) {
            return true;
        }
        parent = parent->parent();
    }

    return false;
}

void Folder::slotAddToFetchQueue(FetchQueue *queue, bool intervalFetchOnly)
{
    const auto f = feeds();
    for (Feed *const i : f) {
        if (i->useCustomFetchInterval()) {
            if (i->fetchInterval() != -1) {
                i->slotAddToFetchQueue(queue, intervalFetchOnly);
            } else {
                qCDebug(AKREGATOR_LOG) << " excluded feeds: " << i->description();
            }
        } else {
            i->slotAddToFetchQueue(queue, intervalFetchOnly);
        }
    }
}

void Folder::doArticleNotification()
{
}

void Folder::connectToNode(TreeNode *child)
{
    connect(child, &TreeNode::signalChanged, this, &Folder::slotChildChanged);
    connect(child, &TreeNode::signalDestroyed, this, &Folder::slotChildDestroyed);
    connect(child, &TreeNode::signalArticlesAdded, this, &TreeNode::signalArticlesAdded);
    connect(child, &TreeNode::signalArticlesRemoved, this, &TreeNode::signalArticlesRemoved);
    connect(child, &TreeNode::signalArticlesUpdated, this, &TreeNode::signalArticlesUpdated);
}

void Folder::disconnectFromNode(TreeNode *child)
{
    Q_ASSERT(child);
    child->disconnect(this);
}

TreeNode *Folder::childAt(int pos)
{
    if (pos < 0 || pos >= d->children.count()) {
        return nullptr;
    }
    return d->children.at(pos);
}

const TreeNode *Folder::childAt(int pos) const
{
    if (pos < 0 || pos >= d->children.count()) {
        return nullptr;
    }
    return d->children.at(pos);
}

TreeNode *Folder::next()
{
    if (firstChild()) {
        return firstChild();
    }

    if (nextSibling()) {
        return nextSibling();
    }

    Folder *p = parent();
    while (p) {
        if (p->nextSibling()) {
            return p->nextSibling();
        } else {
            p = p->parent();
        }
    }
    return nullptr;
}

const TreeNode *Folder::next() const
{
    if (firstChild()) {
        return firstChild();
    }

    if (nextSibling()) {
        return nextSibling();
    }

    const Folder *p = parent();
    while (p) {
        if (p->nextSibling()) {
            return p->nextSibling();
        } else {
            p = p->parent();
        }
    }
    return nullptr;
}

QList<const TreeNode *> Folder::namedChildren(const QString &title) const
{
    QList<const TreeNode *> nodeList;
    const auto childs = children();
    for (const TreeNode *child : childs) {
        if (child->title() == title) {
            nodeList.append(child);
        }
        const Folder *fld = dynamic_cast<const Folder *>(child);
        if (fld) {
            nodeList += fld->namedChildren(title);
        }
    }
    return nodeList;
}

QList<TreeNode *> Folder::namedChildren(const QString &title)
{
    QList<TreeNode *> nodeList;
    const auto childs = children();
    for (TreeNode *const child : childs) {
        if (child->title() == title) {
            nodeList.append(child);
        }
        Folder *const fld = qobject_cast<Folder *>(child);
        if (fld) {
            nodeList += fld->namedChildren(title);
        }
    }
    return nodeList;
}
