/*
   SPDX-FileCopyrightText: 2016-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akregator_export.h"
#include "articleviewerwebengine.h"
#include "config-akregator.h"

#include <QWidget>
class KActionCollection;

namespace TextAddonsWidgets
{
class SlideContainer;
}
#if HAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT
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

private:
    void initializeLayout(KActionCollection *ac);
    WebEngineViewer::FindBarWebEngineView *mFindBarWebView = nullptr;
    ArticleViewerWebEngine *mArticleViewerNg = nullptr;
#if HAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT
    TextEditTextToSpeech::TextToSpeechContainerWidget *const mTextToSpeechWidget;
#endif
    TextAddonsWidgets::SlideContainer *const mSliderContainer;
    WebEngineViewer::TrackingWarningWidget *const mTrackingWarningWidget;
};
}
