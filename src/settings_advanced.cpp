#include "settings_advanced.h"
#include "storagefactory.h"
#include "storagefactoryregistry.h"
       
#include <qwidget.h>

namespace Akregator {

SettingsAdvanced::SettingsAdvanced(QWidget* parent, const char* name) : SettingsAdvancedBase(parent, name)
{
   
}

void SettingsAdvanced::slotConfigureStorage()
{
}

} //namespace Akregator
#include "settings_advanced.moc"
