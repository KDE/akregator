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

#ifndef AKREGATOR_CONFIGDIALOG_H
#define AKREGATOR_CONFIGDIALOG_H

#include <kconfigdialog.h>

namespace Akregator {

class SettingsAdvanced;
class SettingsAppearance;

class ConfigDialog : public KConfigDialog
{
    Q_OBJECT
    public:
    
        ConfigDialog(QWidget *parent, const char *name, KConfigSkeleton *config, DialogType dialogType=IconList, int dialogButtons=Default|Ok|Apply|Cancel|Help, ButtonCode defaultButton=Ok, bool modal=false);
        
        virtual ~ConfigDialog();

    protected slots:

        virtual void updateSettings();
        
        virtual void updateWidgets();

    private:
        KConfigSkeleton* m_config;
        SettingsAdvanced* m_settingsAdvanced;
        SettingsAppearance* m_settingsAppearance;
        
};

} // namespace Akregator
#endif // AKREGATOR_CONFIGDIALOG_H
