/***************************************************************************
 *     Copyright (C) 2004 by Sashmit Bhaduri                               *
 *     smt@vfemail.net                                                     *
 *                                                                         *
 *     Licensed under GPL.                                                 *
 ***************************************************************************/

#include <kactioncollection.h>
#include <kdebug.h>
#include <kparts/browserextension.h>
#include <klocale.h>

#include <libkdepim/progressmanager.h>

#include "frame.h"

using namespace Akregator;

Frame::Frame(QObject * parent, KParts::ReadOnlyPart *p, QWidget *visWidget, const QString& tit, bool watchSignals)
   :QObject(parent, "aKregatorFrame")
{
    m_part=p;
    m_widget=visWidget;
    m_title=tit;
    m_state=Idle;
    m_progress=-1;
    m_progressItem=0;

    if (watchSignals) // e.g, articles tab has no part
    {
        connect(m_part, SIGNAL(setWindowCaption (const QString &)), this, SLOT(setCaption (const QString &)));
        connect(m_part, SIGNAL(setStatusBarText (const QString &)), this, SLOT(setStatusText (const QString &)));

        KParts::BrowserExtension *ext=KParts::BrowserExtension::childObject( p );
        if (ext)
            connect( ext, SIGNAL(loadingProgress(int)), this, SLOT(setProgress(int)) );

        connect(p, SIGNAL(started(KIO::Job*)), this, SLOT(setStarted()));
        connect(p, SIGNAL(completed()), this, SLOT(setCompleted()));
        connect(p, SIGNAL(canceled(const QString &)), this, SLOT(setCanceled(const QString&)));
        connect(p, SIGNAL(completed(bool)), this, SLOT(setCompleted()));

/*        KActionCollection *coll=p->actionCollection();
        if (coll)
        {
            connect( coll, SIGNAL( actionStatusText( const QString & ) ),
             this, SLOT( slotActionStatusText( const QString & ) ) );
            connect( coll, SIGNAL( clearStatusText() ),
             this, SLOT( slotClearStatusText() ) );
        }
*/
    }
}

Frame::~Frame()
{
    if(m_progressItem) {
        m_progressItem->setComplete();
    }
}

int Frame::state() const
{
    return m_state;
}

KParts::ReadOnlyPart *Frame::part() const
{
    return m_part;
}

QWidget *Frame::widget() const
{
    return m_widget;
}

void Frame::setTitle(const QString &s)
{
    m_title=s;
}

void Frame::setCaption(const QString &s)
{
    if(m_progressItem) m_progressItem->setLabel(s);
    m_caption=s;
    emit captionChanged(s);
}

void Frame::setStatusText(const QString &s)
{
    m_statusText=s;
    emit statusText(s);
}

void Frame::setProgress(int a)
{
    if(m_progressItem) {
        m_progressItem->setProgress((int)a);
    }
    m_progress=a;
    emit loadingProgress(a);
}

void Frame::setState(int a)
{
    m_state=a;
    
    switch (m_state)
    {
        case Frame::Started:
            emit started();
            break;
        case Frame::Canceled:
            emit canceled(QString::null);
            break;
        case Frame::Idle:
        case Frame::Completed:
        default:
            emit completed();
    }}



const QString Frame::title() const
{
    return m_title;
}

const QString Frame::caption() const
{
    return m_caption;
}

const QString Frame::statusText() const
{
    return m_statusText;
}

void Frame::setStarted()
{
    if(m_progressId.isNull() || m_progressId.isEmpty()) m_progressId = KPIM::ProgressManager::getUniqueID();
    m_progressItem = KPIM::ProgressManager::createProgressItem(m_progressId, title(), QString::null, false);
    m_progressItem->setStatus(i18n("Loading..."));
    //connect(m_progressItem, SIGNAL(progressItemCanceled(KPIM::ProgressItem*)), SLOT(slotAbortFetch()));
    m_state=Started;
    emit started();
}

void Frame::setCanceled(const QString &s)
{
    if(m_progressItem) {
        m_progressItem->setStatus(i18n("Loading canceled"));
        m_progressItem->setComplete();
        m_progressItem = 0;
    }
    m_state=Canceled;
    emit canceled(s);
}

void Frame::setCompleted()
{
    if(m_progressItem) {
        m_progressItem->setStatus(i18n("Loading completed"));
        m_progressItem->setComplete();
        m_progressItem = 0;
    }
    m_state=Completed;
    emit completed();
}

int Frame::progress() const
{
    return m_progress;
}


#include "frame.moc"
// vim: set et ts=4 sts=4 sw=4:
