/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "propertiesdialog.h"

#include <klineedit.h>
#include <klocale.h>

#include <qcheckbox.h>

using namespace Akregator;


FeedPropertiesWidget::FeedPropertiesWidget(QWidget *parent, const char *name)
        : FeedPropertiesWidgetBase(parent, name)
{

}

FeedPropertiesWidget::~FeedPropertiesWidget()
{}

FeedPropertiesDialog::FeedPropertiesDialog(QWidget *parent, const char *name)
        : KDialogBase(KDialogBase::Swallow, Qt::WStyle_DialogBorder, parent, name, true, i18n("Feed Properties"), KDialogBase::Ok|KDialogBase::Cancel)
{
    widget=new FeedPropertiesWidget(this);
    setMainWidget(widget);
}

FeedPropertiesDialog::~FeedPropertiesDialog()
{}

void FeedPropertiesDialog::setUrlForLjUserName(const QString &userName)
{
    if (widget->ljUserChkbox->isChecked())
        widget->urlEdit->setText( "http://www.livejournal.com/~" + userName + "/data/rss" );
}

#include "propertiesdialog.moc"
