/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "propertiesdialog.h"

#include <klineedit.h>
#include <kpassdlg.h>
#include <klocale.h>

#include <qcheckbox.h>
#include <qbuttongroup.h>

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

const QString FeedPropertiesDialog::feedName() const
{
   return widget->feedNameEdit->text();
}

const QString FeedPropertiesDialog::url() const
{
   return widget->urlEdit->text();
}

void FeedPropertiesDialog::setFeedName(const QString& title)
{
   widget->feedNameEdit->setText(title);
}

void FeedPropertiesDialog::setUrl(const QString& url)
{
   widget->urlEdit->setText(url);
}

void FeedPropertiesDialog::selectFeedName()
{
   widget->feedNameEdit->selectAll();
}

#include "propertiesdialog.moc"
