/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "akregatorconfig.h"
#include "balloon.h"
#include "trayicon.h"

#include <kwin.h>
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
        : KSystemTray(parent, name), m_unread(0), m_balloon(0)
{
    m_defaultIcon=KSystemTray::loadIcon("akregator");
    QPixmap m_unreadIcon=KSystemTray::loadIcon("akregator_empty");
    m_lightIconImage=m_unreadIcon.convertToImage();
    KIconEffect::deSaturate(m_lightIconImage, 0.60);
    setPixmap(m_defaultIcon);
    QToolTip::add(this, i18n("aKregator"));
    if(Settings::useNotifications()) {
        m_balloon=new Balloon(i18n( "<qt><nobr><b>Updated Feeds:</b></nobr></qt>" ));
		connect(m_balloon, SIGNAL(signalButtonClicked()),
			this, SLOT(viewButtonClicked()));
        m_balloon->hide();
    }
}
	
TrayIcon::~TrayIcon()
{}


void TrayIcon::newArticle(const QString&feed, const QPixmap&p, const QString&art)
{
    if(!m_balloon) return;
    if (!m_balloon->isVisible())
    {
        m_balloon->setAnchor(mapToGlobal(pos()));
        m_balloon->setFixedWidth(m_balloon->width()-10);
        m_balloon->show();
        KWin::setOnAllDesktops(m_balloon->winId(), true);
    }
    
    m_balloon->addArticle(feed, p, art);
}


void TrayIcon::updateUnread(int unread)
{
    if (unread==m_unread)
        return;
    
    m_unread=unread;
	QToolTip::remove(this);
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

void TrayIcon::viewButtonClicked()
{
	QWidget *p=static_cast<QWidget*>(parent());
	KWin::forceActiveWindow(p->winId());
}

void TrayIcon::settingsChanged()
{
    if (!m_balloon && Settings::useNotifications()) {
        m_balloon=new Balloon(i18n( "<qt><nobr><b>Updated Feeds:</b></nobr></qt>" ));
        connect(m_balloon, SIGNAL(signalButtonClicked()), this, SLOT(viewButtonClicked()));

        m_balloon->hide();
    }
    if (m_balloon && !Settings::useNotifications()) {
        delete m_balloon;
        m_balloon = 0;    
    }
    
    if ( Settings::showTrayIcon() )
        show();
    else
        hide();
}

#include "trayicon.moc"
