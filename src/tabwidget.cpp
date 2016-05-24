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

#include "tabwidget.h"

#include <QStyle>
#include <QApplication>
#include <QIcon>
#include <QClipboard>
#include <QHash>
#include <QString>
#include <QToolButton>

#include <QMenu>
#include <QStyleOption>
#include <QDrag>
#include <QMimeData>

#include "akregator_debug.h"
#include <QApplication>
#include <QTabWidget>
#include <qtabbar.h>
#include <krun.h>
#include <KLocalizedString>
#include <kiconloader.h>
#include <ktoolinvocation.h>
#include <QUrl>
#include <kio/global.h>
#include <kio/pixmaploader.h>

#include "actionmanager.h"
#include "akregatorconfig.h"
#include "frame.h"
#include "framemanager.h"
#include "kernel.h"
#include "openurlrequest.h"
#include "utils/temporaryvalue.h"

#include <cassert>

namespace Akregator
{

class Q_DECL_HIDDEN TabWidget::Private
{
private:
    TabWidget *const q;

public:
    explicit Private(TabWidget *qq) : q(qq), currentMaxLength(30), currentItem(0), tabsClose(0) {}

    QHash<QWidget *, Frame *> frames;
    QHash<int, Frame *> framesById;
    int currentMaxLength;
    QWidget *currentItem;
    QToolButton *tabsClose;

    QWidget *selectedWidget() const
    {
        return (currentItem && q->indexOf(currentItem) != -1) ? currentItem : q->currentWidget();
    }

    uint tabBarWidthForMaxChars(int maxLength);
    void setTitle(const QString &title, QWidget *sender);
    void updateTabBarVisibility();
    Frame *currentFrame();
};

void TabWidget::Private::updateTabBarVisibility()
{
    const bool tabBarIsHidden = ((q->count() <= 1) && !Settings::alwaysShowTabBar());
    if (tabBarIsHidden) {
        q->tabBar()->hide();
    } else {
        q->tabBar()->show();
    }
    if (q->count() >= 1 && Settings::closeButtonOnTabs()) {
        q->tabBar()->tabButton(0, QTabBar::RightSide)->hide();
    }
}

TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent), d(new Private(this))
{
    setMinimumSize(250, 150);
    setMovable(false);
    setDocumentMode(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &TabWidget::customContextMenuRequested, this, &TabWidget::slotTabContextMenuRequest);

    connect(this, &TabWidget::currentChanged, this, &TabWidget::slotTabChanged);
    connect(this, &QTabWidget::tabCloseRequested, this, &TabWidget::slotCloseRequest);

    setTabsClosable(Settings::closeButtonOnTabs());

    d->tabsClose = new QToolButton(this);
    connect(d->tabsClose, &QToolButton::clicked, this, &TabWidget::slotRemoveCurrentFrame);

    d->tabsClose->setIcon(QIcon::fromTheme(QStringLiteral("tab-close")));
    d->tabsClose->setEnabled(false);
    d->tabsClose->adjustSize();
    d->tabsClose->setToolTip(i18n("Close the current tab"));

#ifndef QT_NO_ACCESSIBILITY
    d->tabsClose->setAccessibleName(i18n("Close tab"));
#endif

    setCornerWidget(d->tabsClose, Qt::TopRightCorner);
    d->updateTabBarVisibility();
}

TabWidget::~TabWidget()
{
    delete d;
}

void TabWidget::slotTabContextMenuRequest(const QPoint &pos)
{

    QTabBar *bar = tabBar();
    if (count() <= 1) {
        return;
    }

    const int indexBar = bar->tabAt(bar->mapFrom(this, pos));
    if (indexBar == -1) {
        return;
    }
    QMenu menu(this);

    const int countTab = (count() > 1);
    QAction *detachTab = menu.addAction(i18nc("@action:inmenu", "Detach Tab"));
    detachTab->setEnabled((indexBar != 0) && countTab);
    detachTab->setIcon(QIcon::fromTheme(QStringLiteral("tab-detach")));
    menu.addSeparator();

    QAction *closeTab = menu.addAction(i18nc("@action:inmenu", "Close Tab"));
    closeTab->setEnabled((indexBar != 0) && countTab);
    closeTab->setIcon(QIcon::fromTheme(QStringLiteral("tab-close")));

    QAction *allOther = menu.addAction(i18nc("@action:inmenu", "Close All Other Tabs"));
    allOther->setEnabled(countTab);
    allOther->setIcon(QIcon::fromTheme(QStringLiteral("tab-close-other")));

    QAction *allTab = menu.addAction(i18nc("@action:inmenu", "Close All Tabs"));
    allTab->setEnabled(countTab);
    allTab->setIcon(QIcon::fromTheme(QStringLiteral("tab-close")));

    QAction *action = menu.exec(mapToGlobal(pos));

    if (action == allOther) {   // Close all other tabs
        slotCloseAllTabExcept(indexBar);
    } else if (action == closeTab) {
        slotCloseRequest(indexBar);
    } else if (action == allTab) {
        slotCloseAllTab();
    } else if (action == detachTab) {
        slotDetachTab(indexBar);
    }
}

void TabWidget::closeAllTabExcept(int index)
{
    //Don't close first tab
    for (int i = count() - 1; i > 0; --i) {
        if (i == index) {
            continue;
        }
        slotCloseRequest(i);
    }
}

void TabWidget::slotCloseAllTabExcept(int index)
{
    closeAllTabExcept(index);
}

void TabWidget::slotCloseAllTab()
{
    closeAllTabExcept();
}

void TabWidget::slotSettingsChanged()
{
    if (tabsClosable() != Settings::closeButtonOnTabs()) {
        setTabsClosable(Settings::closeButtonOnTabs());
    }
    d->updateTabBarVisibility();
}

void TabWidget::slotNextTab()
{
    setCurrentIndex((currentIndex() + 1) % count());
}

void TabWidget::slotPreviousTab()
{
    if (currentIndex() == 0) {
        setCurrentIndex(count() - 1);
    } else {
        setCurrentIndex(currentIndex() - 1);
    }
}
void TabWidget::slotSelectFrame(int frameId)
{
    Frame *frame = d->framesById.value(frameId);
    if (frame && frame != d->currentFrame()) {
        setCurrentWidget(frame);
        frame->setFocus();
    }
}

void TabWidget::slotAddFrame(Frame *frame)
{
    if (!frame) {
        return;
    }
    d->frames.insert(frame, frame);
    d->framesById.insert(frame->id(), frame);
    addTab(frame, frame->title());
    connect(frame, &Frame::signalTitleChanged, this, &TabWidget::slotSetTitle);

    slotSetTitle(frame, frame->title());
}

Frame *TabWidget::Private::currentFrame()
{
    QWidget *w = q->currentWidget();
    Q_ASSERT(frames.value(w));
    return w ? frames.value(w) : 0;
}

void TabWidget::slotZoomChanged(qreal value)
{
    if (!d->currentFrame()) {
        return;
    }
    Q_EMIT signalZoomChangedInFrame(d->currentFrame()->id(), value);
}

void TabWidget::slotTabChanged(int index)
{
    Frame *frame = d->frames.value(widget(index));
    d->tabsClose->setEnabled(frame && frame->isRemovable());
    Q_EMIT signalCurrentFrameChanged(frame ? frame->id() : -1);
}

void TabWidget::tabInserted(int)
{
    d->updateTabBarVisibility();
}

void TabWidget::tabRemoved(int)
{
    d->updateTabBarVisibility();
}

void TabWidget::slotRemoveCurrentFrame()
{
    Frame *const frame = d->currentFrame();
    if (frame) {
        Q_EMIT signalRemoveFrameRequest(frame->id());
    }
}

void TabWidget::slotRemoveFrame(int frameId)
{
    if (!d->framesById.contains(frameId)) {
        return;
    }
    Frame *f = d->framesById.value(frameId);
    d->frames.remove(f);
    d->framesById.remove(frameId);
    f->disconnect(this);
    removeTab(indexOf(f));
    Q_EMIT signalRemoveFrameRequest(f->id());
    if (d->currentFrame()) {
        d->setTitle(d->currentFrame()->title(), currentWidget());
    }
}

// copied wholesale from KonqFrameTabs
uint TabWidget::Private::tabBarWidthForMaxChars(int maxLength)
{
    int hframe;
    QStyleOption o;
    hframe = q->tabBar()->style()->pixelMetric(QStyle::PM_TabBarTabHSpace, &o, q);

    QFontMetrics fm = q->tabBar()->fontMetrics();
    int x = 0;
    for (int i = 0; i < q->count(); ++i) {
        Frame *f = frames.value(q->widget(i));
        if (!f) {
            continue; // frames is out of sync, e.g. because tabInserted wasn't called yet - #185597
        }
        QString newTitle = f->title();
        if (newTitle.length() > maxLength) {
            newTitle = newTitle.left(maxLength - 3) + QLatin1String("...");
        }

        int lw = fm.width(newTitle);
        int iw = q->tabBar()->tabIcon(i).pixmap(q->tabBar()->style()->pixelMetric(
                QStyle::PM_SmallIconSize), QIcon::Normal
                                               ).width() + 4;

        x += (q->tabBar()->style()->sizeFromContents(QStyle::CT_TabBarTab, &o,
                QSize(qMax(lw + hframe + iw, QApplication::globalStrut().width()), 0), q)).width();
    }
    return x;
}

void TabWidget::slotSetTitle(Frame *frame, const QString &title)
{
    d->setTitle(title, frame);
}

void TabWidget::slotWebPageMutedOrAudibleChanged(Akregator::Frame *frame, bool isAudioMuted, bool wasRecentlyAudible)
{
    const int idx = indexOf(frame);
    if (idx < 0) {
        return;
    }
    qDebug() << "void TabWidget::slotWebPageMutedOrAudibleChanged(Akregator::Frame *frame, bool isAudioMuted, bool wasRecentlyAudible)" << isAudioMuted << " wasRecentlyAudible" << wasRecentlyAudible;
    //TODO
    //setTabIcon(idx, icon);
}

void TabWidget::slotSetIcon(Akregator::Frame *frame, const QIcon &icon)
{
    const int idx = indexOf(frame);
    if (idx < 0) {
        return;
    }
    setTabIcon(idx, icon);
}

void TabWidget::Private::setTitle(const QString &title, QWidget *sender)
{
    int senderIndex = q->indexOf(sender);

    q->setTabToolTip(senderIndex, QString());

    uint lcw = 0, rcw = 0;
    int tabBarHeight = q->tabBar()->sizeHint().height();

    QWidget *leftCorner = q->cornerWidget(Qt::TopLeftCorner);

    if (leftCorner  && leftCorner->isVisible()) {
        lcw = qMax(leftCorner->width(), tabBarHeight);
    }

    QWidget *rightCorner = q->cornerWidget(Qt::TopRightCorner);

    if (rightCorner && rightCorner->isVisible()) {
        rcw = qMax(rightCorner->width(), tabBarHeight);
    }
    uint maxTabBarWidth = q->width() - lcw - rcw;

    int newMaxLength = 30;

    for (; newMaxLength > 3; newMaxLength--) {
        if (tabBarWidthForMaxChars(newMaxLength) < maxTabBarWidth) {
            break;
        }
    }

    QString newTitle = title;
    if (newTitle.length() > newMaxLength) {
        q->setTabToolTip(senderIndex, newTitle);
        newTitle = newTitle.left(newMaxLength - 3) + QLatin1String("...");
    }

    newTitle.replace(QLatin1Char('&'), QStringLiteral("&&"));

    if (q->tabText(senderIndex) != newTitle) {
        q->setTabText(senderIndex, newTitle);
    }

    if (newMaxLength != currentMaxLength) {
        for (int i = 0; i < q->count(); ++i) {
            Frame *f = frames.value(q->widget(i));
            if (!f) {
                continue; // frames is out of sync, e.g. because tabInserted wasn't called yet - #185597
            }
            newTitle = f->title();
            int index = q->indexOf(q->widget(i));
            q->setTabToolTip(index, QString());

            if (newTitle.length() > newMaxLength) {
                q->setTabToolTip(index, newTitle);
                newTitle = newTitle.left(newMaxLength - 3) + QLatin1String("...");
            }

            newTitle.replace(QLatin1Char('&'), QStringLiteral("&&"));
            if (newTitle != q->tabText(index)) {
                q->setTabText(index, newTitle);
            }
        }
        currentMaxLength = newMaxLength;
    }
}

void TabWidget::slotDetachTab(int index)
{
    QWidget *w = widget(index);
    Frame *frame = d->frames.value(w);
    if (frame && frame->url().isValid() && frame->isRemovable()) {
        OpenUrlRequest request;
        request.setUrl(frame->url());
        request.setOptions(OpenUrlRequest::ExternalBrowser);
        Q_EMIT signalOpenUrlRequest(request);
        slotCloseRequest(index);
    }
}

void TabWidget::slotTextToSpeech()
{
    Q_EMIT signalTextToSpeechInFrame(d->currentFrame()->id());
}

void TabWidget::slotFindTextInHtml()
{
    Q_EMIT signalFindTextInFrame(d->currentFrame()->id());
}

void TabWidget::slotCopyLinkAddress()
{
    Q_EMIT signalCopyLinkAsInFrame(d->currentFrame()->id());
}

void TabWidget::slotSaveLinkAs()
{
    Q_EMIT signalSaveLinkAsInFrame(d->currentFrame()->id());
}

void TabWidget::slotPrintPreview()
{
    Q_EMIT signalPrintPreviewInFrame(d->currentFrame()->id());
}

void TabWidget::slotPrint()
{
    Q_EMIT signalPrintInFrame(d->currentFrame()->id());
}

void TabWidget::slotCopy()
{
    Q_EMIT signalCopyInFrame(d->currentFrame()->id());
}

void TabWidget::slotSaveImageOnDisk()
{
    Q_EMIT signalSaveImageOnDisk(d->currentFrame()->id());
}

void TabWidget::slotUnMute()
{
    Q_EMIT signalMute(d->currentFrame()->id(), false);
}

void TabWidget::slotMute()
{
    Q_EMIT signalMute(d->currentFrame()->id(), true);
}

void TabWidget::slotCopyImageLocation()
{
    Q_EMIT signalCopyImageLocation(d->currentFrame()->id());
}

void TabWidget::slotCloseTab()
{
    QWidget *widget =  d->selectedWidget();
    Frame *frame = d->frames.value(widget);

    if (frame == 0 || !frame->isRemovable()) {
        return;
    }

    Q_EMIT signalRemoveFrameRequest(frame->id());
}

void TabWidget::slotReloadAllTabs()
{
    Q_FOREACH (Frame *frame, d->frames) {
        frame->slotReload();
    }
}

void TabWidget::slotCloseRequest(int index)
{
    QWidget *w = widget(index);
    if (d->frames.value(w)) {
        Q_EMIT signalRemoveFrameRequest(d->frames.value(w)->id());
    }
}

void TabWidget::slotActivateTab()
{
    setCurrentIndex(sender()->objectName().rightRef(2).toInt() - 1);
}

} // namespace Akregator

