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
                 i18n("Copyright © 2004–%1 Akregator authors", QStringLiteral("2026")),
                 QString(),
                 QStringLiteral("https://userbase.kde.org/Akregator"))
{
    addAuthor(i18nc("@info:credit", "Laurent Montel"), i18n("Maintainer"), QStringLiteral("montel@kde.org"));
    addAuthor(i18nc("@info:credit", "Frank Osterfeld"), i18n("Former maintainer"), QStringLiteral("osterfeld@kde.org"));
    addAuthor(i18nc("@info:credit", "Teemu Rytilahti"), i18n("Developer"), QStringLiteral("tpr@d5k.net"));
    addAuthor(i18nc("@info:credit", "Sashmit Bhaduri"), i18n("Developer"), QStringLiteral("sashmit@vfemail.net"));
    addAuthor(i18nc("@info:credit", "Pierre Habouzit"), i18n("Developer"), QStringLiteral("pierre.habouzit@m4x.org"));
    addAuthor(i18nc("@info:credit", "Stanislav Karchebny"), i18n("Developer"), QStringLiteral("Stanislav.Karchebny@kdemail.net"));
    addAuthor(i18nc("@info:credit", "Gary Cramblitt"), i18n("Contributor"), QStringLiteral("garycramblitt@comcast.net"));
    addAuthor(i18nc("@info:credit", "Stephan Binner"), i18n("Contributor"), QStringLiteral("binner@kde.org"));
    addAuthor(i18nc("@info:credit", "Christof Musik"), i18n("Contributor"), QStringLiteral("christof@freenet.de"));
    addCredit(i18nc("@info:credit", "Anne-Marie Mahfouf"), i18n("Handbook"), QStringLiteral("annma@kde.org"));
    addCredit(i18nc("@info:credit", "Frerich Raabe"), i18n("Author of librss"), QStringLiteral("raabe@kde.org"));
    addCredit(i18nc("@info:credit", "Eckhart Woerner"), i18n("Bug tracker management, Usability improvements"), QStringLiteral("kde@ewsoftware.de"));
    addCredit(i18nc("@info:credit", "Heinrich Wendel"), i18n("Tons of bug fixes"), QStringLiteral("h_wendel@cojobo.net"));
    addCredit(i18nc("@info:credit", "Eike Hein"), i18n("'Delayed mark as read' feature"), QStringLiteral("sho@eikehein.com"));
    addCredit(i18nc("@info:credit", "Marcel Dierkes"), i18n("Icons"), QStringLiteral("marcel.dierkes@gmx.de"));
    addCredit(i18nc("@info:credit", "George Staikos"), i18n("Insomnia"), QStringLiteral("staikos@kde.org"));
    addCredit(i18nc("@info:credit", "Philipp Droessler"), i18n("Gentoo Ebuild"), QStringLiteral("kingmob@albert-unser.net"));
}

AboutData::~AboutData() = default;
