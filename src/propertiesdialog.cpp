/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "feed.h"
#include "propertiesdialog.h"

#include <kcombobox.h>
#include <klineedit.h>
#include <kpassdlg.h>
#include <klocale.h>
#include <knuminput.h>

#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

using namespace Akregator;

FeedPropertiesWidget::FeedPropertiesWidget(QWidget *parent, const char *name)
        : FeedPropertiesWidgetBase(parent, name)
{
}

FeedPropertiesWidget::~FeedPropertiesWidget()
{}

void FeedPropertiesWidget::slotUpdateComboBoxActivated( int index )
{
    if ( index == 3 ) // "never"
        updateSpinBox->setEnabled(false);
    else
        updateSpinBox->setEnabled(true);
}


void FeedPropertiesWidget::slotUpdateCheckBoxToggled( bool enabled )
{
    if (enabled && updateComboBox->currentItem() != 3 ) // "never"
        updateSpinBox->setEnabled(true);
    else
        updateSpinBox->setEnabled(false);
}


FeedPropertiesDialog::FeedPropertiesDialog(QWidget *parent, const char *name)
        : KDialogBase(KDialogBase::Swallow, Qt::WStyle_DialogBorder, parent, name, true, i18n("Feed Properties"), KDialogBase::Ok|KDialogBase::Cancel)
{
    widget=new FeedPropertiesWidget(this);
    setMainWidget(widget);
    widget->feedNameEdit->setFocus();
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
    switch (widget->updateComboBox->currentItem() )
    {
        case 0: // minutes
            return widget->updateSpinBox->value();
        case 1: // hours
            return widget->updateSpinBox->value()*60;
        case 2: // days
            return widget->updateSpinBox->value()*60*24;
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

void FeedPropertiesDialog::setAutoFetch(bool w)
{
   widget->upChkbox->setChecked(w);
   widget->updateSpinBox->setEnabled(w);
}

void FeedPropertiesDialog::setFetchInterval(int i)
{
    if (i == -1)
    {
        widget->updateSpinBox->setValue(0);
        widget->updateComboBox->setCurrentItem(3); // never
        return;
    }

    if (i == 0)
    {
        widget->updateSpinBox->setValue(0);
        widget->updateComboBox->setCurrentItem(0); // minutes
        return;
    }
 
   if (i % (60*24) == 0)
   {
       widget->updateSpinBox->setValue(i / (60*24) );
       widget->updateComboBox->setCurrentItem(2); // days
       return;
   }
   
   if (i % 60 == 0)
   {
       widget->updateSpinBox->setValue(i / 60 );
       widget->updateComboBox->setCurrentItem(1); // hours
       return;
   }

   widget->updateSpinBox->setValue(i);
   widget->updateComboBox->setCurrentItem(0); // minutes
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
      
void FeedPropertiesDialog::selectFeedName()
{
   widget->feedNameEdit->selectAll();
}

#include "propertiesdialog.moc"
// vim: ts=4 sw=4 et
