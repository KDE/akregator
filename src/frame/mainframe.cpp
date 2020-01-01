/*
   Copyright (C) 2016-2020 Laurent Montel <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "mainframe.h"
#include "articleviewerwidget.h"
#include "articleviewer-ng/webengine/articleviewerwebenginewidgetng.h"

#include <QGridLayout>

using namespace Akregator;

MainFrame::MainFrame(QWidget *parent, QWidget *visibleWidget)
    : Frame(parent)
    , mArticleViewer(nullptr)
{
    setRemovable(false);
    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(visibleWidget, 0, 0);
}

MainFrame::~MainFrame()
{
}

QUrl MainFrame::url() const
{
    return QUrl();
}

void MainFrame::setArticleViewer(Akregator::ArticleViewerWidget *articleViewer)
{
    mArticleViewer = articleViewer;
}

void MainFrame::slotZoomChangeInFrame(int frameId, qreal value)
{
    if (frameId != 0) {
        return;
    }
    mArticleViewer->slotZoomChangeInFrame(value);
}

void MainFrame::slotCopyInFrame(int frameId)
{
    if (frameId != 0) {
        return;
    }
    mArticleViewer->slotCopy();
}

void MainFrame::slotPrintInFrame(int frameId)
{
    if (frameId != 0) {
        return;
    }
    mArticleViewer->slotPrint();
}

void MainFrame::slotPrintPreviewInFrame(int frameId)
{
    if (frameId != 0) {
        return;
    }
    mArticleViewer->slotPrintPreview();
}

void MainFrame::slotFindTextInFrame(int frameId)
{
    if (frameId != 0) {
        return;
    }
    mArticleViewer->articleViewerWidgetNg()->slotFind();
}

void MainFrame::slotTextToSpeechInFrame(int frameId)
{
    if (frameId != 0) {
        return;
    }
    mArticleViewer->articleViewerWidgetNg()->slotSpeakText();
}

qreal MainFrame::zoomFactor() const
{
    return mArticleViewer->zoomFactor();
}

void MainFrame::slotSaveLinkAsInFrame(int frameId)
{
    if (frameId != 0) {
        return;
    }
    mArticleViewer->articleViewerWidgetNg()->articleViewerNg()->slotSaveLinkAs();
}

void MainFrame::slotCopyLinkAsInFrame(int frameId)
{
    if (frameId != 0) {
        return;
    }
    mArticleViewer->articleViewerWidgetNg()->articleViewerNg()->slotCopyLinkAddress();
}

void MainFrame::slotSaveImageOnDiskInFrame(int frameId)
{
    if (frameId != 0) {
        return;
    }
    mArticleViewer->articleViewerWidgetNg()->articleViewerNg()->slotSaveImageOnDiskInFrame();
}

void MainFrame::slotMute(int frameId, bool mute)
{
    if (frameId != 0) {
        return;
    }
    mArticleViewer->articleViewerWidgetNg()->articleViewerNg()->slotMute(mute);
}

void MainFrame::slotCopyImageLocationInFrame(int frameId)
{
    if (frameId != 0) {
        return;
    }
    mArticleViewer->articleViewerWidgetNg()->articleViewerNg()->slotCopyImageLocationInFrame();
}
