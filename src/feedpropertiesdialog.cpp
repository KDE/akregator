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

#include "feedpropertiesdialog.h"
#include "akregatorconfig.h"

#include <kcombobox.h>
#include <klineedit.h>
#include <KLocalizedString>
#include <knuminput.h>
#include <kpassworddialog.h>

#include <QCheckBox>

namespace Akregator {

FeedPropertiesWidget::FeedPropertiesWidget(QWidget *parent, const char *name)
        : QWidget(parent)
{
    setObjectName(name);
    setupUi(this);
    connect( cb_updateInterval, SIGNAL(toggled(bool)),
             updateSpinBox, SLOT(setEnabled(bool)) );
    connect( cb_updateInterval, SIGNAL(toggled(bool)),
             updateComboBox, SLOT(setEnabled(bool)) );
    connect( cb_updateInterval, SIGNAL(toggled(bool)),
             updateLabel, SLOT(setEnabled(bool)) );
    connect( updateComboBox, SIGNAL(activated(int)),
             this, SLOT(slotUpdateComboBoxActivated(int)) );
    connect( updateSpinBox, SIGNAL(valueChanged(int)),
             this, SLOT(slotUpdateComboBoxLabels(int)) );
    connect( rb_limitArticleAge, SIGNAL(toggled(bool)),
             sb_maxArticleAge, SLOT(setEnabled(bool)) );
    connect( rb_limitArticleNumber, SIGNAL(toggled(bool)),
             sb_maxArticleNumber, SLOT(setEnabled(bool)) );
}

FeedPropertiesWidget::~FeedPropertiesWidget()
{}


void FeedPropertiesWidget::slotUpdateComboBoxActivated( int index )
{
    updateSpinBox->setEnabled( index != Never );
}


void FeedPropertiesWidget::slotUpdateComboBoxLabels( int value )
{
    updateComboBox->setItemText(FeedPropertiesWidget::Minutes, i18np("Minute", "Minutes", value));
    updateComboBox->setItemText(FeedPropertiesWidget::Hours, i18np("Hour", "Hours", value));
    updateComboBox->setItemText(FeedPropertiesWidget::Days, i18np("Day", "Days", value));
}


void FeedPropertiesWidget::slotUpdateCheckBoxToggled( bool enabled )
{
    updateSpinBox->setEnabled( enabled && updateComboBox->currentIndex() != Never );
}


FeedPropertiesDialog::FeedPropertiesDialog(QWidget *parent, const char *name)
        : KDialog(parent/*, Qt::WStyle_DialogBorder*/)
{
    setObjectName(name);
    widget=new FeedPropertiesWidget(this);
    setWindowTitle(i18n("Feed Properties"));
    setButtons(KDialog::Ok|KDialog::Cancel);
    setDefaultButton(KDialog::Ok);
    setModal(true);

    setMainWidget(widget);
    widget->feedNameEdit->setFocus();

    widget->updateComboBox->insertItem(FeedPropertiesWidget::Minutes, i18np("Minute", "Minutes", 0));
    widget->updateComboBox->insertItem(FeedPropertiesWidget::Hours, i18np("Hour", "Hours", 0));
    widget->updateComboBox->insertItem(FeedPropertiesWidget::Days, i18np("Day", "Days", 0));
    widget->updateComboBox->insertItem(FeedPropertiesWidget::Never,
                                       i18nc("never fetch new articles", "Never"));
    widget->sb_maxArticleAge->setSuffix(ki18np(" day", " days"));
    widget->sb_maxArticleNumber->setSuffix(ki18np(" article", " articles"));

    connect(widget->feedNameEdit, SIGNAL(textChanged(QString)), this, SLOT(slotSetWindowTitle(QString)));
}

FeedPropertiesDialog::~FeedPropertiesDialog()
{}

void FeedPropertiesDialog::accept()
{
     m_feed->setNotificationMode(false);
     m_feed->setTitle( feedName() );
     m_feed->setXmlUrl( url() );
     m_feed->setCustomFetchIntervalEnabled(autoFetch());
     if (autoFetch())
        m_feed->setFetchInterval(fetchInterval());
     m_feed->setArchiveMode(archiveMode());
     m_feed->setMaxArticleAge(maxArticleAge());
     m_feed->setMaxArticleNumber(maxArticleNumber());
     m_feed->setMarkImmediatelyAsRead(markImmediatelyAsRead());
     m_feed->setUseNotification(useNotification());
     m_feed->setLoadLinkedWebsite(loadLinkedWebsite());
     m_feed->setNotificationMode(true);

     KDialog::accept();
}

void FeedPropertiesDialog::slotSetWindowTitle(const QString& title)
{
    setWindowTitle(title.isEmpty() ? i18n("Feed Properties")
                                   : i18n("Properties of %1", title));
}

void FeedPropertiesDialog::setFeed(Feed* feed)
{
    m_feed = feed;
    if (!feed)
        return;

    setFeedName( feed->title() );
    setUrl( feed->xmlUrl() );
    setAutoFetch(feed->useCustomFetchInterval());
    if (feed->useCustomFetchInterval())
        setFetchInterval(feed->fetchInterval());
    else
        setFetchInterval(Settings::autoFetchInterval());
    setArchiveMode(feed->archiveMode());
    setMaxArticleAge(feed->maxArticleAge());
    setMaxArticleNumber(feed->maxArticleNumber());
    setMarkImmediatelyAsRead(feed->markImmediatelyAsRead());
    setUseNotification(feed->useNotification());
    setLoadLinkedWebsite(feed->loadLinkedWebsite());
    slotSetWindowTitle(feedName());
}


QString FeedPropertiesDialog::feedName() const
{
   return widget->feedNameEdit->text();
}

QString FeedPropertiesDialog::url() const
{
   return widget->urlEdit->text();
}

bool FeedPropertiesDialog::autoFetch() const
{
   return widget->cb_updateInterval->isChecked();
}

int FeedPropertiesDialog::fetchInterval() const
{
    switch (widget->updateComboBox->currentIndex() )
    {
        case FeedPropertiesWidget::Minutes:
            return widget->updateSpinBox->value();
        case FeedPropertiesWidget::Hours:
            return widget->updateSpinBox->value()*60;
        case FeedPropertiesWidget::Days:
            return widget->updateSpinBox->value()*60*24;
        case FeedPropertiesWidget::Never:
        default:
            return -1; // never
    }
}

Feed::ArchiveMode FeedPropertiesDialog::archiveMode() const
{
    // i could check the button group's int, but order could change...
    if ( widget->rb_globalDefault->isChecked() )
        return Feed::globalDefault;

   if ( widget->rb_keepAllArticles->isChecked() )
        return Feed::keepAllArticles;

   if ( widget->rb_limitArticleAge->isChecked() )
        return Feed::limitArticleAge;

   if ( widget->rb_limitArticleNumber->isChecked() )
        return Feed::limitArticleNumber;

   if ( widget->rb_disableArchiving->isChecked() )
        return Feed::disableArchiving;

    // in a perfect world, this is never reached

    return Feed::globalDefault;
}


int FeedPropertiesDialog::maxArticleAge() const
{
    return widget->sb_maxArticleAge->value();
}

int FeedPropertiesDialog::maxArticleNumber() const
{
    return widget->sb_maxArticleNumber->value();
}

void FeedPropertiesDialog::setArchiveMode(Feed::ArchiveMode mode)
 {
    switch (mode)
    {
         case Feed::globalDefault:
            widget->rb_globalDefault->setChecked(true);
            break;
         case Feed::keepAllArticles:
            widget->rb_keepAllArticles->setChecked(true);
            break;
         case Feed::disableArchiving:
            widget->rb_disableArchiving->setChecked(true);
            break;
         case Feed::limitArticleAge:
            widget->rb_limitArticleAge->setChecked(true);
            break;
         case Feed::limitArticleNumber:
            widget->rb_limitArticleNumber->setChecked(true);
    }
}
void FeedPropertiesDialog::setFeedName(const QString& title)
{
   widget->feedNameEdit->setText(title);
}

void FeedPropertiesDialog::setUrl(const QString& url)
{
   widget->urlEdit->setText(url);
}

void FeedPropertiesDialog::setAutoFetch(bool customFetchEnabled)
{
    widget->cb_updateInterval->setChecked(customFetchEnabled);
    widget->updateComboBox->setEnabled(customFetchEnabled);

    if (widget->updateSpinBox->value() > -1)
        widget->updateSpinBox->setEnabled(customFetchEnabled);
    else
        widget->updateSpinBox->setEnabled(false);
}

void FeedPropertiesDialog::setFetchInterval(int interval)
{
    if (interval == -1) // never update
    {
        widget->updateSpinBox->setValue(0);
        widget->updateSpinBox->setDisabled(true);
        widget->updateComboBox->setCurrentIndex( FeedPropertiesWidget::Never );
        return;
    }

    if (interval == 0)
    {
        widget->updateSpinBox->setValue(0);
        widget->updateSpinBox->setEnabled(widget->cb_updateInterval->isChecked());
        widget->updateComboBox->setCurrentIndex( FeedPropertiesWidget::Minutes );
        return;
    }

   if (interval % (60*24) == 0)
   {
       widget->updateSpinBox->setValue(interval / (60*24) );
       widget->updateSpinBox->setEnabled(widget->cb_updateInterval->isChecked());
       widget->updateComboBox->setCurrentIndex( FeedPropertiesWidget::Days );
       return;
   }

   if (interval % 60 == 0)
   {
       widget->updateSpinBox->setValue(interval / 60 );
       widget->updateSpinBox->setEnabled(widget->cb_updateInterval->isChecked());
       widget->updateComboBox->setCurrentIndex( FeedPropertiesWidget::Hours );
       return;
   }

   widget->updateSpinBox->setValue(interval);
   widget->updateSpinBox->setEnabled(widget->cb_updateInterval->isChecked());
   widget->updateComboBox->setCurrentIndex( FeedPropertiesWidget::Minutes );
}

void FeedPropertiesDialog::setMaxArticleAge(int age)
 {
    widget->sb_maxArticleAge->setValue(age);
}

void FeedPropertiesDialog::setMaxArticleNumber(int number)
{
    widget->sb_maxArticleNumber->setValue(number);
}

void FeedPropertiesDialog::setMarkImmediatelyAsRead(bool enabled)
{
    widget->checkBox_markRead->setChecked(enabled);
}

bool FeedPropertiesDialog::markImmediatelyAsRead() const
{
    return widget->checkBox_markRead->isChecked();
}

void FeedPropertiesDialog::setUseNotification(bool enabled)
{
    widget->checkBox_useNotification->setChecked(enabled);
}

bool FeedPropertiesDialog::useNotification() const
{
    return widget->checkBox_useNotification->isChecked();
}

bool FeedPropertiesDialog::loadLinkedWebsite() const
{
    return widget->checkBox_loadWebsite->isChecked();
}

void FeedPropertiesDialog::setLoadLinkedWebsite(bool enabled)
{
    widget->checkBox_loadWebsite->setChecked(enabled);
}

void FeedPropertiesDialog::selectFeedName()
{
   widget->feedNameEdit->selectAll();
}

} // namespace Akregator

