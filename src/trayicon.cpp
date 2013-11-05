/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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

#include "trayicon.h"
#include "akregatorconfig.h"

#include <kapplication.h>
#include <kwindowsystem.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kmenu.h>
#include <kicon.h>
#include <kiconloader.h>
#include <KColorScheme>

#include <QPainter>
#include <QBitmap>
#include <QDesktopWidget>


namespace Akregator {

TrayIcon* TrayIcon::m_instance = 0;

TrayIcon* TrayIcon::getInstance()
{
    return m_instance;
}

void TrayIcon::setInstance(TrayIcon* trayIcon)
{
    m_instance = trayIcon;
}


TrayIcon::TrayIcon(QObject* parent)
        : KStatusNotifierItem(parent), m_unread(0)
{
    setToolTipTitle( i18n("Akregator") );
    setToolTipIconByName( i18n("Akregator") );
    setIconByName( QLatin1String("akregator") );
    m_defaultIcon = KIcon( QLatin1String("akregator") );
}


TrayIcon::~TrayIcon()
{}

void TrayIcon::slotSetUnread(int unread)
{
    m_unread = unread;

    this->setToolTip( m_defaultIcon.name(), i18n("Akregator"), unread == 0 ? i18n("There are no unread articles")  : i18np( "1 unread article", "%1 unread articles", unread ) );
    setStatus( unread > 0 ? KStatusNotifierItem::Active : KStatusNotifierItem::Passive );

    if (unread <= 0 || !Settings::enableTrayIconUnreadArticleCount())
    {
        setIconByName( m_defaultIcon.name() );
    }
    else
    {
        // adapted from KMSystemTray::updateCount()
        int oldWidth = KIconLoader::SizeSmallMedium;

        QString countStr = QString::number( unread );
        QFont f = KGlobalSettings::generalFont();
        f.setBold(true);

        float pointSize = f.pointSizeF();
        QFontMetrics fm(f);
        int w = fm.width(countStr);
        if( w > (oldWidth - 2) )
        {
            pointSize *= float(oldWidth - 2) / float(w);
            f.setPointSizeF(pointSize);
        }

        // overlay
        QPixmap overlayImg( oldWidth, oldWidth );
        overlayImg.fill( Qt::transparent );

        QPainter p(&overlayImg);
        p.setFont(f);
        KColorScheme scheme(QPalette::Active, KColorScheme::View);

        fm = QFontMetrics(f);
        QRect boundingRect = fm.tightBoundingRect(countStr);
        boundingRect.adjust(0, 0, 0, 2);
        boundingRect.setHeight(qMin(boundingRect.height(), oldWidth));
        boundingRect.moveTo((oldWidth - boundingRect.width()) / 2,
                            ((oldWidth - boundingRect.height()) / 2) - 1);
        p.setOpacity(0.7);
        p.setBrush(scheme.background(KColorScheme::LinkBackground));
        p.setPen(scheme.background(KColorScheme::LinkBackground).color());
        p.drawRoundedRect(boundingRect, 2.0, 2.0);

        p.setBrush(Qt::NoBrush);
        p.setPen(scheme.foreground(KColorScheme::LinkText).color());
        p.setOpacity(1.0);
        p.drawText(overlayImg.rect(), Qt::AlignCenter, countStr);
        p.end();

        QPixmap iconPixmap = m_defaultIcon.pixmap( oldWidth, oldWidth );

        QPainter pp( &iconPixmap );
        pp.drawPixmap( 0, 0, overlayImg );
        pp.end();

        setIconByPixmap( iconPixmap );
    }
}

void TrayIcon::viewButtonClicked()
{
    QWidget* p = static_cast<QWidget*>(parent());
    KWindowSystem::activateWindow(p->winId());
}

void TrayIcon::settingsChanged()
{
    slotSetUnread(m_unread);
}

} // namespace Akregator

