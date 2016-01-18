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

#ifndef AKREGATOR_TABWIDGET_H
#define AKREGATOR_TABWIDGET_H

#include <QTabWidget>

class QString;

namespace Akregator
{

class Frame;
class OpenUrlRequest;

class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit TabWidget(QWidget *parent = Q_NULLPTR);
    ~TabWidget();

public Q_SLOTS:
    void slotCopy();
    void slotZoomChanged(qreal value);

    void slotSetTitle(Akregator::Frame *frame, const QString &title);
    void slotSetIcon(Akregator::Frame *frame, const QIcon &icon);
    void slotSettingsChanged();
    void slotNextTab();
    void slotPreviousTab();
    void slotRemoveCurrentFrame();
    void slotAddFrame(Akregator::Frame *f);
    void slotRemoveFrame(int frameId);
    void slotSelectFrame(int frameId);
    void slotReloadAllTabs();
    void slotActivateTab();
    void slotDetachTab();
    void slotCopyLinkAddress();
    void slotCloseTab();
    void slotZoomTextOnlyChanged(bool zoomTextOnly);
    void slotPrint();
    void slotPrintPreview();
    void slotFindTextInHtml();
    void slotTextToSpeech();
Q_SIGNALS:
    void signalCurrentFrameChanged(int);
    void signalRemoveFrameRequest(int);
    void signalOpenUrlRequest(Akregator::OpenUrlRequest &);
    void signalZoomChangedInFrame(int currentFrame, qreal value);
    void signalZoomTextOnlyInFrame(int currentFrame, bool zoomTextOnly);
    void signalPrintInFrame(int currentFrame);
    void signalCopyInFrame(int currentFrame);
    void signalPrintPreviewInFrame(int currentFrame);
    void signalTextToSpeechInFrame(int currentFrame);
    void signalFindTextInFrame(int currentFrame);

private:
    void tabInserted(int) Q_DECL_OVERRIDE;
    void tabRemoved(int) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotCloseRequest(int index);
    void slotTabChanged(int index);
    void slotCloseAllTab();
    void slotTabContextMenuRequest(const QPoint &pos);
    void slotCloseAllTabExcept(int index);
private:
    void closeAllTabExcept(int index = -1);
    class Private;
    Private *const d;
};

} // namespace Akregator

#endif // AKREGATOR_TABWIDGET_H
