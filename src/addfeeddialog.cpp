/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#include "addfeeddialog.h"
#include <klineedit.h>
#include <qcheckbox.h>

using namespace Akregator;

AddFeedDialog::AddFeedDialog(QWidget *parent, const char *name)
        : AddFeedDialogBase(parent, name)
{
    connect( ljUserEdit, SIGNAL(textChanged(const QString&)),
                   this, SLOT(setUrlForLjUserName(const QString&)) );
}

AddFeedDialog::~AddFeedDialog()
{}

void AddFeedDialog::setUrlForLjUserName(const QString &userName)
{
    if (ljUserChkbox->isChecked())
        urlEdit->setText( "http://www.livejournal.com/~" + userName + "/data/rss" );
}

#include "addfeeddialog.moc"
