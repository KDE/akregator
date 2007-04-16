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

ConfigDialog::ConfigDialog(QWidget* parent, const char* name, KConfigSkeleton* config, FaceType dialogType, QFlags<ButtonCode> dialogButtons, ButtonCode defaultButton, bool modal) : KConfigDialog(parent, name, config, dialogType, dialogButtons, defaultButton, modal), d(new ConfigDialogPrivate)
{
    QWidget* generalWidget = new QWidget(this);
    Ui::SettingsGeneral general;
    general.setupUi(generalWidget);

    QWidget* archiveWidget = new QWidget(this);
    Ui::SettingsArchive archive;
    archive.setupUi(archiveWidget);

    QWidget* appearanceWidget = new QWidget(this);
    d->settingsAppearance.setupUi(appearanceWidget);
    
    QWidget* browserWidget = new QWidget(this);
    Ui::SettingsBrowser browser;
    browser.setupUi(browserWidget);
    connect(browser.kcfg_ExternalBrowserUseCustomCommand, SIGNAL(toggled(bool)), browser.kcfg_ExternalBrowserCustomCommand, SLOT(setEnabled(bool)));
    d->settingsAdvanced = new SettingsAdvanced(this, "Advanced");

    addPage(generalWidget, i18n("General"), "package_settings");
    addPage(archiveWidget, i18n("Archive"), "package_settings");
    addPage(appearanceWidget, i18n("Appearance"), "fonts");
    addPage(browserWidget, i18n("Browser"), "package-network");
    addPage(d->settingsAdvanced, i18n("Advanced"), "package-network");
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
