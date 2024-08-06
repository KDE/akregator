/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "config-akregator.h"
#include "feed.h"
#include "ui_feedpropertieswidgetbase.h"
#include <QDialog>

#include <QWidget>
#if HAVE_ACTIVITY_SUPPORT
#include "activities/activitiesmanager.h"
#endif

#if HAVE_ACTIVITY_SUPPORT
namespace PimCommonActivities
{
class ConfigureActivitiesWidget;
}
#endif
namespace Akregator
{
class FeedPropertiesWidget : public QWidget, public Ui::FeedPropertiesWidgetBase
{
    Q_OBJECT
public:
    explicit FeedPropertiesWidget(QWidget *parent = nullptr);
    ~FeedPropertiesWidget() override;

    enum IntervalStep {
        Minutes = 0,
        Hours,
        Days,
        Never,
    };

#if HAVE_ACTIVITY_SUPPORT
    [[nodiscard]] PimCommonActivities::ConfigureActivitiesWidget *configureActivitiesWidget() const;
    [[nodiscard]] ActivitiesManager::ActivitySettings activitySettings() const;
    void setActivitiesSettings(const ActivitiesManager::ActivitySettings &activitySettings);
#endif

private:
    void slotUpdateComboBoxActivated(int index);
    void slotUpdateComboBoxLabels(int value);
    void slotUpdateCheckBoxToggled(bool enabled);

#if HAVE_ACTIVITY_SUPPORT
    PimCommonActivities::ConfigureActivitiesWidget *const mConfigureActivitiesWidget;
#endif
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
#if HAVE_ACTIVITY_SUPPORT
    void setActivitiesSettings(const ActivitiesManager::ActivitySettings &activitySettings);
    [[nodiscard]] ActivitiesManager::ActivitySettings activitySettings() const;
#endif

    [[nodiscard]] QString comment() const;
    [[nodiscard]] QString feedName() const;
    [[nodiscard]] QString url() const;
    [[nodiscard]] bool autoFetch() const;
    [[nodiscard]] int fetchInterval() const;
    [[nodiscard]] Feed::ArchiveMode archiveMode() const;
    [[nodiscard]] int maxArticleAge() const;
    [[nodiscard]] int maxArticleNumber() const;
    [[nodiscard]] bool markImmediatelyAsRead() const;
    [[nodiscard]] bool useNotification() const;
    [[nodiscard]] bool loadLinkedWebsite() const;

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
    FeedPropertiesWidget *const mFeedPropertiesWidget;
    Feed *m_feed = nullptr;
    QPushButton *mOkButton = nullptr;

private Q_SLOTS:
    void slotSetWindowTitle(const QString &);
};
} // namespace Akregator
