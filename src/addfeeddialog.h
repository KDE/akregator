/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORADDFEEDDIALOG_H
#define AKREGATORADDFEEDDIALOG_H

#include "addfeedwidgetbase.h"
#include "librss/librss.h"

#include <kdialogbase.h>

using namespace RSS;

namespace Akregator
{
   class AddFeedWidget : public AddFeedWidgetBase
   {
      Q_OBJECT
      public:
         AddFeedWidget(QWidget *parent = 0, const char *name = 0);
         ~AddFeedWidget();

      private slots:
         void setUrlForLjUserName(const QString&);
   };

   class AddFeedDialog : public KDialogBase
   {
      Q_OBJECT
      public:
         AddFeedDialog(QWidget *parent = 0, const char *name = 0);
         ~AddFeedDialog();

         QString feedTitle;
         QString feedURL;
         
         void setURL(const QString& t);

      public slots:
         void slotOk( );
         void fetchCompleted(Loader*, Document, Status);

      private:
         AddFeedWidget *widget;
   };
}

#endif
