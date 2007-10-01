/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "addfeeddialog.h"
#include "feed.h"
#include "kernel.h"

#include <kdebug.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksqueezedtextlabel.h>
#include <kurl.h>

#include <QCheckBox>

namespace Akregator {

AddFeedWidget::AddFeedWidget(QWidget *parent, const char* name)
   : QWidget(parent)
{
    setObjectName(name);
    setupUi(this);
    pixmapLabel1->setPixmap(KIconLoader::global()->loadIcon( "package-network",KIconLoader::Desktop,KIconLoader::SizeHuge, KIconLoader::DefaultState, QStringList(), 0, true));
    statusLabel->setText(QString());
}

AddFeedWidget::~AddFeedWidget()
{}

Feed* AddFeedDialog::feed()
{
    return m_feed;
}

AddFeedDialog::AddFeedDialog(QWidget *parent, const char *name)
   : KDialog(parent
     /*Qt::WStyle_DialogBorder*/), m_feed( 0 )
{
    setObjectName(name);
    widget = new AddFeedWidget(this);
    setCaption(i18n("Add Feed"));
    setButtons(KDialog::Ok|KDialog::Cancel);
    setDefaultButton(KDialog::Cancel);
    connect(widget->urlEdit, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    enableButtonOk(false);
    setMainWidget(widget);
}

AddFeedDialog::~AddFeedDialog()
{}

void AddFeedDialog::setUrl(const QString& t)
{
    widget->urlEdit->setText(t);
}

void AddFeedDialog::accept()
{
    enableButtonOk(false);
    feedUrl = widget->urlEdit->text().trimmed();

    delete m_feed;
    m_feed = new Feed( Kernel::self()->storage() );

    // HACK: make weird wordpress links ("feed:http://foobar/rss") work
    if (feedUrl.startsWith("feed:"))
        feedUrl = feedUrl.right( feedUrl.length() - 5 );

    if (feedUrl.indexOf(":/") == -1)
        feedUrl.prepend("http://");
    m_feed->setXmlUrl(feedUrl);

    widget->statusLabel->setText( i18n("Downloading %1", feedUrl) );

    connect( m_feed, SIGNAL(fetched(Akregator::Feed* )),
             this, SLOT(fetchCompleted(Akregator::Feed *)) );
    connect( m_feed, SIGNAL(fetchError(Akregator::Feed* )),
             this, SLOT(fetchError(Akregator::Feed *)) );
    connect( m_feed, SIGNAL(fetchDiscovery(Akregator::Feed* )),
             this, SLOT(fetchDiscovery(Akregator::Feed *)) );

    m_feed->fetch(true);
}

void AddFeedDialog::fetchCompleted(Feed */*f*/)
{
    KDialog::accept();
}

void AddFeedDialog::fetchError(Feed *)
{
    KMessageBox::error(this, i18n("Feed not found from %1.", feedUrl));
    KDialog::reject();
}

void AddFeedDialog::fetchDiscovery(Feed *f)
{
    widget->statusLabel->setText( i18n("Feed found, downloading...") );
    feedUrl=f->xmlUrl();
}

void AddFeedDialog::textChanged(const QString& text)
{
    enableButtonOk(!text.isEmpty());
}

} // namespace Akregator

#include "addfeeddialog.moc"
