/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "articleviewerwidget.h"
#include "articleviewer-ng/articleviewerwidgetng.h"
#include "mainframe.h"

#include <QGridLayout>
#include <QDebug>

using namespace Akregator;

MainFrame::MainFrame(QWidget *parent, KParts::ReadOnlyPart *part, QWidget *visibleWidget)
    : Frame(parent),
      m_part(part),
      mArticleViewer(Q_NULLPTR)
{
    setRemovable(false);
    QGridLayout *layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->addWidget(visibleWidget, 0, 0);
    setLayout(layout);
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

void MainFrame::slotZoomTextOnlyInFrame(int frameId, bool textOnlyInFrame)
{
    if (frameId != 0) {
        return;
    }
    mArticleViewer->slotZoomTextOnlyInFrame(textOnlyInFrame);
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

bool MainFrame::zoomTextOnlyInFrame() const
{
    return mArticleViewer->zoomTextOnlyInFrame();
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
