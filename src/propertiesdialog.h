/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
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

         const QString feedName() const;
         const QString url() const;
         bool autoFetch() const;
         int fetchInterval() const;
         Feed::ArchiveMode archiveMode() const;
         int maxArticleAge() const;
         int maxArticleNumber() const;
         bool markImmediatelyAsRead() const;
               
         void setFeedName(const QString& title);
         void setUrl(const QString& url);
         void setAutoFetch(bool);
         void setFetchInterval(int);
         void setArchiveMode(Feed::ArchiveMode mode);
         void setMaxArticleAge(int age);
         void setMaxArticleNumber(int number);
         void setMarkImmediatelyAsRead(bool enabled);
         void selectFeedName();
      private:
         FeedPropertiesWidget *widget;
   };
}

#endif
