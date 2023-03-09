/*
   SPDX-FileCopyrightText: 2016-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akregator_export.h"
#include "articleviewerwebengine.h"

#include <QWidget>
class QPrinter;
class KActionCollection;

namespace KPIMTextEdit
{
class SlideContainer;
}
#ifdef HAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT
namespace TextEditTextToSpeech
{
class TextToSpeechContainerWidget;
}
#endif

namespace WebEngineViewer
{
class FindBarWebEngineView;
class TrackingWarningWidget;
}

namespace Akregator
{
class AKREGATOR_EXPORT ArticleViewerWebEngineWidgetNg : public QWidget
{
    Q_OBJECT
public:
    explicit ArticleViewerWebEngineWidgetNg(ArticleViewerWebEngine *customViewer, KActionCollection *ac, QWidget *parent);
    explicit ArticleViewerWebEngineWidgetNg(KActionCollection *ac, QWidget *parent = nullptr);
    ~ArticleViewerWebEngineWidgetNg() override;

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
    WebEngineViewer::FindBarWebEngineView *mFindBarWebView = nullptr;
    ArticleViewerWebEngine *mArticleViewerNg = nullptr;
#ifdef HAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT
    TextEditTextToSpeech::TextToSpeechContainerWidget *const mTextToSpeechWidget;
#endif
    KPIMTextEdit::SlideContainer *const mSliderContainer;
    WebEngineViewer::TrackingWarningWidget *const mTrackingWarningWidget;
    QPrinter *mCurrentPrinter = nullptr;
};
}
