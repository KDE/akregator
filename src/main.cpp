/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "app.h"
#include "akregator.h"
#include "akregator_options.h"
#include "aboutdata.h"

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include <kdebug.h>

int main(int argc, char **argv)
{
    Akregator::AboutData about;
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( akregator_options );
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
        // no session.. just start up normally
        Akregator::MainWindow *widget = 0;
        
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        widget = new Akregator::MainWindow();

        if(!widget->loadPart())
            return 1;
        
        widget->loadStandardFile();
        app.setHaveWindowLoaded(true);
        widget->setupProgressWidgets();
        widget->show();

        app.setMainWidget(widget);

        QString addFeedGroup = args->getOption("group");
        QCStringList addFeeds = args->getOptionList("addfeed");
        QCStringList::iterator addFeedsIt;
        for (addFeedsIt = addFeeds.begin(); (addFeedsIt != addFeeds.end()) ; ++addFeedsIt )
        {
          kdDebug() << "--addfeed " << *addFeedsIt << "--group " << addFeedGroup << endl;
          widget->addFeedToGroup(*addFeedsIt, addFeedGroup);
        }
        args->clear();
    }

    return app.exec();
}
