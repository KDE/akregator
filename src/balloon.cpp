/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri (smt@vfemail.net                *
 *                                                                         *
 *	 based on kopeteballoon.cpp:                                           *
 *    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>        *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include <qpointarray.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qlayout.h>

#include <kdeversion.h>
#include <kstringhandler.h>
#include <kglobalsettings.h>

#include <kapplication.h>
#include <kdialog.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <qlistview.h>
#include <kactivelabel.h>
#include <qlabel.h>
#include <qheader.h>

#include "balloon.h"

using namespace Akregator;

Akregator::Balloon::Balloon(const QString &text)
: QWidget(0L, "Balloon", WStyle_StaysOnTop | WStyle_Customize |
	WStyle_NoBorder | WStyle_Tool  | WX11BypassWM)
{
	setCaption("");

	QVBoxLayout *BalloonLayout = new QVBoxLayout(this, 22,
		KDialog::spacingHint(), "BalloonLayout");

	// BEGIN Layout1
	QHBoxLayout *Layout1 = new QHBoxLayout(BalloonLayout,
		KDialog::spacingHint(), "Layout1");
	//QLabel *mCaption = new QLabel(text, this, "mCaption");
	KActiveLabel *mCaption = new KActiveLabel(text, this, "mCaption");
	mCaption->setPalette(QToolTip::palette());
	mCaption->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );

	Layout1->addWidget(mCaption);
	// END Layout1

	m_articleList=new QListView(this, "Feed");
	m_articleList->setHScrollBarMode(QListView::AlwaysOff);
	m_articleList->addColumn("Feed");
	//m_articleList->addColumn("Title");
	m_articleList->setFrameStyle(QListView::NoFrame);
	m_articleList->header()->hide();
	m_articleList->setAllColumnsShowFocus(true);
	m_articleList->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
	m_articleList->setSelectionMode((QListView::SelectionMode)QListView::NoSelection);
	BalloonLayout->addWidget(m_articleList);

	QFont f = m_articleList->font();
	if (f.pointSize() > 12)
		f.setPointSize(f.pointSize()-4);
        else 
            f.setPointSize(8);
	
	m_articleList->setFont(f);

	/*m_articleList->setColumnWidthMode(0, QListView::Manual);
	m_articleList->setColumnWidthMode(1, QListView::Manual);
	m_articleList->setColumnWidth(0,);
	m_articleList->setColumnWidth(1,m_articleList->width()-);
*/

	// BEGIN Layout2
	QHBoxLayout *Layout2 = new QHBoxLayout(BalloonLayout,
		KDialog::spacingHint(), "Layout2");
	QPushButton *mViewButton = new QPushButton(i18n("to view", "View"), this,
		"mViewButton");
	QPushButton *mIgnoreButton = new QPushButton(i18n("Close"), this,
		"mIgnoreButton");

	Layout2->addStretch();
	Layout2->addWidget(mViewButton);
	Layout2->addWidget(mIgnoreButton);
	Layout2->addStretch();
	// END Layout2

	setPalette(QToolTip::palette());
	setAutoMask(TRUE);

	connect(mViewButton, SIGNAL(clicked()),
		this, SIGNAL(signalButtonClicked()));
	connect(mViewButton, SIGNAL(clicked()),
		this, SLOT(hide()));
	connect(mIgnoreButton, SIGNAL(clicked()),
		this, SIGNAL(signalIgnoreButtonClicked()));
	connect(mIgnoreButton, SIGNAL(clicked()),
		this, SLOT(hide()));
	connect(mCaption, SIGNAL(linkClicked(const QString &)),
		this, SIGNAL(signalIgnoreButtonClicked()));
	connect(mCaption, SIGNAL(linkClicked(const QString &)),
		this, SLOT(hide()));
}

void Akregator::Balloon::setAnchor(const QPoint &anchor)
{
	mAnchor = anchor;
	updateMask();
}

void Akregator::Balloon::updateMask()
{
	QRegion mask(10, 10, width() - 20, height() - 20);

	QPoint corners[8] = {
		QPoint(width() - 50, 10),
		QPoint(10, 10),
		QPoint(10, height() - 50),
		QPoint(width() - 50, height() - 50),
		QPoint(width() - 10, 10),
		QPoint(10, 10),
		QPoint(10, height() - 10),
		QPoint(width() - 10, height() - 10)
	};

	for (int i = 0; i < 4; ++i)
	{
		QPointArray corner;
		corner.makeArc(corners[i].x(), corners[i].y(), 40, 40,
			i * 16 * 90, 16 * 90);
		corner.resize(corner.size() + 1);
		corner.setPoint(corner.size() - 1, corners[i + 4]);
		mask -= corner;
	}

	// get screen-geometry for screen our anchor is on
	// (geometry can differ from screen to screen!
	QRect deskRect = KGlobalSettings::desktopGeometry(mAnchor);

	bool bottom = (mAnchor.y() + height()) > ((deskRect.y() + deskRect.height()-48));
	bool right = (mAnchor.x() + width()) > ((deskRect.x() + deskRect.width()-48));

	QPointArray arrow(4);
	arrow.setPoint(0, QPoint(right ? width() : 0, bottom ? height() : 0));
	arrow.setPoint(1, QPoint(right ? width() - 10 : 10,
		bottom ? height() - 30 : 30));
	arrow.setPoint(2, QPoint(right ? width() - 30 : 30,
		bottom ? height() - 10 : 10));
	arrow.setPoint(3, arrow[0]);
	mask += arrow;
	setMask(mask);

	move( right ? mAnchor.x() - width() : ( mAnchor.x() < 0 ? 0 : mAnchor.x() ),
	      bottom ? mAnchor.y() - height() : ( mAnchor.y() < 0 ? 0 : mAnchor.y() )  );

}

void Akregator::Balloon::addArticle(const QString&feed, const QPixmap&p, const QString&art)
{
	QString title=KStringHandler::rPixelSqueeze(feed, QFontMetrics(m_articleList->font()), m_articleList->width()/4);
	title=title+art;
	QListViewItem *i=new QListViewItem(m_articleList, title);
	i->setPixmap(0,p);
}
	
#include "balloon.moc"
// vim: set noet ts=4 sts=4 sw=4:
