/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/


#include "trayicon.h"

#include <kiconeffect.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobalsettings.h>

#include <qbitmap.h>
#include <qpainter.h>
#include <qfont.h>
#include <qtooltip.h>

using namespace Akregator;

TrayIcon::TrayIcon(QWidget *parent, const char *name)
        : KSystemTray(parent, name), m_unread(0)
{
    m_defaultIcon=KSystemTray::loadIcon("akregator");
    m_lightIconImage=m_defaultIcon.convertToImage();
    KIconEffect::toGamma(m_lightIconImage, 0.95);
    KIconEffect::deSaturate(m_lightIconImage, 0.60);
    setPixmap(m_defaultIcon);
    QToolTip::add(this, i18n("aKregator"));
}

TrayIcon::~TrayIcon()
{}

void TrayIcon::updateUnread(int unread)
{
    if (unread==m_unread)
        return;
    
    m_unread=unread;

    if (unread == 0)
    {
        QToolTip::add(this, i18n("aKregator - No unread articles"));
        setPixmap(m_defaultIcon);
    }
    else
    {           
        QToolTip::add(this, i18n("aKregator - %1 unread").arg(unread));
        
        // from KMSystemTray
        int oldW = pixmap()->size().width();
        int oldH = pixmap()->size().height();

        QString uStr=QString::number( unread );
        QFont f=KGlobalSettings::generalFont();
        f.setBold(true);
        float pointSize=f.pointSizeFloat();
        QFontMetrics fm(f);
        int w=fm.width(uStr);
        if( w > (oldW) )
        {
            pointSize *= float(oldW) / float(w);
            f.setPointSizeFloat(pointSize);
        }

        QPixmap pix(oldW, oldH);
        pix.fill(Qt::white);
        QPainter p(&pix);
        p.setFont(f);
        p.setPen(Qt::blue);
        p.drawText(pix.rect(), Qt::AlignCenter, uStr);

        pix.setMask(pix.createHeuristicMask());
        QImage img=pix.convertToImage();

        // overlay
        QImage overlayImg=m_lightIconImage.copy();
        KIconEffect::overlay(overlayImg, img);

        QPixmap icon;
        icon.convertFromImage(overlayImg);
        setPixmap(icon);
    }
}

#include "trayicon.moc"
