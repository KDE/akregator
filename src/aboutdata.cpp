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

#include "aboutdata.h"

#include <klocale.h>




namespace Akregator {

AboutData::AboutData()
    : KAboutData("akregator", 0, ki18n("Akregator"), AKREGATOR_VERSION, ki18n("A KDE Feed Reader"),
                 License_GPL, ki18n("Copyright © 2004–2010 Akregator authors"), KLocalizedString(),
                     "http://akregator.kde.org/")
{
    setOrganizationDomain("kde.org"); // needed to get org.kde.akregator used for DBUS
    addAuthor( ki18n("Frank Osterfeld"), ki18n("Maintainer"), "osterfeld@kde.org" );
    addAuthor( ki18n("Teemu Rytilahti"), ki18n("Developer"), "tpr@d5k.net" );
    addAuthor( ki18n("Sashmit Bhaduri"), ki18n("Developer"), "sashmit@vfemail.net" );
    addAuthor( ki18n("Pierre Habouzit"), ki18n("Developer"), "pierre.habouzit@m4x.org" );
    addAuthor( ki18n("Stanislav Karchebny"), ki18n("Developer"), "Stanislav.Karchebny@kdemail.net" );
    addAuthor( ki18n("Gary Cramblitt"), ki18n("Contributor"), "garycramblitt@comcast.net");
    addAuthor( ki18n("Stephan Binner"), ki18n("Contributor"), "binner@kde.org" );
    addAuthor( ki18n("Christof Musik"), ki18n("Contributor"), "christof@freenet.de" );
    addCredit( ki18n("Anne-Marie Mahfouf"), ki18n("Handbook"), "annma@kde.org" );
    addCredit( ki18n("Frerich Raabe"), ki18n("Author of librss"), "raabe@kde.org" );
    addCredit( ki18n("Eckhart Woerner"), ki18n("Bug tracker management, Usability improvements"), "kde@ewsoftware.de");
    addCredit( ki18n("Heinrich Wendel"), ki18n("Tons of bug fixes"), "h_wendel@cojobo.net");
    addCredit( ki18n("Eike Hein"), ki18n("'Delayed mark as read' feature"), "sho@eikehein.com" );
    addCredit( ki18n("Marcel Dierkes"), ki18n("Icons"), "marcel.dierkes@gmx.de");
    addCredit( ki18n("George Staikos"), ki18n("Insomnia"), "staikos@kde.org" );
    addCredit( ki18n("Philipp Droessler"), ki18n("Gentoo Ebuild"), "kingmob@albert-unser.net");
}

AboutData::~AboutData()
{
}

}
