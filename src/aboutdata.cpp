/***************************************************************************
 *   Copyright (C) 2004 by Teemu Rytilahti                                 *
 *   tpr@d5k.net                                                           *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include <qstring.h>

#include "aboutdata.h"

namespace Akregator {

AboutData::AboutData()
    : KAboutData("akregator", I18N_NOOP("Akregator"), AKREGATOR_VERSION, I18N_NOOP("A KDE Feed Aggregator"),
                 License_GPL, I18N_NOOP("(C) 2004, 2005 Akregator developers"), 0,
                     "http://akregator.sourceforge.net/")
{
    addAuthor( "Frank Osterfeld", I18N_NOOP("Maintainer"), "frank.osterfeld@kdemail.net" );
    addAuthor( "Teemu Rytilahti", I18N_NOOP("Developer"), "teemu.rytilahti@kde-fi.org" );
    addAuthor( "Sashmit Bhaduri", I18N_NOOP("Developer"), "sashmit@vfemail.net" );
    addAuthor( "Pierre Habouzit", I18N_NOOP("Developer"), "pierre.habouzit@m4x.org" );
    addAuthor( "Stanislav Karchebny", I18N_NOOP("Developer"), "Stanislav.Karchebny@kdemail.net" );
    addAuthor( "Gary Cramblitt", I18N_NOOP("Contributor"), "garycramblitt@comcast.net");
    addAuthor( "Stephan Binner", I18N_NOOP("Contributor"), "binner@kde.org" );
    addAuthor( "Christof Musik", I18N_NOOP("Contributor"), "christof@freenet.de" );
    addCredit( "Philipp Droessler", I18N_NOOP("Gentoo Ebuild"), "kingmob@albert-unser.net");
    addCredit( "Frerich Raabe", I18N_NOOP("Author of librss"), "raabe@kde.org" );
    addCredit( "George Staikos", I18N_NOOP("Insomnia"), "staikos@kde.org" );
}

AboutData::~AboutData()
{
}

}
