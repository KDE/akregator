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

#include "frame.h"

#include <QGridLayout>
#include <QRegExp>

#include <kactioncollection.h>
#include <qdebug.h>
#include <KLocalizedString>
#include <kurl.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>

#include <libkdepim/progresswidget/progressmanager.h>

namespace Akregator {


void Frame::slotSetTitle(const QString &s)
{
    if (m_title != s)
    {
        m_title = s;
        emit signalTitleChanged(this, s);
    }
}

bool Frame::isLoading() const 
{
    return m_loading;
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
    m_statusText.remove(QRegExp(QLatin1String("<[^>]*>")));
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

void Frame::slotSetState(State state)
{
    m_state=state;

    switch (m_state)
    {
        case Frame::Started:
            emit signalStarted(this);
            break;
        case Frame::Canceled:
            emit signalCanceled(this, QString());
            break;
        case Frame::Idle:
        case Frame::Completed:
        default:
            emit signalCompleted(this);
    }
}

Frame::Frame(QWidget* parent)
   : QWidget(parent)
{
    m_title = i18n("Untitled");
    m_state=Idle;
    m_progress=-1;
    m_progressItem=0;
    m_isRemovable = true;
    m_loading = false; 
    m_id = m_idCounter++;
}

int Frame::m_idCounter = 0;

int Frame::id() const
{
    return m_id;
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


Frame::State Frame::state() const
{
    return m_state;
}

QString Frame::title() const
{
    return m_title;
}

QString Frame::caption() const
{
    return m_caption;
}

QString Frame::statusText() const
{
    return m_statusText;
}

void Frame::slotSetStarted()
{
    m_loading = true;
    if(m_progressId.isNull() || m_progressId.isEmpty()) m_progressId = KPIM::ProgressManager::getUniqueID();
    m_progressItem = KPIM::ProgressManager::createProgressItem(m_progressId, title(), QString(), false);
    m_progressItem->setStatus(i18n("Loading..."));
    //connect(m_progressItem, SIGNAL(progressItemCanceled(KPIM::ProgressItem*)), SLOT(slotAbortFetch()));
    m_state=Started;
    emit signalStarted(this);
    emit signalIsLoadingToggled(this, m_loading);
}

void Frame::slotStop()
{
    if (m_loading)
    {
        m_loading = false;
        emit signalIsLoadingToggled(this, false);
    }
}

void Frame::slotSetCanceled(const QString &s)
{
    m_loading = false;
    if(m_progressItem) 
    {
        m_progressItem->setStatus(i18n("Loading canceled"));
        m_progressItem->setComplete();
        m_progressItem = 0;
    }
    m_state=Canceled;
    emit signalCanceled(this, s);
    emit signalIsLoadingToggled(this, m_loading);
}

void Frame::slotSetCompleted()
{
    m_loading = false; 
    if(m_progressItem)
    {
        m_progressItem->setStatus(i18n("Loading completed"));
        m_progressItem->setComplete();
        m_progressItem = 0;
    }
    m_state=Completed;
    emit signalCompleted(this);
    emit signalIsLoadingToggled(this, m_loading);
}

int Frame::progress() const
{
    return m_progress;
}

MainFrame::MainFrame(QWidget* parent, KParts::ReadOnlyPart* part, QWidget* visibleWidget) : Frame(parent), m_part(part)
{
    setRemovable(false);
    QGridLayout* layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->addWidget(visibleWidget, 0, 0);
    setLayout(layout);
}

MainFrame::~MainFrame()
{
}

KUrl MainFrame::url() const
{
    return KUrl();
}

} // namespace Akregator

