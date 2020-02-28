/*
   Copyright (C) 2020 Laurent Montel <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCM_Akregator_Config_Plugins_H
#define KCM_Akregator_Config_Plugins_H

#include <KCModule>

#include <QVariant>
namespace KUserFeedback {
class FeedbackConfigWidget;
}

namespace KAddressBook {
class KCMAkregatorUserFeedBackConfig : public KCModule
{
    Q_OBJECT
public:
    KCMAkregatorUserFeedBackConfig(QWidget *parent, const QVariantList &args);
    void load() override;
    void save() override;
    void defaults() override;
private:
    KUserFeedback::FeedbackConfigWidget *mUserFeedbackWidget = nullptr;
};
}
#endif // KCM_Akregator_Config_Plugins_H
