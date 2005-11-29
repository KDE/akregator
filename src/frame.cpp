/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

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

#include <QGridLayout>
#include <qregexp.h>

#include <kactioncollection.h>
#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>


#include <libkdepim/progressmanager.h>

#include "frame.h"

namespace Akregator {


void Frame::slotSetTitle(const QString &s)
{
    if (m_title != s)
    {
        m_title = s;
         emit signalTitleChanged(this, s);
    }
}

void Frame::slotSetCaption(const QString &s)
{
    if(m_progressItem) m_progressItem->setLabel(s);
    m_caption=s;
    emit signalCaptionChanged(this, s);
}

void Frame::slotSetStatusText(const QString &s)
{
    m_statusText=s;
    m_statusText.replace(QRegExp("<[^>]*>"), "");
    emit signalStatusText(this, m_statusText);
}

void Frame::slotSetProgress(int a)
{
    if(m_progressItem) {
        m_progressItem->setProgress((int)a);
    }
    m_progress=a;
    emit signalLoadingProgress(this, a);
}

void Frame::slotSetState(int a)
{
    m_state=a;
    
    switch (m_state)
    {
        case Frame::Started:
            emit signalStarted(this);
            break;
        case Frame::Canceled:
            emit signalCanceled(this, QString::null);
            break;
        case Frame::Idle:
        case Frame::Completed:
        default:
            emit signalCompleted(this);
    }
}

Frame::Frame(QWidget* parent, KParts::ReadOnlyPart *p, QWidget *visWidget, const QString& tit, bool watchSignals)
   : QWidget(parent)
{
    m_part=p;
    m_widget=visWidget;
    m_title=tit;
    m_state=Idle;
    m_progress=-1;
    m_progressItem=0;
    m_isRemovable = true;

    if (watchSignals) // e.g, articles tab has no part
    {
        connect(m_part, SIGNAL(setWindowCaption (const QString &)), this, SLOT(slotSetCaption (const QString &)));
        connect(m_part, SIGNAL(setStatusBarText (const QString &)), this, SLOT(slotSetStatusText (const QString &)));

        KParts::BrowserExtension *ext=KParts::BrowserExtension::childObject( p );
        if (ext)
            connect( ext, SIGNAL(loadingProgress(int)), this, SLOT(slotSetProgress(int)) );

        connect(p, SIGNAL(started(KIO::Job*)), this, SLOT(slotSetStarted()));
        connect(p, SIGNAL(completed()), this, SLOT(slotSetCompleted()));
        connect(p, SIGNAL(canceled(const QString &)), this, SLOT(slotSetCanceled(const QString&)));
        connect(p, SIGNAL(completed(bool)), this, SLOT(slotSetCompleted()));
    }

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(visWidget, 0, 0);
    setLayout(layout);
}

void Frame::setRemovable(bool removable)
{
    m_isRemovable = removable;
}

bool Frame::isRemovable() const
{
    return m_isRemovable;
}

Frame::~Frame()
{
    if(m_progressItem) 
    {
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

const QString& Frame::title() const
{
    return m_title;
}

const QString& Frame::caption() const
{
    return m_caption;
}

const QString& Frame::statusText() const
{
    return m_statusText;
}

void Frame::slotSetStarted()
{
    if(m_progressId.isNull() || m_progressId.isEmpty()) m_progressId = KPIM::ProgressManager::getUniqueID();
    m_progressItem = KPIM::ProgressManager::createProgressItem(m_progressId, title(), QString::null, false);
    m_progressItem->setStatus(i18n("Loading..."));
    //connect(m_progressItem, SIGNAL(progressItemCanceled(KPIM::ProgressItem*)), SLOT(slotAbortFetch()));
    m_state=Started;
    emit signalStarted(this);
}

void Frame::slotSetCanceled(const QString &s)
{
    if(m_progressItem) {
        m_progressItem->setStatus(i18n("Loading canceled"));
        m_progressItem->setComplete();
        m_progressItem = 0;
    }
    m_state=Canceled;
    emit signalCanceled(this, s);
}

void Frame::slotSetCompleted()
{
    if(m_progressItem) {
        m_progressItem->setStatus(i18n("Loading completed"));
        m_progressItem->setComplete();
        m_progressItem = 0;
    }
    m_state=Completed;
    emit signalCompleted(this);
}

int Frame::progress() const
{
    return m_progress;
}

MainFrame::MainFrame(QWidget* parent, KParts::ReadOnlyPart* part, QWidget* widget, const QString& title) : Frame(parent, part, widget, title, false)
{
    setRemovable(false);
}

MainFrame::~MainFrame()
{
}

KURL MainFrame::url() const
{
    return KURL();
}

BrowserFrame::BrowserFrame(QWidget* parent, KParts::ReadOnlyPart* part, QWidget* widget, const QString& title) : Frame(parent, part, widget, title, true)
{
    setRemovable(true);
}

BrowserFrame::~BrowserFrame()
{
    part()->deleteLater();
}

KURL BrowserFrame::url() const
{
    return part()->url();
}

FrameManager::FrameManager(QObject* parent) : QObject(parent)
{
}

FrameManager::~FrameManager()
{
}

Frame* FrameManager::currentFrame() const
{
    return m_currentFrame;
}

void FrameManager::addFrame(Frame* frame)
{
    m_frames.insert(frame);

    if (m_frames.count() == 1)
        slotChangeFrame(frame);

    connect(frame, SIGNAL(signalCanceled(Frame*, const QString&)), this, SLOT(slotSetCanceled(Frame*, const QString&)) );
    connect(frame, SIGNAL(signalStarted(Frame*)), this, SLOT(slotSetStarted(Frame*)) );
    connect(frame, SIGNAL(signalCaptionChanged(Frame*, const QString&)), this, SLOT(slotSetCaption(Frame*, const QString&)));
    connect(frame, SIGNAL(signalLoadingProgress(Frame*, int)), this, SLOT(slotSetProgress(Frame*, int)));
    connect(frame, SIGNAL(signalCompleted(Frame*)), this, SLOT(slotSetCompleted(Frame*)));
    connect(frame, SIGNAL(signalTitleChanged(Frame*, const QString&)), this, SLOT(slotSetTitle(Frame*, const QString&)) );
    connect(frame, SIGNAL(signalStatusText(Frame*, const QString&)), this, SLOT(slotSetStatusText(Frame*, const QString&)) );
}

void FrameManager::removeFrame(Frame* frame)
{

    disconnect(frame, SIGNAL(signalCanceled(Frame*, const QString&)), this, SLOT(slotSetCanceled(Frame*, const QString&)) );
    disconnect(frame, SIGNAL(signalStarted(Frame*)), this, SLOT(slotSetStarted(Frame*)) );
    disconnect(frame, SIGNAL(signalCaptionChanged(Frame*, const QString&)), this, SLOT(slotSetCaption(Frame*, const QString&)));
    disconnect(frame, SIGNAL(signalLoadingProgress(Frame*, int)), this, SLOT(slotSetProgress(Frame*, int)));
    disconnect(frame, SIGNAL(signalCompleted(Frame*)), this, SLOT(slotSetCompleted(Frame*)));
    disconnect(frame, SIGNAL(signalTitleChanged(Frame*, const QString&)), this, SLOT(slotSetTitle(Frame*, const QString&)) );
    disconnect(frame, SIGNAL(signalStatusText(Frame*, const QString&)), this, SLOT(slotSetStatusText(Frame*, const QString&)) );

    if (m_currentFrame == frame)
    {
        slotChangeFrame(0);
    }

    m_frames.remove(frame);

}

void FrameManager::slotChangeFrame(Frame* frame)
{
    if (frame == m_currentFrame)
        return;
    
    m_currentFrame = frame;
    
    if (frame)
    {
        switch (frame->state())
        {
            case Frame::Started:
                emit signalStarted();
                break;
            case Frame::Canceled:
                emit signalCanceled(QString::null);
                break;
            case Frame::Idle:
            case Frame::Completed:
            default:
                emit signalCompleted();
        }

        emit signalCaptionChanged(frame->caption());
        emit signalTitleChanged(frame->title());
        emit signalLoadingProgress( frame->progress());
        emit signalStatusText( frame->statusText());
    }
    else
    {
        emit signalCompleted();
        emit signalCaptionChanged(QString::null);
        emit signalTitleChanged(QString::null);
        emit signalLoadingProgress(100);
        emit signalStatusText(QString::null);
    }

    emit signalCurrentFrameChanged(frame);
}

void FrameManager::slotSetStarted(Frame* frame)
{
    if (frame == m_currentFrame)
        emit signalStarted();
}

void FrameManager::slotSetCanceled(Frame* frame, const QString& reason)
{
    if (frame == m_currentFrame)
        emit signalCanceled(reason);
}

void FrameManager::slotSetCompleted(Frame* frame)
{
    if (frame == m_currentFrame)
        emit signalCompleted();
}

void FrameManager::slotSetProgress(Frame* frame, int progress)
{
    if (frame == m_currentFrame)
        emit signalLoadingProgress(progress);
}

void FrameManager::slotSetCaption(Frame* frame, const QString& caption)
{
    if (frame == m_currentFrame)
        emit signalCaptionChanged(caption);
}

void FrameManager::slotSetTitle(Frame* frame, const QString& title)
{
    if (frame == m_currentFrame)
        emit signalTitleChanged(title);
}

void FrameManager::slotSetStatusText(Frame* frame, const QString& statusText)
{
    if (frame == m_currentFrame)
        emit signalStatusText(statusText);
}


} // namespace Akregator

#include "frame.moc"
