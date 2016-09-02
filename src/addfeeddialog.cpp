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

#include "akregator_debug.h"
#include <kiconloader.h>
#include <kicontheme.h>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <ksqueezedtextlabel.h>
#include <QUrl>
#include <QCheckBox>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace Akregator;
AddFeedWidget::AddFeedWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    pixmapLabel1->setPixmap(QIcon::fromTheme(QStringLiteral("applications-internet")).pixmap(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop)));
    statusLabel->setText(QString());
}

AddFeedWidget::~AddFeedWidget()
{
}

QSize AddFeedDialog::sizeHint() const
{
    QSize sh = QDialog::sizeHint();
    sh.setHeight(minimumSize().height());
    sh.setWidth(sh.width() * 1.2);
    return sh;
}

Feed *AddFeedDialog::feed() const
{
    return m_feed;
}

AddFeedDialog::AddFeedDialog(QWidget *parent, const QString &name)
    : QDialog(parent), m_feed(0)
{
    setObjectName(name);
    setWindowTitle(i18n("Add Feed"));
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    widget = new AddFeedWidget(this);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AddFeedDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AddFeedDialog::reject);

    mainLayout->addWidget(widget);
    mainLayout->addWidget(buttonBox);

    widget->urlEdit->setFocus();
    connect(widget->urlEdit, &QLineEdit::textChanged, this, &AddFeedDialog::textChanged);
    mOkButton->setEnabled(false);
}

AddFeedDialog::~AddFeedDialog()
{}

void AddFeedDialog::setUrl(const QString &t)
{
    widget->urlEdit->setText(t);
}

void AddFeedDialog::accept()
{
    mOkButton->setEnabled(false);
    feedUrl = widget->urlEdit->text().trimmed();

    delete m_feed;
    m_feed = new Feed(Kernel::self()->storage());

    // HACK: make weird wordpress links ("feed:http://foobar/rss") work
    if (feedUrl.startsWith(QStringLiteral("feed:http"))) {
        feedUrl = feedUrl.right(feedUrl.length() - 5);
    }

    if (feedUrl.indexOf(QStringLiteral(":/")) == -1) {
        feedUrl.prepend(QStringLiteral("http://"));
    }

    QUrl asUrl(feedUrl);
    if (asUrl.scheme() == QLatin1String("feed")) {
        asUrl.setScheme(QStringLiteral("http"));
        feedUrl = asUrl.url();
    }
    m_feed->setXmlUrl(feedUrl);

    widget->statusLabel->setText(i18n("Downloading %1", feedUrl));

    connect(m_feed, &Feed::fetched, this, &AddFeedDialog::fetchCompleted);
    connect(m_feed, &Feed::fetchError, this, &AddFeedDialog::fetchError);
    connect(m_feed, &Feed::fetchDiscovery, this, &AddFeedDialog::fetchDiscovery);

    m_feed->fetch(true);
}

void AddFeedDialog::fetchCompleted(Feed * /*f*/)
{
    QDialog::accept();
}

void AddFeedDialog::fetchError(Feed *)
{
    KMessageBox::error(this, i18n("Feed not found from %1.", feedUrl));
    QDialog::reject();
}

void AddFeedDialog::fetchDiscovery(Feed *f)
{
    widget->statusLabel->setText(i18n("Feed found, downloading..."));
    feedUrl = f->xmlUrl();
}

void AddFeedDialog::textChanged(const QString &text)
{
    mOkButton->setEnabled(!text.trimmed().isEmpty());
}
