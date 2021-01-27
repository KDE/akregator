/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005-2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
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
SettingsAdvanced::SettingsAdvanced(QWidget *parent)
    : QWidget(parent)
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
