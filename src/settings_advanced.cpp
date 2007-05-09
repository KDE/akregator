/*
    This file is part of Akregator.

    Copyright (C) 2005-2007 Frank Osterfeld <frank.osterfeld@kdemail.net>

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
#include "settings_advanced.h"
#include "storagefactory.h"
#include "storagefactoryregistry.h"

#include <QPushButton>
#include <QStringList>
#include <QWidget>

#include <kcombobox.h>

namespace Akregator {

SettingsAdvanced::SettingsAdvanced(QWidget* parent, const char* name) : QWidget(parent)
{
    setObjectName(name);
    setupUi(this);
    
    QStringList backends = Backend::StorageFactoryRegistry::self()->list();
    QString tname;
    int i = 0;
    QStringList::Iterator end( backends.end() );
    for (QStringList::Iterator it = backends.begin(); it != end; ++it)
    {
        m_factories[i] = Backend::StorageFactoryRegistry::self()->getFactory(*it);
	if(m_factories[i])
	{
          m_keyPos[m_factories[i]->key()] = i;
          cbBackend->addItem(m_factories[i]->name());
	}
        i++;
    }
    connect(pbBackendConfigure, SIGNAL(clicked()), this, SLOT(slotConfigureStorage()));
    connect(cbBackend, SIGNAL(activated(int)), this, SLOT(slotFactorySelected(int)));
    connect( kcfg_UseMarkReadDelay, SIGNAL( toggled( bool ) ),
             kcfg_MarkReadDelay, SLOT( setEnabled( bool ) ) );
}

QString SettingsAdvanced::selectedFactory() const
{
    return m_factories[cbBackend->currentIndex()]->key();
}

void SettingsAdvanced::selectFactory(const QString& key)
{
    cbBackend->setCurrentIndex(m_keyPos[key]);
    pbBackendConfigure->setEnabled((m_factories[m_keyPos[key]]->isConfigurable()));
}

void SettingsAdvanced::slotConfigureStorage()
{
    m_factories[cbBackend->currentIndex()]->configure();
}

void SettingsAdvanced::slotFactorySelected(int pos)
{
    pbBackendConfigure->setEnabled(m_factories[pos]->isConfigurable());
}

} //namespace Akregator
#include "settings_advanced.moc"
