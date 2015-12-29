/*
    This file is part of Akregator.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

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

#include "articleviewerpart.h"
#include "akregatorconfig.h"
#include <KGuiItem>
#include <KMessageBox>
#include <KLocalizedString>

#include <QStandardPaths>
#include <kconfiggroup.h>

using namespace Akregator;

ArticleViewerPart::ArticleViewerPart(QWidget *parent)
    : KHTMLPart(parent),
      m_button(-1)
{
    setXMLFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("akregator/articleviewer.rc")), true);
}

int ArticleViewerPart::button() const
{
    return m_button;
}

bool ArticleViewerPart::closeUrl()
{
    Q_EMIT browserExtension()->loadingProgress(-1);
    Q_EMIT canceled(QString());
    return KHTMLPart::closeUrl();
}

bool ArticleViewerPart::urlSelected(const QString &url, int button, int state, const QString &_target,
                                    const KParts::OpenUrlArguments &args,
                                    const KParts::BrowserArguments &browserArgs)
{
    m_button = button;
    if (url == QLatin1String("config:/disable_introduction")) {
        KGuiItem yesButton(KStandardGuiItem::yes());
        yesButton.setText(i18n("Disable"));
        KGuiItem noButton(KStandardGuiItem::no());
        noButton.setText(i18n("Keep Enabled"));
        if (KMessageBox::questionYesNo(widget(), i18n("Are you sure you want to disable this introduction page?"), i18n("Disable Introduction Page"), yesButton, noButton) == KMessageBox::Yes) {
            KConfigGroup conf(Settings::self()->config(), "General");
            conf.writeEntry("Disable Introduction", "true");
            conf.sync();
            return true;
        }

        return false;
    } else {
        return KHTMLPart::urlSelected(url, button, state, _target, args, browserArgs);
    }
}
