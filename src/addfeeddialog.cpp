/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "archive.h"
#include "feed.h"
#include "addfeeddialog.h"

#include <qcheckbox.h>

#include <kapplication.h>
#include <kurl.h>
#include <klocale.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <kdebug.h>
#include <ksqueezedtextlabel.h>
#include <kmessagebox.h>

using namespace Akregator;

AddFeedWidget::AddFeedWidget(QWidget *parent, const char *name)
   : AddFeedWidgetBase(parent, name)
{
    pixmapLabel1->setPixmap(kapp->iconLoader()->loadIcon( "package_network",KIcon::Desktop,KIcon::SizeHuge, KIcon::DefaultState, 0, true));
    statusLabel->setText(QString::null);
}

AddFeedWidget::~AddFeedWidget()
{}

AddFeedDialog::AddFeedDialog(QWidget *parent, const char *name)
   : KDialogBase(KDialogBase::Swallow, Qt::WStyle_DialogBorder, parent, name, true, i18n("Add Feed"), KDialogBase::Ok|KDialogBase::Cancel)
{
    widget = new AddFeedWidget(this);
    setMainWidget(widget);
}

AddFeedDialog::~AddFeedDialog()
{}

void AddFeedDialog::setURL(const QString& t)
{
    widget->urlEdit->setText(t);
}

void AddFeedDialog::slotOk( )
{
    enableButtonOK(false);
    feedURL = widget->urlEdit->text();

    Feed *f=new Feed();

    feed=f;
    if (feedURL.find(":/") == -1)
        feedURL.prepend("http://");
    f->setXmlUrl(feedURL);

    widget->statusLabel->setText( i18n("Downloading %1").arg(feedURL) );

    connect( feed, SIGNAL(fetched(Feed* )),
             this, SLOT(fetchCompleted(Feed *)) );
    connect( feed, SIGNAL(fetchError(Feed* )),
             this, SLOT(fetchError(Feed *)) );
    connect( feed, SIGNAL(fetchDiscovery(Feed* )),
             this, SLOT(fetchDiscovery(Feed *)) );

    f->fetch(true);
}

void AddFeedDialog::fetchCompleted(Feed *f)
{
   KDialogBase::slotOk();
}

void AddFeedDialog::fetchError(Feed *)
{
    KMessageBox::error(this, i18n("Feed not found from %1.").arg(feedURL));
    KDialogBase::slotCancel();
}

void AddFeedDialog::fetchDiscovery(Feed *f)
{
	widget->statusLabel->setText( i18n("Feed found, downloading...") );
    feedURL=f->xmlUrl();
}

#include "addfeeddialog.moc"
