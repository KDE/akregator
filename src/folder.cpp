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
#include <qdebug.h>

#include <cassert>

using namespace Akregator;

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

    /** caches guids for notifying added articles */
    QList<Article> addedArticlesNotify;
    /** caches guids for notifying removed articles */
    QList<Article> removedArticlesNotify;
};

Folder::FolderPrivate::FolderPrivate(Folder *qq) : q(qq), unread(0), open(false)
{
}

Folder::FolderPrivate::~FolderPrivate()
{
    while (!children.isEmpty()) {
        // child removes itself from list in its destructor
        delete children.first();
    }
    emit q->emitSignalDestroyed();
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
    Folder *fg = new Folder(e.hasAttribute(QString::fromLatin1("text")) ? e.attribute(QString::fromLatin1("text")) : e.attribute(QString::fromLatin1("title")));
    fg->setOpen(e.attribute(QString::fromLatin1("isOpen")) == QString::fromLatin1(("true")));
    fg->setId(e.attribute(QString::fromLatin1("id")).toUInt());
    return fg;
}

Folder::Folder(const QString &title) : TreeNode(), d(new FolderPrivate(this))
{
    setTitle(title);
}

Folder::~Folder()
{
    delete d;
    d = 0;
}

QList<Article> Folder::articles()
{
    QList<Article> seq;
    Q_FOREACH (Feed *const i, feeds()) {
        seq += i->articles();
    }
    return seq;
}

QDomElement Folder::toOPML(QDomElement parent, QDomDocument document) const
{
    QDomElement el = document.createElement(QLatin1String("outline"));
    el.setAttribute(QLatin1String("text"), title());
    parent.appendChild(el);
    el.setAttribute(QLatin1String("isOpen"), d->open ? QLatin1String("true") : QLatin1String("false"));
    el.setAttribute(QLatin1String("id"), QString::number(id()));

    Q_FOREACH (const Akregator::TreeNode *i, d->children) {
        el.appendChild(i->toOPML(el, document));
    }
    return el;
}

QList<const TreeNode *> Folder::children() const
{
    QList<const TreeNode *> children;
    Q_FOREACH (const TreeNode *i, d->children) {
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
    Q_FOREACH (const TreeNode *i, d->children)
        Q_FOREACH (const Akregator::Feed *j, i->feeds()) {
            feedsById.insert(j->id(), j);
        }
    return feedsById.values().toVector();
}

QVector<Akregator::Feed *> Folder::feeds()
{
    QHash<int, Akregator::Feed *> feedsById;
    Q_FOREACH (TreeNode *i, d->children)
        Q_FOREACH (Akregator::Feed *j, i->feeds()) {
            feedsById.insert(j->id(), j);
        }
    return feedsById.values().toVector();
}

QVector<const Folder *> Folder::folders() const
{
    QHash<int, const Folder *> foldersById;
    foldersById.insert(id(), this);
    Q_FOREACH (const TreeNode *i, d->children)
        Q_FOREACH (const Folder *j, i->folders()) {
            foldersById.insert(j->id(), j);
        }
    return foldersById.values().toVector();
}

QVector<Folder *> Folder::folders()
{
    QHash<int, Folder *> foldersById;
    foldersById.insert(id(), this);
    Q_FOREACH (TreeNode *i, d->children)
        Q_FOREACH (Folder *j, i->folders()) {
            foldersById.insert(j->id(), j);
        }
    return foldersById.values().toVector();
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
    return QIcon::fromTheme(QLatin1String("folder"));
}

void Folder::insertChild(int index, TreeNode *node)
{
//    qDebug() <<"enter Folder::insertChild(int, node)" << node->title();
    if (node) {
        if (index >= d->children.size()) {
            d->children.append(node);
        } else {
            d->children.insert(index, node);
        }
        node->setParent(this);
        connectToNode(node);
        updateUnreadCount();
        emit signalChildAdded(node);
        d->addedArticlesNotify += node->articles();
        articlesModified();
        nodeModified();
    }
//    qDebug() <<"leave Folder::insertChild(int, node)" << node->title();
}

void Folder::appendChild(TreeNode *node)
{
//    qDebug() <<"enter Folder::appendChild()" << node->title();
    if (node) {
        d->children.append(node);
        node->setParent(this);
        connectToNode(node);
        updateUnreadCount();
        emit signalChildAdded(node);
        d->addedArticlesNotify += node->articles();
        articlesModified();
        nodeModified();
    }
//    qDebug() <<"leave Folder::appendChild()" << node->title();
}

void Folder::prependChild(TreeNode *node)
{
//    qDebug() <<"enter Folder::prependChild()" << node->title();
    if (node) {
        d->children.prepend(node);
        node->setParent(this);
        connectToNode(node);
        updateUnreadCount();
        emit signalChildAdded(node);
        d->addedArticlesNotify += node->articles();
        articlesModified();
        nodeModified();
    }
//    qDebug() <<"leave Folder::prependChild()" << node->title();
}

void Folder::removeChild(TreeNode *node)
{
    if (!node || !d->children.contains(node)) {
        return;
    }

    emit signalAboutToRemoveChild(node);
    node->setParent(0);
    d->children.removeOne(node);
    disconnectFromNode(node);
    updateUnreadCount();
    emit signalChildRemoved(this, node);
    d->removedArticlesNotify += node->articles();
    articlesModified(); // articles were removed, TODO: add guids to a list
    nodeModified();
}

TreeNode *Folder::firstChild()
{
    return d->children.isEmpty() ? 0 : children().first();
}

const TreeNode *Folder::firstChild() const
{
    return d->children.isEmpty() ? 0 : children().first();
}

TreeNode *Folder::lastChild()
{
    return d->children.isEmpty() ? 0 : children().last();
}

const TreeNode *Folder::lastChild() const
{
    return d->children.isEmpty() ? 0 : children().last();
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
    Q_FOREACH (const Feed *const i, feeds()) {
        total += i->totalCount();
    }
    return total;
}

void Folder::updateUnreadCount() const
{
    int unread = 0;
    Q_FOREACH (const Feed *const i, feeds()) {
        unread += i->unread();
    }
    d->unread = unread;
}

KJob *Folder::createMarkAsReadJob()
{
    std::auto_ptr<CompositeJob> job(new CompositeJob);
    Q_FOREACH (Feed *const i, feeds()) {
        job->addSubjob(i->createMarkAsReadJob());
    }
    return job.release();
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
    const Folder *parent = node ? node->parent() : 0;
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
    Q_FOREACH (Feed *const i, feeds()) {
        i->slotAddToFetchQueue(queue, intervalFetchOnly);
    }
}

void Folder::doArticleNotification()
{
}

void Folder::connectToNode(TreeNode *child)
{
    connect(child, SIGNAL(signalChanged(Akregator::TreeNode*)), this, SLOT(slotChildChanged(Akregator::TreeNode*)));
    connect(child, SIGNAL(signalDestroyed(Akregator::TreeNode*)), this, SLOT(slotChildDestroyed(Akregator::TreeNode*)));
    connect(child, SIGNAL(signalArticlesAdded(Akregator::TreeNode*,QList<Akregator::Article>)), this, SIGNAL(signalArticlesAdded(Akregator::TreeNode*,QList<Akregator::Article>)));
    connect(child, SIGNAL(signalArticlesRemoved(Akregator::TreeNode*,QList<Akregator::Article>)), this, SIGNAL(signalArticlesRemoved(Akregator::TreeNode*,QList<Akregator::Article>)));
    connect(child, SIGNAL(signalArticlesUpdated(Akregator::TreeNode*,QList<Akregator::Article>)), this, SIGNAL(signalArticlesUpdated(Akregator::TreeNode*,QList<Akregator::Article>)));
}

void Folder::disconnectFromNode(TreeNode *child)
{
    assert(child);
    child->disconnect(this);
}

TreeNode *Folder::childAt(int pos)
{
    if (pos < 0 || pos >= d->children.count()) {
        return 0;
    }
    return d->children.at(pos);
}

const TreeNode *Folder::childAt(int pos) const
{
    if (pos < 0 || pos >= d->children.count()) {
        return 0;
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
    return 0;
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
    return 0;
}

QList<const TreeNode *> Folder::namedChildren(const QString &title) const
{
    QList<const TreeNode *> nodeList;
    foreach (const TreeNode *child, children()) {
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
    foreach (TreeNode *const child, children()) {
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

