/*
   SPDX-FileCopyrightText: 2016-2023 Laurent Montel <montel@kde.org>

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
#ifdef HAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT
#include <TextEditTextToSpeech/TextToSpeechContainerWidget>
#endif
#include <KPIMTextEdit/SlideContainer>

#include <WebEngineViewer/FindBarWebEngineView>
#include <WebEngineViewer/TrackingWarningWidget>
#include <WebEngineViewer/WebEngineExportHtmlPageJob>

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
#ifdef HAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT
    , mTextToSpeechWidget(new TextEditTextToSpeech::TextToSpeechContainerWidget(this))
#endif
    , mSliderContainer(new KPIMTextEdit::SlideContainer(this))
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
#ifdef HAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT
    mTextToSpeechWidget->setObjectName(QStringLiteral("texttospeechwidget"));
    layout->addWidget(mTextToSpeechWidget);
#endif
    mTrackingWarningWidget->setObjectName(QStringLiteral("mTrackingWarningWidget"));
    layout->addWidget(mTrackingWarningWidget);

    if (!mArticleViewerNg) {
        mArticleViewerNg = new ArticleViewerWebEngine(ac, this);
    }
    mArticleViewerNg->setObjectName(QStringLiteral("articleviewerng"));
    layout->addWidget(mArticleViewerNg);
    connect(mArticleViewerNg, &ArticleViewerWebEngine::mailTrackingFound, mTrackingWarningWidget, &WebEngineViewer::TrackingWarningWidget::addTracker);

    mArticleViewerNg->createViewerPluginToolManager(ac, this);

    mSliderContainer->setObjectName(QStringLiteral("slidercontainer"));
    mFindBarWebView = new WebEngineViewer::FindBarWebEngineView(mArticleViewerNg, this);
    mFindBarWebView->setObjectName(QStringLiteral("findbarwebview"));
    connect(mFindBarWebView, &WebEngineViewer::FindBarWebEngineView::hideFindBar, mSliderContainer, &KPIMTextEdit::SlideContainer::slideOut);
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
    const QString text = mArticleViewerNg->selectedText();
#ifdef HAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT
    mTextToSpeechWidget->say(text);
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
    qDebug() << " print preview ";
    auto printMessage = new MessageViewer::PrintMessage(this);
    printMessage->setParentWidget(this);
    printMessage->setView(mArticleViewerNg);
    printMessage->printPreview();
}

void ArticleViewerWebEngineWidgetNg::slotOpenInBrowser()
{
    const QUrl currentUrl(mArticleViewerNg->url());
    if (currentUrl.isLocalFile()) {
        auto job = new WebEngineViewer::WebEngineExportHtmlPageJob;
        job->setEngineView(mArticleViewerNg);
        connect(job, &WebEngineViewer::WebEngineExportHtmlPageJob::failed, this, &ArticleViewerWebEngineWidgetNg::slotExportHtmlPageFailed);
        connect(job, &WebEngineViewer::WebEngineExportHtmlPageJob::success, this, &ArticleViewerWebEngineWidgetNg::slotExportHtmlPageSuccess);
        job->start();
    } else {
        auto job = new KIO::OpenUrlJob(currentUrl, QStringLiteral("text/html"));
        job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
        job->setDeleteTemporaryFile(true);
        job->start();
    }
}

void ArticleViewerWebEngineWidgetNg::slotExportHtmlPageFailed()
{
    qCDebug(AKREGATOR_LOG) << " Failed to export as HTML";
}

void ArticleViewerWebEngineWidgetNg::slotExportHtmlPageSuccess(const QString &filename)
{
    auto job = new KIO::OpenUrlJob(QUrl::fromLocalFile(filename), QStringLiteral("text/html"));
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
    job->setDeleteTemporaryFile(true);
    job->start();
}

#include "moc_articleviewerwebenginewidgetng.cpp"
