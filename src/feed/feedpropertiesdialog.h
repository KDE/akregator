/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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

#ifndef AKREGATOR_FEEDPROPERTIESDIALOG_H
#define AKREGATOR_FEEDPROPERTIESDIALOG_H

#include "feed.h"
#include "ui_feedpropertieswidgetbase.h"

#include <QDialog>

#include <QWidget>

namespace Akregator
{

class FeedPropertiesWidget : public QWidget, public Ui::FeedPropertiesWidgetBase
{
    Q_OBJECT
public:
    explicit FeedPropertiesWidget(QWidget *parent = Q_NULLPTR, const char *name = Q_NULLPTR);
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
    explicit FeedPropertiesDialog(QWidget *parent = Q_NULLPTR, const QString &name = QString());
    ~FeedPropertiesDialog();

    void setFeed(Feed *feed);

    /** selects the text in the feed title lineedit */
    void selectFeedName();

public Q_SLOTS:

    void accept() Q_DECL_OVERRIDE;

protected:
    QString feedName() const;
    QString url() const;
    bool autoFetch() const;
    int fetchInterval() const;
    Feed::ArchiveMode archiveMode() const;
    int maxArticleAge() const;
    int maxArticleNumber() const;
    bool markImmediatelyAsRead() const;
    bool useNotification() const;
    bool loadLinkedWebsite() const;

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

private:
    FeedPropertiesWidget *widget;
    Feed *m_feed;

private Q_SLOTS:
    void slotSetWindowTitle(const QString &);
};

} // namespace Akregator

#endif // AKREGATOR_FEEDPROPERTIESDIALOG_H

