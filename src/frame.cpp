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

Frame::Frame(QWidget* parent)
   : QWidget(parent)
{
    m_title = i18n("Untitled");
    m_state=Idle;
    m_progress=-1;
    m_progressItem=0;
    m_isRemovable = true;
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

MainFrame::MainFrame(QWidget* parent, KParts::ReadOnlyPart* part, QWidget* visibleWidget, const QString& /*title*/) : Frame(parent), m_part(part)
{

    setRemovable(false);
    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(visibleWidget, 0, 0);
    setLayout(layout);
}

MainFrame::~MainFrame()
{
}

KURL MainFrame::url() const
{
    return KURL();
}

} // namespace Akregator

#include "frame.moc"
