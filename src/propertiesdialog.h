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
    };

    class FeedPropertiesDialog : public KDialogBase
    {
        Q_OBJECT
        public:
            FeedPropertiesDialog(QWidget *parent = 0, const char *name = 0);
            ~FeedPropertiesDialog();
            void setUrlForLjUserName(const QString &userName);
            FeedPropertiesWidget *widget;
    };
};

#endif
