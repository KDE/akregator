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
#include <knuminput.h>

#include <qcheckbox.h>
#include <qbuttongroup.h>

using namespace Akregator;

FeedPropertiesWidget::FeedPropertiesWidget(QWidget *parent, const char *name)
        : FeedPropertiesWidgetBase(parent, name)
{
   connect(      upChkbox, SIGNAL(setChecked(bool)),
            updateSpinBox, SLOT(setEnabled(bool)) );
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

bool FeedPropertiesDialog::autoFetch() const
{
   return widget->upChkbox->isChecked();
}

int FeedPropertiesDialog::fetchInterval() const
{
   return widget->updateSpinBox->value();
}

void FeedPropertiesDialog::setFeedName(const QString& title)
{
   widget->feedNameEdit->setText(title);
}

void FeedPropertiesDialog::setUrl(const QString& url)
{
   widget->urlEdit->setText(url);
}

void FeedPropertiesDialog::setAutoFetch(bool w)
{
   widget->upChkbox->setChecked(w);
}

void FeedPropertiesDialog::setFetchInterval(int i)
{
   widget->updateSpinBox->setValue(i);
}

void FeedPropertiesDialog::selectFeedName()
{
   widget->feedNameEdit->selectAll();
}

#include "propertiesdialog.moc"
