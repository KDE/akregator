/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "akregatorconfig.h"
#include "balloon.h"
#include "trayicon.h"

#include <kapplication.h>
#include <kwin.h>
#include <kiconeffect.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <kpopupmenu.h>
#include <kiconloader.h>

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
    contextMenu()->insertItem(SmallIcon("bottom"),i18n("Fetch All Feeds"), this, SLOT(fetchAllFeeds()));
}
	
TrayIcon::~TrayIcon()
{
	delete m_balloon;
}


void TrayIcon::mousePressEvent(QMouseEvent *e) {
    if (e->button() == LeftButton) {
        emit showPart();
    }

    KSystemTray::mousePressEvent(e);
}


QPixmap TrayIcon::takeScreenshot() const
{
    QPoint g = mapToGlobal(pos());
    int desktopWidth  = kapp->desktop()->width();
    int desktopHeight = kapp->desktop()->height();
    int tw = width();
    int th = height();
    int w = desktopWidth / 4;
    int h = desktopHeight / 9;
    int x = g.x() + tw/2 - w/2; // Center the rectange in the systray icon
    int y = g.y() + th/2 - h/2;
    if (x < 0)
        x = 0; // Move the rectangle to stay in the desktop limits
    if (y < 0)
        y = 0;
    if (x + w > desktopWidth)
        x = desktopWidth - w;
    if (y + h > desktopHeight)
        y = desktopHeight - h;

        // Grab the desktop and draw a circle arround the icon:
    QPixmap shot = QPixmap::grabWindow(qt_xrootwin(), x, y, w, h);
    QPainter painter(&shot);
    const int MARGINS = 6;
    const int WIDTH   = 3;
    int ax = g.x() - x - MARGINS -1;
    int ay = g.y() - y - MARGINS -1;
    painter.setPen( QPen(Qt::red/*KApplication::palette().active().highlight()*/, WIDTH) );
    painter.drawArc(ax, ay, tw + 2*MARGINS, th + 2*MARGINS, 0, 16*360);
    painter.end();

    // Paint the border
    const int BORDER = 1;
    QPixmap finalShot(w + 2*BORDER, h + 2*BORDER);
    finalShot.fill(KApplication::palette().active().foreground());
    painter.begin(&finalShot);
    painter.drawPixmap(BORDER, BORDER, shot);
    painter.end();
    return shot; // not finalShot?? -fo
}

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


void TrayIcon::slotSetUnread(int unread)
{
    if (unread==m_unread)
        return;
    
    m_unread=unread;
    
    QToolTip::remove(this);
    QToolTip::add(this, i18n("aKregator - 1 unread article", "aKregator - %n unread articles", unread));
    
    if (unread == 0)
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

void TrayIcon::fetchAllFeeds()
{
    DCOPRef("akregator","AkregatorIface").send("fetchAllFeeds");
}

#include "trayicon.moc"
