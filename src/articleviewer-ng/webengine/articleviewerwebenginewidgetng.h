/*
   SPDX-FileCopyrightText: 2016-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ARTICLEVIEWERWEBENGINEWIDGETNG_H
#define ARTICLEVIEWERWEBENGINEWIDGETNG_H

#include <QWidget>
#include "akregator_export.h"
#include "articleviewerwebengine.h"
class QPrinter;
class KActionCollection;

namespace KPIMTextEdit {
class SlideContainer;
class TextToSpeechWidget;
}

namespace WebEngineViewer {
class FindBarWebEngineView;
}

namespace Akregator {
class AKREGATOR_EXPORT ArticleViewerWebEngineWidgetNg : public QWidget
{
    Q_OBJECT
public:
    explicit ArticleViewerWebEngineWidgetNg(ArticleViewerWebEngine *customViewer, KActionCollection *ac, QWidget *parent);
    explicit ArticleViewerWebEngineWidgetNg(KActionCollection *ac, QWidget *parent = nullptr);
    ~ArticleViewerWebEngineWidgetNg();

    ArticleViewerWebEngine *articleViewerNg() const;

    void saveCurrentPosition();
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
    void slotRestorePosition();
    WebEngineViewer::FindBarWebEngineView *mFindBarWebView = nullptr;
    ArticleViewerWebEngine *mArticleViewerNg = nullptr;
    KPIMTextEdit::TextToSpeechWidget *mTextToSpeechWidget = nullptr;
    KPIMTextEdit::SlideContainer *mSliderContainer = nullptr;
    QPrinter *mCurrentPrinter = nullptr;
};
}
#endif // ARTICLEVIEWERWEBENGINEWIDGETNG_H
