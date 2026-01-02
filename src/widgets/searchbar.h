/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include "articlematcher.h"
#include "widgets/statussearchbuttons.h"
#include <QSharedPointer>
#include <QTimer>
#include <QWidget>
#include <vector>

namespace Akregator
{
namespace Filters
{
class AbstractMatcher;
}
class StatusSearchLine;
class SearchBar : public QWidget
{
    Q_OBJECT

public:
    explicit SearchBar(QWidget *parent = nullptr);
    ~SearchBar() override;

    [[nodiscard]] QString text() const;
    [[nodiscard]] StatusSearchButtons::Status status() const;

    void setFocusSearchLine();
    [[nodiscard]] std::vector<QSharedPointer<const Filters::AbstractMatcher>> matchers() const;
    void updateQuickSearchLineText(const QString &searchLine);
    void slotClearSearch();
    void slotSetStatus(int status);
    void slotSetText(const QString &text);

Q_SIGNALS:
    /** emitted when the text and status filters were updated. Params are textfilter, statusfilter */
    void signalSearch(const std::vector<QSharedPointer<const Akregator::Filters::AbstractMatcher>> &);
    void forceLostFocus();

private:
    void slotSearchStringChanged(const QString &search);
    void slotStopActiveSearch();
    void slotActivateSearch();
    void slotStatusChanged(Akregator::StatusSearchButtons::Status);
    void triggerTimer();

    QString m_searchText;
    QTimer m_timer;
    StatusSearchLine *const m_searchLine;
    StatusSearchButtons *const m_statusSearchButtons;
    const int m_delay;
    std::vector<QSharedPointer<const Filters::AbstractMatcher>> m_matchers;
};
} // namespace Akregator
