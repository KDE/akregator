/*
   SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mainframe.h"
#include "articleviewer-ng/webengine/articleviewerwebenginewidgetng.h"
#include "articleviewerwidget.h"

#include <QGridLayout>

using namespace Akregator;

MainFrame::MainFrame(QWidget *parent, QWidget *visibleWidget)
    : Frame(parent)
{
    setRemovable(false);
    auto *layout = new QGridLayout(this);
    layout->setContentsMargins({});
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
