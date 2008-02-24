#include "configurationdialog.h"

ConfigurationDialog::ConfigurationDialog( QWidget *parent)
{
    // UI setup
    QWidget *widget = new QWidget( this );
    ui.setupUi(widget);
    setMainWidget( widget );
    setCaption( i18n("Online reader") );
    // setButtons( KDialog::Close );

    // Init
    ui.list_readerList->setColumnCount(2);
    QStringList deleteTags;
        deleteTags.append("Never");
        deleteTags.append("Always");
        deleteTags.append("Ask");
    ui.cb_deleteFeeds->addItems(deleteTags);

    // Test
    QStringList title;
        title.append("Type");
        title.append("Description");
    ui.list_readerList->setHeaderLabels(title);

    QList<QTreeWidgetItem *> items;
    QStringList line;
        line.append("Google Reader");
        line.append("user_login");
    items.append(new QTreeWidgetItem((QTreeWidget*)0, line));
    QStringList line2;
        line2.append("OPML File");
        line2.append("http://website.com/feeds.opml");
    items.append(new QTreeWidgetItem((QTreeWidget*)0, line2));
    ui.list_readerList->insertTopLevelItems(0, items);


}

ConfigurationDialog::~ConfigurationDialog()
{

}

void ConfigurationDialog::slotButtonClicked() 
{
/*    KMessageBox::information( this, 
                              i18n("You pressed the button!" ),
                              i18n( "Hooray!" ) );*/
}
