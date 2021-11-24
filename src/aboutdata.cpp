/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Teemu Rytilahti <tpr@d5k.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "aboutdata.h"

#include "akregator-version.h"

#include <KLocalizedString>

using namespace Akregator;

AboutData::AboutData()
    : KAboutData(QStringLiteral("akregator"),
                 i18n("Akregator"),
                 QStringLiteral(AKREGATOR_VERSION),
                 i18n("A KDE Feed Reader"),
                 KAboutLicense::GPL,
                 i18n("Copyright © 2004–2021 Akregator authors"),
                 QString(),
                 QStringLiteral("https://userbase.kde.org/Akregator"))
{
    setOrganizationDomain("kde.org"); // needed to get org.kde.akregator used for DBUS
    addAuthor(i18n("Laurent Montel"), i18n("Maintainer"), QStringLiteral("montel@kde.org"));
    addAuthor(i18n("Frank Osterfeld"), i18n("Former maintainer"), QStringLiteral("osterfeld@kde.org"));
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

AboutData::~AboutData() = default;
