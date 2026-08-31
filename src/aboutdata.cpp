/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Teemu Rytilahti <tpr@d5k.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "aboutdata.h"

#include "akregator-version.h"

#include <KLocalizedString>

using namespace Qt::Literals::StringLiterals;

using namespace Akregator;

AboutData::AboutData()
    : KAboutData(u"akregator"_s,
                 i18n("Akregator"),
                 QStringLiteral(AKREGATOR_VERSION),
                 i18n("A KDE Feed Reader"),
                 KAboutLicense::GPL,
                 i18n("Copyright © 2004–%1 Akregator authors", u"2026"_s),
                 QString(),
                 u"https://userbase.kde.org/Akregator"_s)
{
    addAuthor(i18nc("@info:credit", "Laurent Montel"), i18n("Maintainer"), u"montel@kde.org"_s);
    addAuthor(i18nc("@info:credit", "Frank Osterfeld"), i18n("Former maintainer"), u"osterfeld@kde.org"_s);
    addAuthor(i18nc("@info:credit", "Teemu Rytilahti"), i18n("Developer"), u"tpr@d5k.net"_s);
    addAuthor(i18nc("@info:credit", "Sashmit Bhaduri"), i18n("Developer"), u"sashmit@vfemail.net"_s);
    addAuthor(i18nc("@info:credit", "Pierre Habouzit"), i18n("Developer"), u"pierre.habouzit@m4x.org"_s);
    addAuthor(i18nc("@info:credit", "Stanislav Karchebny"), i18n("Developer"), u"Stanislav.Karchebny@kdemail.net"_s);
    addAuthor(i18nc("@info:credit", "Gary Cramblitt"), i18n("Contributor"), u"garycramblitt@comcast.net"_s);
    addAuthor(i18nc("@info:credit", "Stephan Binner"), i18n("Contributor"), u"binner@kde.org"_s);
    addAuthor(i18nc("@info:credit", "Christof Musik"), i18n("Contributor"), u"christof@freenet.de"_s);
    addCredit(i18nc("@info:credit", "Anne-Marie Mahfouf"), i18n("Handbook"), u"annma@kde.org"_s);
    addCredit(i18nc("@info:credit", "Frerich Raabe"), i18n("Author of librss"), u"raabe@kde.org"_s);
    addCredit(i18nc("@info:credit", "Eckhart Woerner"), i18n("Bug tracker management, Usability improvements"), u"kde@ewsoftware.de"_s);
    addCredit(i18nc("@info:credit", "Heinrich Wendel"), i18n("Tons of bug fixes"), u"h_wendel@cojobo.net"_s);
    addCredit(i18nc("@info:credit", "Eike Hein"), i18n("'Delayed mark as read' feature"), u"sho@eikehein.com"_s);
    addCredit(i18nc("@info:credit", "Marcel Dierkes"), i18n("Icons"), u"marcel.dierkes@gmx.de"_s);
    addCredit(i18nc("@info:credit", "George Staikos"), i18n("Insomnia"), u"staikos@kde.org"_s);
    addCredit(i18nc("@info:credit", "Philipp Droessler"), i18n("Gentoo Ebuild"), u"kingmob@albert-unser.net"_s);
}

AboutData::~AboutData() = default;
