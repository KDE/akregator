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

static const char description[] =
    I18N_NOOP("A KDE RSS Aggregator");

static const char version[] = "1.0-beta1 \"Sashmit\"";

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("akregator", I18N_NOOP("aKregator"), version, description,
                     KAboutData::License_GPL, "(C) 2004 Stanislav Karchebny", 0,
                     "http://akregator.upnet.ru/");
    about.addAuthor( "Stanislav Karchebny", I18N_NOOP("Author, Developer, Maintainer"), "Stanislav.Karchebny@kdemail.net" );
    about.addAuthor( "Sashmit Bhaduri", I18N_NOOP("Developer"), "smt@vfemail.net" );
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
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if ( args->count() == 0 )
        {
            aKregator *widget = new aKregator;
            widget->loadLastOpenFile();
            app.setHaveWindowLoaded(true);
            widget->show();
        }
        else
        {
            int i = 0;
            for (; i < args->count(); i++ )
            {
                aKregator *widget = new aKregator;
                app.setHaveWindowLoaded(true);
                widget->show();
                widget->load( args->url( i ) );
            }
        }
        args->clear();
    }

    return app.exec();
}
