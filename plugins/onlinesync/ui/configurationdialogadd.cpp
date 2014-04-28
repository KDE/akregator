/*
    This file is part of Akregator.

    Copyright (C) 2008 Didier Hoarau <did.hoarau@gmail.com>

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

#include "configurationdialogadd.h"

#include <qdebug.h>


using namespace feedsync;

ConfigurationDialogAdd::ConfigurationDialogAdd( QWidget *parent) : KDialog(parent)
{
    qDebug();

    // UI setup
    QWidget *widget = new QWidget( parent );
    ui.setupUi(widget);
    setMainWidget( widget );

    setWindowTitle( i18n("Modify Online Reader Account") );
    ui.cb_AggregatorType->addItem( i18n("Google Reader") , QVariant("GoogleReader") );
    /* TODO OPML not available for the first version
    ui.cb_AggregatorType->addItem( i18n("OPML file") , QVariant("Opml") ); */


    slotUpdateUI();

    connect( ui.cb_AggregatorType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUpdateUI()) );
}

ConfigurationDialogAdd::~ConfigurationDialogAdd()
{
    qDebug();
}

void ConfigurationDialogAdd::accept()
{
    qDebug();

    if (ui.cb_AggregatorType->itemData( ui.cb_AggregatorType->currentIndex() ) == "GoogleReader") {

        if (ui.le_loginGoogleReader->text().isEmpty())
            return;
        if (ui.le_passwdGoogleReader->text().isEmpty())
            return;
        // Remove old
        if (!_baseconfigname.isEmpty()) {
            KConfig config("akregator_feedsyncrc");
            config.deleteGroup(_baseconfigname);
        }
        // Insert new
        KConfig config("akregator_feedsyncrc");
        KConfigGroup generalGroup( &config, "FeedSyncSource_GoogleReader" + ui.le_loginGoogleReader->text() );
        generalGroup.writeEntry( "AggregatorType", ui.cb_AggregatorType->itemData( ui.cb_AggregatorType->currentIndex() ) );
        generalGroup.writeEntry( "Login", ui.le_loginGoogleReader->text() );
        generalGroup.writeEntry( "Password", ui.le_passwdGoogleReader->text() );
        generalGroup.writeEntry( "Identifier", ui.le_loginGoogleReader->text() );
        generalGroup.config()->sync();
    } else if (ui.cb_AggregatorType->itemData( ui.cb_AggregatorType->currentIndex() ) == "Opml") {

        if (ui.filerequester->url().isEmpty() )
            return;
        // Remove old
        if (!_baseconfigname.isEmpty()) {
            KConfig config("akregator_feedsyncrc");
            config.deleteGroup(_baseconfigname);
        }
        // Insert new
        KConfig config("akregator_feedsyncrc");
        const KUrl url = ui.filerequester->url();
        KConfigGroup generalGroup( &config, "FeedSyncSource_Opml" + url.url() );
        generalGroup.writeEntry( "AggregatorType", ui.cb_AggregatorType->itemData( ui.cb_AggregatorType->currentIndex() ) );
        generalGroup.writeEntry( "Filename", url.url() );
        generalGroup.writeEntry( "Identifier", url.url() );
        generalGroup.config()->sync();
    }
    done( KDialog::Ok );
}

void ConfigurationDialogAdd::load( const KConfigGroup& group )
{
    qDebug();
    _baseconfigname = group.name();

    ui.cb_AggregatorType->setCurrentIndex( ui.cb_AggregatorType->findData( group.readEntry( "AggregatorType", QString() ) ) );

    if ( group.readEntry( "AggregatorType", QString() ) == "GoogleReader") {

        ui.le_loginGoogleReader->setText( group.readEntry( "Login", QString() ) );
        ui.le_passwdGoogleReader->setText( group.readEntry( "Password", QString() ) );

    } else if ( group.readEntry( "AggregatorType", QString() ) == "Opml") {

        ui.filerequester->setUrl( group.readEntry( "Filename", QString() ) );

    } else {

    }
}

// SLOT

void ConfigurationDialogAdd::slotUpdateUI()
{
    qDebug();

    if (ui.cb_AggregatorType->itemData( ui.cb_AggregatorType->currentIndex() ) == "GoogleReader") {
        ui.groupOpml->hide();
        ui.groupGoogleReader->show();

    } else if (ui.cb_AggregatorType->itemData( ui.cb_AggregatorType->currentIndex() ) == "Opml") {
        ui.groupGoogleReader->hide();
        ui.groupOpml->show();

    } else {
    }
}

