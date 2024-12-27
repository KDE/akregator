/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "searchbar.h"
#include "akregatorconfig.h"
#include "statussearchbuttons.h"
#include "statussearchline.h"

#include "article.h"
#include <TextUtils/ConvertText>

#include <KLineEdit>
#include <KLocalizedString>

#include <QFrame>
#include <QHBoxLayout>
#include <QStandardPaths>
#include <QString>
#include <chrono>

using namespace std::chrono_literals;

using namespace Akregator;
using namespace Akregator::Filters;

SearchBar::SearchBar(QWidget *parent)
    : QWidget(parent)
    , m_searchLine(new StatusSearchLine(this))
    , m_statusSearchButtons(new StatusSearchButtons(this))
    , m_delay(400)
{
    auto layout = new QHBoxLayout(this);

    layout->setContentsMargins({});
    layout->setSpacing(0);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));

    m_searchLine->setClearButtonEnabled(true);
    m_searchLine->setPlaceholderText(i18nc("@info:placeholder", "Search articles…"));

    auto verticalSeparator = new QFrame(this);
    verticalSeparator->setFrameStyle(QFrame::VLine);
    verticalSeparator->setFixedWidth(1);

    layout->addWidget(m_searchLine);
    layout->addWidget(verticalSeparator);

    auto horizontalSeparator = new QFrame(this);
    horizontalSeparator->setFrameStyle(QFrame::HLine);
    horizontalSeparator->setFixedHeight(1);

    auto vbox = new QVBoxLayout;
    vbox->setContentsMargins({});
    vbox->setSpacing(0);

    vbox->addWidget(m_statusSearchButtons);
    vbox->addWidget(horizontalSeparator);

    layout->addLayout(vbox);

    connect(m_searchLine, &KLineEdit::textChanged, this, &SearchBar::slotSearchStringChanged);
    connect(m_searchLine, &StatusSearchLine::forceLostFocus, this, &SearchBar::forceLostFocus);
    connect(m_statusSearchButtons, &StatusSearchButtons::statusChanged, this, &SearchBar::slotStatusChanged);

    connect(&m_timer, &QTimer::timeout, this, &SearchBar::slotActivateSearch);
    m_timer.setSingleShot(true);
}

SearchBar::~SearchBar() = default;

QString SearchBar::text() const
{
    return m_searchText;
}

StatusSearchButtons::Status SearchBar::status() const
{
    return m_statusSearchButtons->status();
}

void SearchBar::setFocusSearchLine()
{
    m_searchLine->setFocus();
}

void SearchBar::slotClearSearch()
{
    if (status() != Akregator::StatusSearchButtons::AllArticles || !m_searchLine->text().trimmed().isEmpty()) {
        m_searchLine->clear();
        m_statusSearchButtons->setStatus(Akregator::StatusSearchButtons::AllArticles);
        m_statusSearchButtons->setStatus(Akregator::StatusSearchButtons::AllArticles);
        m_timer.stop();
        slotStopActiveSearch();
    }
}

void SearchBar::slotSetStatus(int status)
{
    m_statusSearchButtons->setStatus(static_cast<Akregator::StatusSearchButtons::Status>(status));
    m_statusSearchButtons->setStatus(static_cast<Akregator::StatusSearchButtons::Status>(status));
    triggerTimer();
}

void SearchBar::slotSetText(const QString &text)
{
    m_searchLine->setText(text);
    triggerTimer();
}

void SearchBar::slotStatusChanged(Akregator::StatusSearchButtons::Status /*status*/)
{
    triggerTimer();
}

std::vector<QSharedPointer<const AbstractMatcher>> SearchBar::matchers() const
{
    return m_matchers;
}

void SearchBar::updateQuickSearchLineText(const QString &searchLineShortcut)
{
    m_searchLine->setPlaceholderText(i18nc("@info:placeholder", "Search articles…<%1>", searchLineShortcut));
}

void SearchBar::slotSearchStringChanged(const QString &search)
{
    m_searchText = search;
    triggerTimer();
}

void SearchBar::slotStopActiveSearch()
{
    std::vector<QSharedPointer<const AbstractMatcher>> matchers;
    Settings::setStatusFilter(m_statusSearchButtons->status());
    Settings::setTextFilter(m_searchText);
    m_matchers = matchers;
    Q_EMIT signalSearch(matchers);
}

void SearchBar::slotActivateSearch()
{
    QList<Criterion> textCriteria;
    QList<Criterion> statusCriteria;

    if (!m_searchText.isEmpty()) {
        const QString searchTextDiacritic(TextUtils::ConvertText::normalize(m_searchText));
        textCriteria.reserve(3);
        Criterion subjCrit(Criterion::Title, Criterion::Contains, searchTextDiacritic);
        textCriteria << subjCrit;
        Criterion crit1(Criterion::Description, Criterion::Contains, searchTextDiacritic);
        textCriteria << crit1;
        Criterion authCrit(Criterion::Author, Criterion::Contains, searchTextDiacritic);
        textCriteria << authCrit;
    }

    switch (m_statusSearchButtons->status()) {
    case StatusSearchButtons::AllArticles:
        break;
    case StatusSearchButtons::NewArticles: {
        Criterion crit(Criterion::Status, Criterion::Equals, New);
        statusCriteria << crit;
        break;
    }
    case StatusSearchButtons::UnreadArticles: {
        Criterion crit1(Criterion::Status, Criterion::Equals, New);
        Criterion crit2(Criterion::Status, Criterion::Equals, Unread);
        statusCriteria << crit1;
        statusCriteria << crit2;
        break;
    }
    case StatusSearchButtons::ReadArticles: {
        Criterion crit(Criterion::Status, Criterion::Equals, Read);
        statusCriteria << crit;
        break;
    }
    case StatusSearchButtons::ImportantArticles: {
        Criterion crit(Criterion::KeepFlag, Criterion::Equals, true);
        statusCriteria << crit;
        break;
    }
    }

    std::vector<QSharedPointer<const AbstractMatcher>> matchers;
    if (!textCriteria.isEmpty()) {
        matchers.push_back(QSharedPointer<const AbstractMatcher>(new ArticleMatcher(textCriteria, ArticleMatcher::LogicalOr)));
    }
    if (!statusCriteria.isEmpty()) {
        matchers.push_back(QSharedPointer<const AbstractMatcher>(new ArticleMatcher(statusCriteria, ArticleMatcher::LogicalOr)));
    }
    Settings::setStatusFilter(m_statusSearchButtons->status());
    Settings::setTextFilter(m_searchText);
    m_matchers = matchers;
    Q_EMIT signalSearch(matchers);
}

void Akregator::SearchBar::triggerTimer()
{
    if (m_timer.isActive()) {
        m_timer.stop();
    }

    m_timer.start(200ms);
}

#include "moc_searchbar.cpp"
