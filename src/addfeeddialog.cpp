/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

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

using namespace Akregator;

AddFeedWidget::AddFeedWidget(QWidget *parent, const char *name)
   : AddFeedWidgetBase(parent, name)
{
    pixmapLabel1->setPixmap(kapp->iconLoader()->loadIcon( "package_network",KIcon::Desktop,KIcon::SizeHuge, KIcon::DefaultState, 0, true));
    statusLabel->setText(QString::null);

    connect( ljUserEdit, SIGNAL(textChanged(const QString&)),
                   this, SLOT(setUrlForLjUserName(const QString&)) );
}

AddFeedWidget::~AddFeedWidget()
{}

void AddFeedWidget::setUrlForLjUserName(const QString &userName)
{
    if (ljUserChkbox->isChecked())
        urlEdit->setText( "http://www.livejournal.com/~" + userName + "/data/rss" );
}


AddFeedDialog::AddFeedDialog(QWidget *parent, const char *name)
   : KDialogBase(KDialogBase::Swallow, Qt::WStyle_DialogBorder, parent, name, true, i18n("Add Feed"), KDialogBase::Ok|KDialogBase::Cancel)
{
    feedTitle = i18n("New Feed");

    widget = new AddFeedWidget(this);
    setMainWidget(widget);
}

AddFeedDialog::~AddFeedDialog()
{}

void AddFeedDialog::slotOk( )
{
    enableButtonOK(false);
    feedURL = widget->urlEdit->text();
    widget->statusLabel->setText( i18n("Downloading %1").arg(feedURL) );

    Loader *loader = Loader::create( this, SLOT(fetchCompleted(Loader *, Document, Status)) );
    loader->loadFrom( feedURL, new FileRetriever );
}

void AddFeedDialog::fetchCompleted(Loader *l, Document doc, Status status)
{
    if (status==RetrieveError)
    {
        KDialogBase::slotOk();
    }
    else if (status==ParseError)
    {
        if (l->discoveredFeedURL().isValid())
        {
            widget->statusLabel->setText( i18n("Feed found, downloading...") );
            feedURL=l->discoveredFeedURL().url();

            Loader *loader = Loader::create( this, SLOT(fetchCompleted(Loader *, Document, Status)) );
            loader->loadFrom( l->discoveredFeedURL(), new FileRetriever );
        }
        else
            KDialogBase::slotOk();
    }
    else
    {
        feedTitle=doc.title();
        KDialogBase::slotOk();
    }
}

#include "addfeeddialog.moc"
