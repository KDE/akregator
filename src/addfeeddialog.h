/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORADDFEEDDIALOG_H
#define AKREGATORADDFEEDDIALOG_H

#include "addfeeddialogbase.h"

namespace Akregator
{
    class AddFeedDialog : public AddFeedDialogBase
    {
        Q_OBJECT
        public:
            AddFeedDialog(QWidget *parent = 0, const char *name = 0);
            ~AddFeedDialog();

        public slots:
            void setUrlForLjUserName(const QString&);
    };
};

#endif
