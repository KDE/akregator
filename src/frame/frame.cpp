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
#include "utils.h"

#include <KActionCollection>
#include "akregator_debug.h"
#include <KLocalizedString>

#include <Libkdepim/ProgressManager>

using namespace Akregator;
Frame::Frame(QWidget *parent)
    : QWidget(parent)
{
    m_title = i18n("Untitled");
    m_state = Idle;
    m_progress = -1;
    m_progressItem = nullptr;
    m_isRemovable = true;
    m_loading = false;
    m_id = m_idCounter++;
}

void Frame::slotSetTitle(const QString &s)
{
    if (m_title != s) {
        m_title = s;
        Q_EMIT signalTitleChanged(this, s);
    }
}

bool Frame::isLoading() const
{
    return m_loading;
}

void Frame::slotSetCaption(const QString &s)
{
    if (m_progressItem) {
        m_progressItem->setLabel(s);
    }
    m_caption = s;
    Q_EMIT signalCaptionChanged(this, s);
}

void Frame::slotSetStatusText(const QString &s)
{
    m_statusText = s;
    m_statusText = Akregator::Utils::stripTags(m_statusText);
    Q_EMIT signalStatusText(this, m_statusText);
}

void Frame::slotSetProgress(int a)
{
    if (m_progressItem) {
        m_progressItem->setProgress(static_cast<uint>(a));
    }
    m_progress = a;
    Q_EMIT signalLoadingProgress(this, a);
}

void Frame::slotSetState(State state)
{
    m_state = state;

    switch (m_state) {
    case Frame::Started:
        Q_EMIT signalStarted(this);
        break;
    case Frame::Canceled:
        Q_EMIT signalCanceled(this, QString());
        break;
    case Frame::Idle:
    case Frame::Completed:
        Q_EMIT signalCompleted(this);
    }
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
    if (m_progressItem) {
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

QIcon Frame::icon() const
{
    return m_icon;
}

void Frame::setIcon(const QIcon &icon)
{
    m_icon = icon;
}

void Frame::slotSetStarted()
{
    m_loading = true;
    if (m_progressId.isNull() || m_progressId.isEmpty()) {
        m_progressId = KPIM::ProgressManager::getUniqueID();
    }
    m_progressItem = KPIM::ProgressManager::createProgressItem(m_progressId, title(), QString(), false);
    m_progressItem->setStatus(i18n("Loading..."));
    m_state = Started;
    Q_EMIT signalStarted(this);
}

void Frame::slotStop()
{
    if (m_loading) {
        m_loading = false;
    }
}

void Frame::slotSetCanceled(const QString &s)
{
    m_loading = false;
    if (m_progressItem) {
        m_progressItem->setStatus(i18n("Loading canceled"));
        m_progressItem->setComplete();
        m_progressItem = nullptr;
    }
    m_state = Canceled;
    Q_EMIT signalCanceled(this, s);
}

void Frame::slotSetCompleted()
{
    m_loading = false;
    if (m_progressItem) {
        m_progressItem->setStatus(i18n("Loading completed"));
        m_progressItem->setComplete();
        m_progressItem = nullptr;
    }
    m_state = Completed;
    Q_EMIT signalCompleted(this);
}

int Frame::progress() const
{
    return m_progress;
}
