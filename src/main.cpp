/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include <qstringlist.h>

#include <dcopref.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

#include "aboutdata.h"
#include "app.h"
#include "akregator.h"
#include "akregator_options.h"


int main(int argc, char **argv)
{
    Akregator::AboutData about;
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( Akregator::akregator_options );
    Akregator::Application app;

    // see if we are starting with session management
    if (app.isRestored())
    {
#undef RESTORE
#define RESTORE(type) { int n = 1;\
    while (KMainWindow::canBeRestored(n)){\
        (new type)->restore(n, false);\
            n++;}}

        RESTORE(Akregator::MainWindow);
    }
    else
    {
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        Akregator::MainWindow* mainwin = new Akregator::MainWindow();

        app.setMainWidget(mainwin);
        app.setHaveWindowLoaded(true);
        if (mainwin->loadPart() == false)
            return 1;
        mainwin->setupProgressWidgets();
        mainwin->show();  

    
        DCOPRef akr("akregator", "AkregatorIface");
        
        akr.send("openStandardFeedList");
     
        QString addFeedGroup = !args->getOption("group").isEmpty() ? args->getOption("group") : i18n("Imported Folder");
        
        QCStringList feeds = args->getOptionList("addfeed");
        QStringList feedsToAdd;
        for (QCStringList::ConstIterator it = feeds.begin(); it != feeds.end(); ++it)
            feedsToAdd.append(*it);
        
        if (!feedsToAdd.isEmpty())
            akr.send("addFeedsToGroup", feedsToAdd, addFeedGroup );

        args->clear();
    }

    return app.exec();
}
