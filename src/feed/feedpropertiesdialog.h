/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_FEEDPROPERTIESDIALOG_H
#define AKREGATOR_FEEDPROPERTIESDIALOG_H

#include "feed.h"
#include "ui_feedpropertieswidgetbase.h"

#include <QDialog>

#include <QWidget>

namespace Akregator {
class FeedPropertiesWidget : public QWidget, public Ui::FeedPropertiesWidgetBase
{
    Q_OBJECT
public:
    explicit FeedPropertiesWidget(QWidget *parent = nullptr, const QString &name = QString());
    ~FeedPropertiesWidget();

    enum IntervalStep {
        Minutes = 0,
        Hours,
        Days,
        Never
    };

public Q_SLOTS:
    void slotUpdateComboBoxActivated(int index);
    void slotUpdateComboBoxLabels(int value);
    void slotUpdateCheckBoxToggled(bool enabled);
};

class FeedPropertiesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FeedPropertiesDialog(QWidget *parent = nullptr, const QString &name = QString());
    ~FeedPropertiesDialog() override;

    void setFeed(Feed *feed);

    /** selects the text in the feed title lineedit */
    void selectFeedName();

public Q_SLOTS:

    void accept() override;

protected:
    Q_REQUIRED_RESULT QString comment() const;
    Q_REQUIRED_RESULT QString feedName() const;
    Q_REQUIRED_RESULT QString url() const;
    Q_REQUIRED_RESULT bool autoFetch() const;
    Q_REQUIRED_RESULT int fetchInterval() const;
    Q_REQUIRED_RESULT Feed::ArchiveMode archiveMode() const;
    Q_REQUIRED_RESULT int maxArticleAge() const;
    Q_REQUIRED_RESULT int maxArticleNumber() const;
    Q_REQUIRED_RESULT bool markImmediatelyAsRead() const;
    Q_REQUIRED_RESULT bool useNotification() const;
    Q_REQUIRED_RESULT bool loadLinkedWebsite() const;

    void setFeedName(const QString &title);
    void setUrl(const QString &url);
    void setAutoFetch(bool);
    void setFetchInterval(int);
    void setArchiveMode(Feed::ArchiveMode mode);
    void setMaxArticleAge(int age);
    void setMaxArticleNumber(int number);
    void setMarkImmediatelyAsRead(bool enabled);
    void setUseNotification(bool enabled);
    void setLoadLinkedWebsite(bool enabled);

    void setComment(const QString &comment);
private:
    FeedPropertiesWidget *widget = nullptr;
    Feed *m_feed = nullptr;
    QPushButton *mOkButton = nullptr;

private Q_SLOTS:
    void slotSetWindowTitle(const QString &);
};
} // namespace Akregator

#endif // AKREGATOR_FEEDPROPERTIESDIALOG_H
