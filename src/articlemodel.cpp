/*
    This file is part of Akregator.

    Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>

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
#include "articlemodel.h"

#include "article.h"
#include "articlematcher.h"
#include "akregatorconfig.h"
#include "feed.h"
#include "utils.h"

#include <Syndication/Tools>

#include <QMimeData>
#include <QString>
#include <QVector>

#include <KLocalizedString>
#include <QUrl>

#include <memory>

#include <QLocale>
#include <cassert>
#include <cmath>

using namespace Akregator;

//like Syndication::htmlToPlainText, but without linebreaks

static QString stripHtml(const QString &html)
{
    QString str(html);
    //TODO: preserve some formatting, such as line breaks
    str = Akregator::Utils::stripTags(str); // remove tags
    str = Syndication::resolveEntities(str);
    return str.simplified();
}

ArticleModel::ArticleModel(const QVector<Article> &articles, QObject *parent) : QAbstractTableModel(parent)
{
    m_articles = articles;
    const int articlesCount(articles.count());
    m_titleCache.resize(articlesCount);
    for (int i = 0; i < articlesCount; ++i) {
        m_titleCache[i] = stripHtml(articles[i].title());
    }
}

ArticleModel::~ArticleModel()
{
}

int ArticleModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColumnCount;
}

int ArticleModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_articles.count();
}

QVariant ArticleModel::headerData(int section, Qt::Orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    case ItemTitleColumn:
        return i18nc("Articlelist's column header", "Title");
    case FeedTitleColumn:
        return i18nc("Articlelist's column header", "Feed");
    case DateColumn:
        return i18nc("Articlelist's column header", "Date");
    case AuthorColumn:
        return i18nc("Articlelist's column header", "Author");
    case DescriptionColumn:
        return i18nc("Articlelist's column header", "Description");
    case ContentColumn:
        return i18nc("Articlelist's column header", "Content");
    }

    return QVariant();
}

QVariant ArticleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_articles.count()) {
        return QVariant();
    }
    const int row = index.row();
    const Article &article(m_articles[row]);

    switch (role) {
    case SortRole:
        if (index.column() == DateColumn) {
            return article.pubDate();
        }
        Q_FALLTHROUGH();
    // no break
    case Qt::DisplayRole:
        switch (index.column()) {
        case FeedTitleColumn:
            return article.feed() ? article.feed()->title() : QVariant();
        case DateColumn:
            return QLocale().toString(article.pubDate(), QLocale::ShortFormat);
        case ItemTitleColumn:
            return m_titleCache[row];
        case AuthorColumn:
            return article.authorShort();
        case DescriptionColumn:
        case ContentColumn:
            return article.description();
        }
    case LinkRole:
        return article.link();
    case ItemIdRole:
    case GuidRole:
        return article.guid();
    case FeedIdRole:
        return article.feed() ? article.feed()->xmlUrl() : QVariant();
    case StatusRole:
        return article.status();
    case IsImportantRole:
        return article.keep();
    case IsDeletedRole:
        return article.isDeleted();
    }

    return QVariant();
}

void ArticleModel::clear()
{
    beginResetModel();
    m_articles.clear();
    m_titleCache.clear();
    endResetModel();
}

void ArticleModel::articlesAdded(Akregator::TreeNode *, const QVector<Article> &l)
{
    if (l.isEmpty()) { //assert?
        return;
    }
    const int first = m_articles.count();
    beginInsertRows(QModelIndex(), first, first + l.size() - 1);

    const int oldSize = m_articles.size();
    m_articles << l;

    const int newArticlesCount(m_articles.count());
    m_titleCache.resize(newArticlesCount);
    for (int i = oldSize; i < newArticlesCount; ++i) {
        m_titleCache[i] = stripHtml(m_articles[i].title());
    }
    endInsertRows();
}

void ArticleModel::articlesRemoved(Akregator::TreeNode *, const QVector<Article> &l)
{
    //might want to avoid indexOf() in case of performance problems
    for (const Article &i : l) {
        const int row = m_articles.indexOf(i);
        Q_ASSERT(row != -1);
        removeRow(row, QModelIndex());
    }
}

void ArticleModel::articlesUpdated(Akregator::TreeNode *, const QVector<Article> &l)
{
    int rmin = 0;
    int rmax = 0;

    const int numberOfArticles(m_articles.count());
    if (numberOfArticles > 0) {
        rmin = numberOfArticles - 1;
        //might want to avoid indexOf() in case of performance problems
        for (const Article &i : l) {
            const int row = m_articles.indexOf(i);
            //TODO: figure out how why the Article might not be found in
            //TODO: the articles list because we should need this conditional.
            if (row >= 0) {
                m_titleCache[row] = stripHtml(m_articles[row].title());
                rmin = std::min(row, rmin);
                rmax = std::max(row, rmax);
            }
        }
    }
    Q_EMIT dataChanged(index(rmin, 0), index(rmax, ColumnCount - 1));
}

bool ArticleModel::rowMatches(int row, const QSharedPointer<const Filters::AbstractMatcher> &matcher) const
{
    Q_ASSERT(matcher);
    return matcher->matches(article(row));
}

Article ArticleModel::article(int row) const
{
    if (row < 0 || row >= m_articles.count()) {
        return Article();
    }
    return m_articles[row];
}

QStringList ArticleModel::mimeTypes() const
{
    return QStringList() << QStringLiteral("text/uri-list");
}

QMimeData *ArticleModel::mimeData(const QModelIndexList &indexes) const
{
    QScopedPointer<QMimeData> md(new QMimeData);
    QList<QUrl> urls;
    QList<int> seenArticles;
    for (const QModelIndex &i : indexes) {
        const int rowIndex = i.row();
        if (seenArticles.contains(rowIndex)) {
            continue;
        }
        seenArticles.append(rowIndex);
        const QUrl url = i.data(ArticleModel::LinkRole).toUrl();
        if (url.isValid()) {
            urls.push_back(url);
        } else {
            const QUrl guid(i.data(ArticleModel::GuidRole).toString());
            if (guid.isValid()) {
                urls.push_back(guid);
            }
        }
    }
    md->setUrls(urls);
    return md.take();
}

Qt::ItemFlags ArticleModel::flags(const QModelIndex &idx) const
{
    const Qt::ItemFlags f = QAbstractTableModel::flags(idx);
    if (!idx.isValid()) {
        return f;
    }
    return f | Qt::ItemIsDragEnabled;
}
