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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef AKREGATORPROPDIALOG_H
#define AKREGATORPROPDIALOG_H

#include "propertieswidgetbase.h"

#include <kdialogbase.h>

namespace Akregator
{
   class FeedPropertiesWidget : public FeedPropertiesWidgetBase
   {
      Q_OBJECT
      public:
         FeedPropertiesWidget(QWidget *parent = 0, const char *name = 0);
         ~FeedPropertiesWidget();
         virtual void slotUpdateComboBoxActivated(int index);
         virtual void slotUpdateCheckBoxToggled(bool enabled);

   };

   class FeedPropertiesDialog : public KDialogBase
   {
      Q_OBJECT
      public:
         FeedPropertiesDialog(QWidget *parent = 0, const char *name = 0);
         ~FeedPropertiesDialog();

         void setFeed(Feed* feed);
         
         /** selects the text in the feed title lineedit */
         void selectFeedName();
         
       protected:
         const QString feedName() const;
         const QString url() const;
         bool autoFetch() const;
         int fetchInterval() const;
         Feed::ArchiveMode archiveMode() const;
         int maxArticleAge() const;
         int maxArticleNumber() const;
         bool markImmediatelyAsRead() const;
         bool useNotification() const;

         void setFeedName(const QString& title);
         void setUrl(const QString& url);
         void setAutoFetch(bool);
         void setFetchInterval(int);
         void setArchiveMode(Feed::ArchiveMode mode);
         void setMaxArticleAge(int age);
         void setMaxArticleNumber(int number);
         void setMarkImmediatelyAsRead(bool enabled);
         void setUseNotification(bool enabled);
         
       protected slots:
           void slotOk();
           
      private:
         FeedPropertiesWidget *widget;
         Feed* m_feed;
   };
}

#endif
