#ifndef AKREGATOR_SETTINGS_ADVANCED_H
#define AKREGATOR_SETTINGS_ADVANCED_H

#include "settings_advancedbase.h"

#include <qmap.h>

class QString;
class QWidget;

namespace Akregator {

namespace Backend
{
    class StorageFactory;
}

class SettingsAdvanced : public SettingsAdvancedBase
{
    Q_OBJECT	

    public:
        SettingsAdvanced(QWidget* parent=0, const char* name=0);

        /** returns the key of the currently selected factory */
        QString selectedFactory() const;
        
        void selectFactory(const QString& key);
        
    public slots:
        
        void slotConfigureStorage();
        void slotFactorySelected(int);
        
    private:
        QMap<int,Backend::StorageFactory*> m_factories;
        QMap<QString, int> m_keyPos;
};

} // namespace Akregator

#endif //AKREGATOR_SETTINGS_ADVANCED_H
