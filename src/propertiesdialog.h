/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORPROPDIALOG_H
#define AKREGATORPROPDIALOG_H

#include "propertieswidgetbase.h"
#include "feed.h"

#include <kdialogbase.h>

namespace Akregator
{
   class FeedPropertiesWidget : public FeedPropertiesWidgetBase
   {
      Q_OBJECT
      public:
         FeedPropertiesWidget(QWidget *parent = 0, const char *name = 0);
         ~FeedPropertiesWidget();

      private slots:
         void setUrlForLjUserName(const QString&);
   };

   class FeedPropertiesDialog : public KDialogBase
   {
      Q_OBJECT
      public:
         FeedPropertiesDialog(QWidget *parent = 0, const char *name = 0);
         ~FeedPropertiesDialog();

         const QString feedName() const;
         const QString url() const;
         Feed::LJAuthMode authMode() const;
         const QString ljLogin() const;
         const QString ljPassword() const;

         void setFeedName(const QString& title);
         void setUrl(const QString& url);
         void setAuthMode(Feed::LJAuthMode authMode);
         void setLjLogin(const QString& ljLogin);
         void setLjPassword(const QString& ljPassword);

         void selectFeedName();
      private:
         FeedPropertiesWidget *widget;
   };
}

#endif
