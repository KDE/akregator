/*
   Copyright (C) 2016 Montel Laurent <montel@kde.org>

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

#ifndef ARTICLEVIEWERWEBENGINEWIDGETNG_H
#define ARTICLEVIEWERWEBENGINEWIDGETNG_H

#include <QWidget>
#include "akregator_export.h"
#include "articleviewerwebengine.h"
class KActionCollection;

namespace KPIMTextEdit
{
class SlideContainer;
class TextToSpeechWidget;
}

namespace WebEngineViewer
{
class FindBarWebEngineView;
}

namespace Akregator
{
class AKREGATOR_EXPORT ArticleViewerWebEngineWidgetNg : public QWidget
{
    Q_OBJECT
public:
    explicit ArticleViewerWebEngineWidgetNg(ArticleViewerWebEngine *customViewer, KActionCollection *ac, QWidget *parent);
    explicit ArticleViewerWebEngineWidgetNg(KActionCollection *ac, QWidget *parent = Q_NULLPTR);
    ~ArticleViewerWebEngineWidgetNg();

    ArticleViewerWebEngine *articleViewerNg() const;

    void saveCurrentPosition();
    void restoreCurrentPosition();
public Q_SLOTS:
    void slotSpeakText();
    void slotFind();
    void slotPrint();
    void slotPrintPreview();

private Q_SLOTS:
    void slotOpenInBrowser();
    void slotExportHtmlPageSuccess(const QString &filename);
    void slotExportHtmlPageFailed();
    void slotHandlePagePrinted(bool result);
private:
    void initializeActions(KActionCollection *ac);
    void initializeLayout(KActionCollection *ac);
    void printRequested(QWebEnginePage *page);
    WebEngineViewer::FindBarWebEngineView *mFindBarWebView;
    ArticleViewerWebEngine *mArticleViewerNg;
    KPIMTextEdit::TextToSpeechWidget *mTextToSpeechWidget;
    KPIMTextEdit::SlideContainer *mSliderContainer;
    QPrinter *mCurrentPrinter;
};
}
#endif // ARTICLEVIEWERWEBENGINEWIDGETNG_H
