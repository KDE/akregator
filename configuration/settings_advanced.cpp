/*
    This file is part of Akregator.

    Copyright (C) 2005-2007 Frank Osterfeld <osterfeld@kde.org>

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

#include "settings_advanced.h"
#include "akregatorconfig.h"
#include "storagefactory.h"
#include "storagefactoryregistry.h"

#include <KLocalizedString>

#include <QPushButton>
#include <QStringList>
#include <QWidget>

#include <cassert>

using namespace Akregator;
SettingsAdvanced::SettingsAdvanced(QWidget *parent) : QWidget(parent)
{
    setupUi(this);

    const QStringList backends = Backend::StorageFactoryRegistry::self()->list();
    for (const QString &i : backends) {
        Backend::StorageFactory *const factory = Backend::StorageFactoryRegistry::self()->getFactory(i);
        if (!factory) {
            continue;
        }

        m_factories.insert(factory->key(), factory);
        cbBackend->addItem(factory->name(), factory->key());
    }

    connect(pbBackendConfigure, &QPushButton::clicked, this, &SettingsAdvanced::slotConfigureStorage);
    connect(cbBackend, qOverload<int>(&QComboBox::activated), this, &SettingsAdvanced::slotFactorySelected);
    connect(kcfg_UseMarkReadDelay, &QCheckBox::toggled, kcfg_MarkReadDelay, &KPluralHandlingSpinBox::setEnabled);

    kcfg_MarkReadDelay->setSuffix(ki18ncp("Mark selected article read after", " second", " seconds"));
}

QString SettingsAdvanced::selectedFactory() const
{
    return cbBackend->itemData(cbBackend->currentIndex()).toString();
}

void SettingsAdvanced::selectFactory(const QString &key)
{
    const int idx = cbBackend->findData(key);
    if (idx < 0) {
        return;
    }
    cbBackend->setCurrentIndex(idx);
    const Backend::StorageFactory *const factory = m_factories.value(key);
    Q_ASSERT(factory);
    pbBackendConfigure->setEnabled(factory->isConfigurable());
}

void SettingsAdvanced::slotConfigureStorage()
{
    const QString key = cbBackend->itemData(cbBackend->currentIndex()).toString();
    if (!key.isEmpty()) {
        Backend::StorageFactory *const factory = m_factories.value(key);
        Q_ASSERT(factory);
        factory->configure();
    }
}

void SettingsAdvanced::slotFactorySelected(int pos)
{
    const QString key = cbBackend->itemData(pos).toString();
    const Backend::StorageFactory *const factory = m_factories.value(key);
    Q_ASSERT(factory);
    pbBackendConfigure->setEnabled(factory->isConfigurable());
}
