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
    connect( ljUserEdit, SIGNAL(textChanged(const QString&)),
                   this, SLOT(setUrlForLjUserName(const QString&)) );
}

FeedPropertiesWidget::~FeedPropertiesWidget()
{}

void FeedPropertiesWidget::setUrlForLjUserName(const QString &userName)
{
    if (ljUserChkbox->isChecked())
        urlEdit->setText( "http://www.livejournal.com/~" + userName + "/data/rss" );
}


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

Feed::LJAuthMode FeedPropertiesDialog::authMode() const
{
   return widget->ljAuthMode->selectedId() == 1 ? Feed::AuthGlobal
        : widget->ljAuthMode->selectedId() == 2 ? Feed::AuthLocal
                                                : Feed::AuthNone;
}

const QString FeedPropertiesDialog::ljLogin() const
{
   return widget->loginEdit->text();
}

const QString FeedPropertiesDialog::ljPassword() const
{
   return widget->passwordEdit->text();
}

void FeedPropertiesDialog::setFeedName(const QString& title)
{
   widget->feedNameEdit->setText(title);
}

void FeedPropertiesDialog::setUrl(const QString& url)
{
   widget->urlEdit->setText(url);
}

void FeedPropertiesDialog::setAuthMode(Feed::LJAuthMode authMode)
{
   widget->ljAuthMode->setButton( authMode == Feed::AuthGlobal ? 1
                                : authMode == Feed::AuthLocal  ? 2
                                                               : 0 );
}

void FeedPropertiesDialog::setLjLogin(const QString& ljLogin)
{
   widget->loginEdit->setText(ljLogin);
}

void FeedPropertiesDialog::setLjPassword(const QString& ljPassword)
{
   widget->passwordEdit->setText(ljPassword);
}

#include "propertiesdialog.moc"
