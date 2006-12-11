/*
    This file is part of Akregator.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qstring.h>

#include "aboutdata.h"

namespace Akregator {

AboutData::AboutData()
    : KAboutData("akregator", I18N_NOOP("Akregator"), AKREGATOR_VERSION, I18N_NOOP("A KDE Feed Aggregator"),
                 License_GPL, I18N_NOOP("(C) 2004, 2005 Akregator developers"), 0,
                     "http://akregator.kde.org/")
{
    addAuthor( "Frank Osterfeld", I18N_NOOP("Maintainer"), "frank.osterfeld@kdemail.net" );
    addAuthor( "Teemu Rytilahti", I18N_NOOP("Developer"), "tpr@d5k.net" );
    addAuthor( "Sashmit Bhaduri", I18N_NOOP("Developer"), "sashmit@vfemail.net" );
    addAuthor( "Pierre Habouzit", I18N_NOOP("Developer"), "pierre.habouzit@m4x.org" );
    addAuthor( "Stanislav Karchebny", I18N_NOOP("Developer"), "Stanislav.Karchebny@kdemail.net" );
    addAuthor( "Gary Cramblitt", I18N_NOOP("Contributor"), "garycramblitt@comcast.net");
    addAuthor( "Stephan Binner", I18N_NOOP("Contributor"), "binner@kde.org" );
    addAuthor( "Christof Musik", I18N_NOOP("Contributor"), "christof@freenet.de" );
    addCredit( "Anne-Marie Mahfouf", I18N_NOOP("Handbook"), "annma@kde.org" );
    addCredit( "Frerich Raabe", I18N_NOOP("Author of librss"), "raabe@kde.org" );
    addCredit( "Eckhart Woerner", I18N_NOOP("Bug tracker management, Usability improvements"), "kde@ewsoftware.de");
    addCredit( "Heinrich Wendel", I18N_NOOP("Tons of bug fixes"), "h_wendel@cojobo.net");
    addCredit( "Eike Hein", I18N_NOOP("'Delayed mark as read' feature"), "sho@eikehein.com" );
    addCredit( "Marcel Dierkes", I18N_NOOP("Icons"), "marcel.dierkes@gmx.de");
    addCredit( "George Staikos", I18N_NOOP("Insomnia"), "staikos@kde.org" );
    addCredit( "Philipp Droessler", I18N_NOOP("Gentoo Ebuild"), "kingmob@albert-unser.net");
}

AboutData::~AboutData()
{
}

}
