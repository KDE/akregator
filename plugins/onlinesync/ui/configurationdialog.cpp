#include "configurationdialog.h"
#include "configurationwidget.h"

#include <KDebug>

using namespace feedsync;

ConfigurationDialog::ConfigurationDialog( QWidget *parent) : KDialog( parent )
{
    kDebug();
    m_widget = new ConfigurationWidget( this );
    setMainWidget( m_widget );
    setWindowTitle( i18n("Online Reader Configuration") );
}

void ConfigurationDialog::accept() {
    kDebug();
    m_widget->save();
    KDialog::accept();
}


#include "configurationdialog.moc"
