/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "app.h"
#include "akregator.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include <kdebug.h>

static const char description[] =
    I18N_NOOP("A KDE Feed Aggregator");

static const char version[] = "1.0-beta6 \"Bezerkus\"";

static KCmdLineOptions options[] =
{
    { "a", 0, 0 },
    { "addfeed <url>", I18N_NOOP( "Add a feed with the given URL." ), 0},
    { "g", 0, 0 },
    { "group <groupname>", I18N_NOOP( "When adding feeds, place them in this group." ), "Imported" },
    { "+[URL]", I18N_NOOP( "Document (feed list) to open." ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("akregator", I18N_NOOP("aKregator"), version, description,
                     KAboutData::License_GPL, "(C) 2004 akregator developers", 0,
                     "http://akregator.sourceforge.net/");
    about.addAuthor( "Stanislav Karchebny", I18N_NOOP("Maintainer, Lead Developer"), "Stanislav.Karchebny@kdemail.net" );
    about.addCredit( "Pierre Habouzit", I18N_NOOP("Release Manager"), "pierre.habouzit@m4x.org" );
    about.addAuthor( "Sashmit Bhaduri", I18N_NOOP("Lead Developer"), "sashmit@vfemail.net" );
    about.addAuthor( "Teemu Rytilahti", I18N_NOOP("Developer"), "teemu.rytilahti@kde-fi.org" );
    about.addAuthor( "Frank Osterfeld", I18N_NOOP("Developer"), "f_osterf@informatik.uni-kl.de" );
    about.addAuthor( "Gary Cramblitt", I18N_NOOP("Contributor"), "garycramblitt@comcast.net");
    about.addAuthor( "Stephan Binner", I18N_NOOP("Contributor"), "binner@kde.org" );
    about.addAuthor( "Christof Musik", I18N_NOOP("Contributor"), "christof@freenet.de" );
    about.addCredit( "Philipp Droessler", I18N_NOOP("Gentoo Ebuild"), "kingmob@albert-unser.net");
    about.addCredit( "Frerich Raabe", I18N_NOOP("Author of librss"), "raabe@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );
    aKregatorApp app;

    // see if we are starting with session management
    if (app.isRestored())
    {
        RESTORE(aKregator);
    }
    else
    {
        aKregator *widget = 0;
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if ( args->count() == 0 )
        {
            widget = new aKregator;
            if(!widget->loadPart()) return 1;
            widget->loadStandardFile();
            app.setHaveWindowLoaded(true);
            widget->show();
        }
        else
        {
            int i = 0;
            for (; i < args->count(); i++ )
            {
                widget = new aKregator;
                if(!widget->loadPart()) return 1;
                app.setHaveWindowLoaded(true);
                widget->show();
                widget->load( args->url( i ) );
            }
        }
	
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
