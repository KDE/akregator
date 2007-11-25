/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#include "akregatorconfig.h"
#include "configdialog.h"
#include "settings_advanced.h"
#include "ui_settings_appearance.h"
#include "ui_settings_archive.h"
#include "ui_settings_browser.h"
#include "ui_settings_general.h"

#include <kcombobox.h>
#include <klocale.h>

#include <QLabel>
#include <QSlider>
#include <QStringList>

namespace Akregator
{
class ConfigDialog::ConfigDialogPrivate 
{
    public:
    KConfigSkeleton* config;
    SettingsAdvanced* settingsAdvanced;
    Ui::SettingsAppearance settingsAppearance;
};

ConfigDialog::ConfigDialog(QWidget* parent, const char* name, KConfigSkeleton* config, FaceType dialogType, QFlags<ButtonCode> dialogButtons, ButtonCode defaultButton, bool modal) : KConfigDialog(parent, name, config), d(new ConfigDialogPrivate)
{
    setModal(modal);
    setFaceType(dialogType);
    setButtons(dialogButtons);
    setDefaultButton(defaultButton);
    QWidget* generalWidget = new QWidget(this);
    Ui::SettingsGeneral general;
    general.setupUi(generalWidget);
    connect( general.kcfg_UseIntervalFetch, SIGNAL( toggled( bool ) ),
             general.kcfg_AutoFetchInterval, SLOT( setEnabled( bool ) ) );

    QWidget* archiveWidget = new QWidget(this);
    Ui::SettingsArchive archive;
    archive.setupUi(archiveWidget);

    connect( archive.rb_LimitArticleNumber, SIGNAL( toggled( bool ) ),
             archive.kcfg_MaxArticleNumber, SLOT( setEnabled( bool ) ) );

    connect( archive.rb_LimitArticleAge, SIGNAL( toggled( bool ) ),
             archive.kcfg_MaxArticleAge, SLOT( setEnabled( bool ) ) );

    QWidget* appearanceWidget = new QWidget(this);
    d->settingsAppearance.setupUi(appearanceWidget);

    connect( d->settingsAppearance.kcfg_UseCustomColors, SIGNAL( toggled( bool ) ),
             d->settingsAppearance.kcfg_ColorUnreadArticles, SLOT( setEnabled( bool ) ) );
    connect( d->settingsAppearance.kcfg_UseCustomColors, SIGNAL( toggled( bool ) ),
             d->settingsAppearance.kcfg_ColorNewArticles, SLOT( setEnabled( bool ) ) );
    connect( d->settingsAppearance.kcfg_UseCustomColors, SIGNAL( toggled( bool ) ),
             d->settingsAppearance.lbl_newArticles, SLOT( setEnabled( bool ) ) );
    connect( d->settingsAppearance.kcfg_UseCustomColors, SIGNAL( toggled( bool ) ),
             d->settingsAppearance.lbl_unreadArticles, SLOT( setEnabled( bool ) ) );
    connect( d->settingsAppearance.slider_minimumFontSize, SIGNAL( sliderMoved( int ) ),
             d->settingsAppearance.kcfg_MinimumFontSize , SLOT( setValue( int ) ) );
    connect( d->settingsAppearance.slider_mediumFontSize, SIGNAL( sliderMoved( int ) ),
             d->settingsAppearance.kcfg_MediumFontSize , SLOT( setValue( int ) ) );
    connect( d->settingsAppearance.kcfg_MinimumFontSize, SIGNAL( valueChanged( int ) ),
             d->settingsAppearance.slider_minimumFontSize, SLOT( setValue( int ) ) );
    connect( d->settingsAppearance.kcfg_MediumFontSize, SIGNAL( valueChanged( int ) ),
             d->settingsAppearance.slider_mediumFontSize, SLOT( setValue( int ) ) );

    QWidget* browserWidget = new QWidget(this);
    Ui::SettingsBrowser browser;
    browser.setupUi(browserWidget);
    connect(browser.kcfg_ExternalBrowserUseCustomCommand, SIGNAL(toggled(bool)), browser.kcfg_ExternalBrowserCustomCommand, SLOT(setEnabled(bool)));
    d->settingsAdvanced = new SettingsAdvanced(this, "Advanced");

    addPage(generalWidget, i18n("General"), "akregator");
    addPage(archiveWidget, i18n("Archive"), "utilities-file-archiver");
    addPage(appearanceWidget, i18n("Appearance"), "video-display");
    addPage(browserWidget, i18n("Browser"), "konqueror");
    addPage(d->settingsAdvanced, i18n("Advanced"), "preferences-other");
    d->settingsAdvanced->selectFactory(Settings::archiveBackend());
    d->config = config;
}

void ConfigDialog::updateSettings()
{
    Settings::setArchiveBackend(d->settingsAdvanced->selectedFactory());
    KConfigDialog::updateSettings();
}
        
void ConfigDialog::updateWidgets()
{
    d->settingsAdvanced->selectFactory(Settings::archiveBackend());
    d->settingsAppearance.slider_minimumFontSize->setDisabled(d->config->isImmutable("MinimumFontSize"));
    d->settingsAppearance.slider_mediumFontSize->setDisabled(d->config->isImmutable("MediumFontSize"));
    d->settingsAppearance.lbl_MinimumFontSize->setDisabled(d->config->isImmutable("MinimumFontSize"));
    d->settingsAppearance.lbl_MediumFontSize->setDisabled(d->config->isImmutable("MediumFontSize"));
    KConfigDialog::updateWidgets();
}
        
ConfigDialog::~ConfigDialog() 
{
    delete d;
    d = 0;
}

} // namespace Akregator

#include "configdialog.moc"
