/*
   SPDX-FileCopyrightText: 2016-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "articleviewerwebenginewidgetng.h"
#include "akregator_debug.h"
#include <KActionCollection>
#include <KIO/JobUiDelegateFactory>
#include <KIO/OpenUrlJob>
#include <KLocalizedString>
#include <MessageViewer/PrintMessage>
#include <MessageViewer/ViewerPluginToolManager>
#include <QVBoxLayout>
#if HAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT
#include <TextEditTextToSpeech/TextToSpeechContainerWidget>
#endif
#include <TextAddonsWidgets/SlideContainer>

#include <WebEngineViewer/FindBarWebEngineView>
#include <WebEngineViewer/TrackingWarningWidget>

#include <QApplication>

using namespace Akregator;
template<typename Arg, typename R, typename C>
struct InvokeWrapper {
    R *receiver;
    void (C::*memberFun)(Arg);
    void operator()(Arg result)
    {
        (receiver->*memberFun)(result);
    }
};

template<typename Arg, typename R, typename C>
InvokeWrapper<Arg, R, C> invoke(R *receiver, void (C::*memberFun)(Arg))
{
    InvokeWrapper<Arg, R, C> wrapper = {receiver, memberFun};
    return wrapper;
}

ArticleViewerWebEngineWidgetNg::ArticleViewerWebEngineWidgetNg(ArticleViewerWebEngine *customViewer, KActionCollection *ac, QWidget *parent)
    : QWidget(parent)
    , mArticleViewerNg(customViewer)
#if HAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT
    , mTextToSpeechWidget(new TextEditTextToSpeech::TextToSpeechContainerWidget(this))
#endif
    , mSliderContainer(new TextAddonsWidgets::SlideContainer(this))
    , mTrackingWarningWidget(new WebEngineViewer::TrackingWarningWidget(this))

{
    initializeLayout(ac);
}

ArticleViewerWebEngineWidgetNg::ArticleViewerWebEngineWidgetNg(KActionCollection *ac, QWidget *parent)
    : ArticleViewerWebEngineWidgetNg::ArticleViewerWebEngineWidgetNg(nullptr, ac, parent)
{
}

void ArticleViewerWebEngineWidgetNg::initializeLayout(KActionCollection *ac)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins({});
    layout->setSpacing(0);
#if HAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT
    mTextToSpeechWidget->setObjectName(QLatin1StringView("texttospeechwidget"));
    layout->addWidget(mTextToSpeechWidget);
#endif
    mTrackingWarningWidget->setObjectName(QLatin1StringView("mTrackingWarningWidget"));
    layout->addWidget(mTrackingWarningWidget);

    if (!mArticleViewerNg) {
        mArticleViewerNg = new ArticleViewerWebEngine(ac, this);
    }
    mArticleViewerNg->setObjectName(QLatin1StringView("articleviewerng"));
    layout->addWidget(mArticleViewerNg);
    connect(mArticleViewerNg, &ArticleViewerWebEngine::mailTrackingFound, mTrackingWarningWidget, &WebEngineViewer::TrackingWarningWidget::addTracker);

    mArticleViewerNg->createViewerPluginToolManager(ac, this);

    mSliderContainer->setObjectName(QLatin1StringView("slidercontainer"));
    mFindBarWebView = new WebEngineViewer::FindBarWebEngineView(mArticleViewerNg, this);
    mFindBarWebView->setObjectName(QLatin1StringView("findbarwebview"));
    connect(mFindBarWebView, &WebEngineViewer::FindBarWebEngineView::hideFindBar, mSliderContainer, &TextAddonsWidgets::SlideContainer::slideOut);
    mSliderContainer->setContent(mFindBarWebView);
    layout->addWidget(mSliderContainer);
    connect(articleViewerNg(), &ArticleViewerWebEngine::textToSpeech, this, &ArticleViewerWebEngineWidgetNg::slotSpeakText);
    connect(articleViewerNg(), &ArticleViewerWebEngine::findTextInHtml, this, &ArticleViewerWebEngineWidgetNg::slotFind);
}

ArticleViewerWebEngineWidgetNg::~ArticleViewerWebEngineWidgetNg() = default;

ArticleViewerWebEngine *ArticleViewerWebEngineWidgetNg::articleViewerNg() const
{
    return mArticleViewerNg;
}

void ArticleViewerWebEngineWidgetNg::slotFind()
{
    if (mArticleViewerNg->hasSelection()) {
        mFindBarWebView->setText(mArticleViewerNg->selectedText());
    }
    mSliderContainer->slideIn();
    mFindBarWebView->focusAndSetCursor();
}

void ArticleViewerWebEngineWidgetNg::slotSpeakText()
{
#if HAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT
    const QString text = mArticleViewerNg->selectedText();
    mTextToSpeechWidget->enqueue(text);
#endif
}

void ArticleViewerWebEngineWidgetNg::saveCurrentPosition()
{
    mArticleViewerNg->saveRelativePosition();
}

void ArticleViewerWebEngineWidgetNg::slotPrint()
{
    auto printMessage = new MessageViewer::PrintMessage(this);
    printMessage->setParentWidget(this);
    printMessage->setView(mArticleViewerNg);
    printMessage->print();
}

void ArticleViewerWebEngineWidgetNg::slotPrintPreview()
{
    auto printMessage = new MessageViewer::PrintMessage(this);
    printMessage->setParentWidget(this);
    printMessage->setView(mArticleViewerNg);
    printMessage->printPreview();
}

#include "moc_articleviewerwebenginewidgetng.cpp"
