#include "akregatorconfig.h"
#include "settings_advanced.h"
#include "storagefactory.h"
#include "storagefactoryregistry.h"

#include <QPushButton>
#include <qstringlist.h>
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
        m_keyPos[m_factories[i]->key()] = i;
        cbBackend->addItem(m_factories[i]->name());
        i++;
    }
    connect(pbBackendConfigure, SIGNAL(clicked()), this, SLOT(slotConfigureStorage()));
    connect(cbBackend, SIGNAL(activated(int)), this, SLOT(slotFactorySelected(int)));
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
