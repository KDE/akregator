/***************************************************************************
 *   Copyright (C) 2004 by Teemu Rytilahti                                 *
 *   tpr@d5k.net                                                           *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include <qstring.h>

#include "aboutdata.h"

namespace Akregator {

const char version[] = "1.0-beta8";

AboutData::AboutData()
    : KAboutData("akregator", I18N_NOOP("aKregator"), "1.0-beta8", I18N_NOOP("A KDE Feed Aggregator"),
                 License_GPL, I18N_NOOP("(C) 2004 aKregator developers"), 0,
                     "http://akregator.sourceforge.net/")
{
    addAuthor( "Stanislav Karchebny", I18N_NOOP("Maintainer, Lead Developer"), "Stanislav.Karchebny@kdemail.net" );
    addCredit( "Pierre Habouzit", I18N_NOOP("Release Manager"), "pierre.habouzit@m4x.org" );
    addAuthor( "Sashmit Bhaduri", I18N_NOOP("Lead Developer"), "sashmit@vfemail.net" );
    addAuthor( "Teemu Rytilahti", I18N_NOOP("Developer"), "teemu.rytilahti@kde-fi.org" );
    addAuthor( "Frank Osterfeld", I18N_NOOP("Developer"), "frank.osterfeld@kdemail.net" );
    addAuthor( "Gary Cramblitt", I18N_NOOP("Contributor"), "garycramblitt@comcast.net");
    addAuthor( "Stephan Binner", I18N_NOOP("Contributor"), "binner@kde.org" );
    addAuthor( "Christof Musik", I18N_NOOP("Contributor"), "christof@freenet.de" );
    addCredit( "Philipp Droessler", I18N_NOOP("Gentoo Ebuild"), "kingmob@albert-unser.net");
    addCredit( "Frerich Raabe", I18N_NOOP("Author of librss"), "raabe@kde.org" );
}

AboutData::~AboutData()
{
}

QString AboutData::version()
{
    /*QString test;
    return test; //QString::fromLatin1(version);*/
}

}
