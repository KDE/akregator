/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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

#include "searchbar.h"
#include "akregatorconfig.h"
#include "articlematcher.h"
#include "article.h"
#include "widgets/statussearchline.h"

#include <kcombobox.h>
#include <klineedit.h>
#include <KLocalizedString>

#include <QIcon>

#include <QLabel>
#include <QList>
#include <QString>
#include <QTimer>
#include <QStandardPaths>
#include <QHBoxLayout>

using namespace Akregator;
using namespace Akregator::Filters;

using namespace Akregator;
class SearchBar::SearchBarPrivate
{
public:
    QString searchText;
    QTimer timer;
    StatusSearchLine *searchLine;
    int delay;
    std::vector<QSharedPointer<const AbstractMatcher> > matchers;
    void triggerTimer()
    {
        if (timer.isActive()) {
            timer.stop();
        }

        timer.start(200);
    }
};

SearchBar::SearchBar(QWidget *parent)
    : QWidget(parent),
      d(new SearchBar::SearchBarPrivate)
{
    d->delay = 400;
    QHBoxLayout *layout = new QHBoxLayout;
    setLayout(layout);

    layout->setMargin(2);
    layout->setSpacing(5);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));

    d->searchLine = new StatusSearchLine(this);
    d->searchLine->setClearButtonShown(true);
    d->searchLine->setPlaceholderText(i18n("Search articles..."));
    layout->addWidget(d->searchLine);

    connect(d->searchLine, &KLineEdit::textChanged, this, &SearchBar::slotSearchStringChanged);
    connect(d->searchLine, &StatusSearchLine::statusChanged, this, &SearchBar::slotStatusChanged);


    connect(&(d->timer), &QTimer::timeout, this, &SearchBar::slotActivateSearch);
    d->timer.setSingleShot(true);
}

SearchBar::~SearchBar()
{
    delete d;
    d = 0;
}

QString SearchBar::text() const
{
    return d->searchText;
}

int SearchBar::status() const
{
    return static_cast<int>(d->searchLine->status());
}

void SearchBar::setDelay(int ms)
{
    d->delay = ms;
}

int SearchBar::delay() const
{
    return d->delay;
}

void SearchBar::setFocusSearchLine()
{
    d->searchLine->setFocus();
}

void SearchBar::slotClearSearch()
{
    if (status() != 0 || !d->searchLine->text().trimmed().isEmpty()) {
        d->searchLine->clear();
        d->searchLine->setStatus(Akregator::StatusSearchLine::AllArticles);
        d->timer.stop();
        slotActivateSearch();
    }
}

void SearchBar::slotSetStatus(int status)
{
    d->searchLine->setStatus(static_cast<Akregator::StatusSearchLine::Status>(status));
    d->triggerTimer();
}

void SearchBar::slotSetText(const QString &text)
{
    d->searchLine->setText(text);
    d->triggerTimer();
}

void SearchBar::slotStatusChanged(Akregator::StatusSearchLine::Status /*status*/)
{
    d->triggerTimer();
}

std::vector<QSharedPointer<const AbstractMatcher> > SearchBar::matchers() const
{
    return d->matchers;
}

void SearchBar::slotSearchStringChanged(const QString &search)
{
    d->searchText = search;
    d->triggerTimer();
}

void SearchBar::slotActivateSearch()
{
    QVector<Criterion> textCriteria;
    QVector<Criterion> statusCriteria;

    if (!d->searchText.isEmpty()) {
        Criterion subjCrit(Criterion::Title, Criterion::Contains, d->searchText);
        textCriteria << subjCrit;
        Criterion crit1(Criterion::Description, Criterion::Contains, d->searchText);
        textCriteria << crit1;
        Criterion authCrit(Criterion::Author, Criterion::Contains, d->searchText);
        textCriteria << authCrit;
    }

    switch (d->searchLine->status()) {
    case StatusSearchLine::AllArticles:
        break;
    case StatusSearchLine::NewArticles: {
        Criterion crit(Criterion::Status, Criterion::Equals, New);
        statusCriteria << crit;
        break;
    }
    case StatusSearchLine::UnreadArticles: {
        Criterion crit1(Criterion::Status, Criterion::Equals, New);
        Criterion crit2(Criterion::Status, Criterion::Equals, Unread);
        statusCriteria << crit1;
        statusCriteria << crit2;
        break;
    }
    case StatusSearchLine::ImportantArticles: {
        Criterion crit(Criterion::KeepFlag, Criterion::Equals, true);
        statusCriteria << crit;
        break;
    }
    }

    std::vector<QSharedPointer<const AbstractMatcher> > matchers;

    matchers.push_back(QSharedPointer<const AbstractMatcher>(new ArticleMatcher(textCriteria, ArticleMatcher::LogicalOr)));
    matchers.push_back(QSharedPointer<const AbstractMatcher>(new ArticleMatcher(statusCriteria, ArticleMatcher::LogicalOr)));
    Settings::setStatusFilter(d->searchLine->status());
    Settings::setTextFilter(d->searchText);
    d->matchers = matchers;
    Q_EMIT signalSearch(matchers);
}
