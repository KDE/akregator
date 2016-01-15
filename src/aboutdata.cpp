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

#include "kdepim-version.h"

#include <KLocalizedString>

using namespace Akregator;

AboutData::AboutData()
    : KAboutData(QStringLiteral("akregator"),
                 i18n("Akregator"),
                 QStringLiteral(KDEPIM_VERSION),
                 i18n("A KDE Feed Reader"),
                 KAboutLicense::GPL,
                 i18n("Copyright © 2004–2016 Akregator authors"),
                 QString(),
                 QStringLiteral("https://userbase.kde.org/Akregator"))
{
    setOrganizationDomain("kde.org"); // needed to get org.kde.akregator used for DBUS
    addAuthor(i18n("Laurent Montel"), i18n("Maintainer"), QStringLiteral("montel@kde.org"));
    addAuthor(i18n("Frank Osterfeld"), i18n("Maintainer"), QStringLiteral("osterfeld@kde.org"));
    addAuthor(i18n("Teemu Rytilahti"), i18n("Developer"), QStringLiteral("tpr@d5k.net"));
    addAuthor(i18n("Sashmit Bhaduri"), i18n("Developer"), QStringLiteral("sashmit@vfemail.net"));
    addAuthor(i18n("Pierre Habouzit"), i18n("Developer"), QStringLiteral("pierre.habouzit@m4x.org"));
    addAuthor(i18n("Stanislav Karchebny"), i18n("Developer"), QStringLiteral("Stanislav.Karchebny@kdemail.net"));
    addAuthor(i18n("Gary Cramblitt"), i18n("Contributor"), QStringLiteral("garycramblitt@comcast.net"));
    addAuthor(i18n("Stephan Binner"), i18n("Contributor"), QStringLiteral("binner@kde.org"));
    addAuthor(i18n("Christof Musik"), i18n("Contributor"), QStringLiteral("christof@freenet.de"));
    addCredit(i18n("Anne-Marie Mahfouf"), i18n("Handbook"), QStringLiteral("annma@kde.org"));
    addCredit(i18n("Frerich Raabe"), i18n("Author of librss"), QStringLiteral("raabe@kde.org"));
    addCredit(i18n("Eckhart Woerner"), i18n("Bug tracker management, Usability improvements"), QStringLiteral("kde@ewsoftware.de"));
    addCredit(i18n("Heinrich Wendel"), i18n("Tons of bug fixes"), QStringLiteral("h_wendel@cojobo.net"));
    addCredit(i18n("Eike Hein"), i18n("'Delayed mark as read' feature"), QStringLiteral("sho@eikehein.com"));
    addCredit(i18n("Marcel Dierkes"), i18n("Icons"), QStringLiteral("marcel.dierkes@gmx.de"));
    addCredit(i18n("George Staikos"), i18n("Insomnia"), QStringLiteral("staikos@kde.org"));
    addCredit(i18n("Philipp Droessler"), i18n("Gentoo Ebuild"), QStringLiteral("kingmob@albert-unser.net"));
}

AboutData::~AboutData()
{
}
