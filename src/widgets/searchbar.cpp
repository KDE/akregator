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

#include "article.h"
#include "widgets/statussearchline.h"

#include <kcombobox.h>
#include <klineedit.h>
#include <KLocalizedString>

#include <QString>
#include <QTimer>
#include <QStandardPaths>
#include <QHBoxLayout>

using namespace Akregator;
using namespace Akregator::Filters;

SearchBar::SearchBar(QWidget *parent)
    : QWidget(parent)
{
    m_delay = 400;
    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->setMargin(2);
    layout->setSpacing(5);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));

    m_searchLine = new StatusSearchLine(this);
    m_searchLine->setClearButtonEnabled(true);
    m_searchLine->setPlaceholderText(i18n("Search articles..."));
    layout->addWidget(m_searchLine);

    connect(m_searchLine, &KLineEdit::textChanged, this, &SearchBar::slotSearchStringChanged);
    connect(m_searchLine, &StatusSearchLine::forceLostFocus, this, &SearchBar::forceLostFocus);
    connect(m_searchLine, &StatusSearchLine::statusChanged, this, &SearchBar::slotStatusChanged);

    connect(&(m_timer), &QTimer::timeout, this, &SearchBar::slotActivateSearch);
    m_timer.setSingleShot(true);
}

SearchBar::~SearchBar()
{
}

QString SearchBar::text() const
{
    return m_searchText;
}

StatusSearchLine::Status SearchBar::status() const
{
    return m_searchLine->status();
}

void SearchBar::setDelay(int ms)
{
    m_delay = ms;
}

int SearchBar::delay() const
{
    return m_delay;
}

void SearchBar::setFocusSearchLine()
{
    m_searchLine->setFocus();
}

void SearchBar::slotClearSearch()
{
    if (status() != Akregator::StatusSearchLine::AllArticles || !m_searchLine->text().trimmed().isEmpty()) {
        m_searchLine->clear();
        m_searchLine->setStatus(Akregator::StatusSearchLine::AllArticles);
        m_timer.stop();
        slotStopActiveSearch();
    }
}

void SearchBar::slotSetStatus(int status)
{
    m_searchLine->setStatus(static_cast<Akregator::StatusSearchLine::Status>(status));
    triggerTimer();
}

void SearchBar::slotSetText(const QString &text)
{
    m_searchLine->setText(text);
    triggerTimer();
}

void SearchBar::slotStatusChanged(Akregator::StatusSearchLine::Status /*status*/)
{
    triggerTimer();
}

std::vector<QSharedPointer<const AbstractMatcher> > SearchBar::matchers() const
{
    return m_matchers;
}

void SearchBar::updateQuickSearchLineText(const QString &searchLineShortcut)
{
    m_searchLine->setPlaceholderText(i18n("Search articles...<%1>", searchLineShortcut));
}

void SearchBar::slotSearchStringChanged(const QString &search)
{
    m_searchText = search;
    triggerTimer();
}

void SearchBar::slotStopActiveSearch()
{
    std::vector<QSharedPointer<const AbstractMatcher> > matchers;
    Settings::setStatusFilter(m_searchLine->status());
    Settings::setTextFilter(m_searchText);
    m_matchers = matchers;
    Q_EMIT signalSearch(matchers);
}

void SearchBar::slotActivateSearch()
{
    QVector<Criterion> textCriteria;
    QVector<Criterion> statusCriteria;

    if (!m_searchText.isEmpty()) {
        Criterion subjCrit(Criterion::Title, Criterion::Contains, m_searchText);
        textCriteria << subjCrit;
        Criterion crit1(Criterion::Description, Criterion::Contains, m_searchText);
        textCriteria << crit1;
        Criterion authCrit(Criterion::Author, Criterion::Contains, m_searchText);
        textCriteria << authCrit;
    }

    switch (m_searchLine->status()) {
    case StatusSearchLine::AllArticles:
        break;
    case StatusSearchLine::NewArticles:
    {
        Criterion crit(Criterion::Status, Criterion::Equals, New);
        statusCriteria << crit;
        break;
    }
    case StatusSearchLine::UnreadArticles:
    {
        Criterion crit1(Criterion::Status, Criterion::Equals, New);
        Criterion crit2(Criterion::Status, Criterion::Equals, Unread);
        statusCriteria << crit1;
        statusCriteria << crit2;
        break;
    }
    case StatusSearchLine::ImportantArticles:
    {
        Criterion crit(Criterion::KeepFlag, Criterion::Equals, true);
        statusCriteria << crit;
        break;
    }
    }

    std::vector<QSharedPointer<const AbstractMatcher> > matchers;
    if (!textCriteria.isEmpty()) {
        matchers.push_back(QSharedPointer<const AbstractMatcher>(new ArticleMatcher(textCriteria, ArticleMatcher::LogicalOr)));
    }
    if (!statusCriteria.isEmpty()) {
        matchers.push_back(QSharedPointer<const AbstractMatcher>(new ArticleMatcher(statusCriteria, ArticleMatcher::LogicalOr)));
    }
    Settings::setStatusFilter(m_searchLine->status());
    Settings::setTextFilter(m_searchText);
    m_matchers = matchers;
    Q_EMIT signalSearch(matchers);
}

void Akregator::SearchBar::triggerTimer()
{
    if (m_timer.isActive()) {
        m_timer.stop();
    }

    m_timer.start(200);
}
