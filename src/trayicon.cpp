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

#if 0
// yes, a number of things wrong with the following code
// disable for now

// returns a mask in which all pixels that have the same color
// as the top left pixel are set transparent
void makeMask(QPixmap p)
{
    QImage d=p.convertToImage();
    QImage m(p.width(), p.height(), 1, 2, QImage::LittleEndian);
    m.setColor( 0, 0xffffff );
    m.setColor( 1, 0 );
    m.fill( 0 );

    uchar matchVal=*(d.scanLine(0));
    uchar *pLine, *mLine;
    for(int y=0; y < d.height(); ++y)
    {
        pLine=d.scanLine(y);
        mLine=m.scanLine(y);
        for(int x=0; x < d.width(); ++x)
        {
            if (pLine[x] == matchVal )
            {
                mLine[x]=0xff;
            }
        }
    }
    QBitmap bit;
    bit.convertFromImage(m);
    p.setMask(bit);
}

#endif

void TrayIcon::updateUnread(int unread)
{
    if (unread==m_unread)
        return;

    QToolTip::add(this, i18n("aKregator - %1 unread").arg(unread));

    if (!unread)
    {
        setPixmap(m_defaultIcon);
    }
    else
    {
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

    m_unread=unread;
}

#include "trayicon.moc"
