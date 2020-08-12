/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005-2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_SETTINGS_ADVANCED_H
#define AKREGATOR_SETTINGS_ADVANCED_H

#include "ui_settings_advancedbase.h"

#include <QHash>

class QString;
class QWidget;

namespace Akregator {
namespace Backend {
class StorageFactory;
}

class SettingsAdvanced : public QWidget, public Ui::SettingsAdvancedBase
{
    Q_OBJECT

public:
    explicit SettingsAdvanced(QWidget *parent = nullptr);

    /** returns the key of the currently selected factory */
    QString selectedFactory() const;

    void selectFactory(const QString &key);

public Q_SLOTS:

    void slotConfigureStorage();
    void slotFactorySelected(int);

private:
    QHash<QString, Backend::StorageFactory *> m_factories;
};
} // namespace Akregator

#endif //AKREGATOR_SETTINGS_ADVANCED_H
