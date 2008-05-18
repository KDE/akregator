#ifndef CONFIGURATIONWIDGET_H_
#define CONFIGURATIONWIDGET_H_

#include "ui_configurationdialog.h"

#include <QWidget>

namespace feedsync
{

class ConfigurationWidget : public QWidget
{
    Q_OBJECT
    public:
        // Constructor if the dialog must be created
        explicit ConfigurationWidget( QWidget *parent=0 );
        ~ConfigurationWidget();

        void load();
        void save();

    private:
        Ui::ConfigurationWidget ui;

    private Q_SLOTS:
        void slotButtonAddClicked();
        void slotButtonUpdateClicked();
        void slotButtonRemoveClicked();
        void refresh();
};
 
}

#endif /*CONFIGURATIONWIDGET_H_*/
