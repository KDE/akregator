/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORADDFEEDDIALOG_H
#define AKREGATORADDFEEDDIALOG_H

#include "addfeedwidgetbase.h"

#include <kdialogbase.h>

using namespace RSS;



namespace Akregator
{

   class Feed;

   class AddFeedWidget : public AddFeedWidgetBase
   {
      Q_OBJECT
      public:
         AddFeedWidget(QWidget *parent = 0, const char *name = 0);
         ~AddFeedWidget();
   };

   class AddFeedDialog : public KDialogBase
   {
      Q_OBJECT
      public:
         AddFeedDialog(QWidget *parent = 0, const char *name = 0);
         ~AddFeedDialog();

         QString feedURL;
         Feed *feed;
         
         void setURL(const QString& t);

      public slots:
         void slotOk( );
         void fetchCompleted(Feed *);
         void fetchDiscovery(Feed *);
         void fetchError(Feed *);

      private:
         AddFeedWidget *widget;
   };
}

#endif
