#ifndef AKREGATOR_SETTINGS_ADVANCED_H
#define AKREGATOR_SETTINGS_ADVANCED_H

#include "settings_advancedbase.h"

class QWidget;

namespace Akregator {

class SettingsAdvanced : public SettingsAdvancedBase
{
    Q_OBJECT	

    public:
        SettingsAdvanced(QWidget* parent=0, const char* name=0);

    public slots: 
        
        void slotConfigureStorage();
};

} // namespace Akregator

#endif //AKREGATOR_SETTINGS_ADVANCED_H
