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

#include "articleviewerwebenginewidgetng.h"
#include "akregator_debug.h"
#include <KActionCollection>
#include <KLocalizedString>
#include <QVBoxLayout>
#include <QAction>
#include <viewerplugintoolmanager.h>
#include <KRun>

#include <KPIMTextEdit/kpimtextedit/texttospeechwidget.h>

#include <kpimtextedit/slidecontainer.h>

#include <WebEngineViewer/WebEnginePrintMessageBox>
#include <WebEngineViewer/FindBarWebEngineView>
#include <webengineviewer/config-webengineviewer.h>
#include <WebEngineViewer/WebEngineExportHtmlPageJob>

#include <QPrinter>
#ifdef WEBENGINEVIEWER_PRINT_SUPPORT
#include <QPrintDialog>
#endif

using namespace Akregator;
template<typename Arg, typename R, typename C>
struct InvokeWrapper {
    R *receiver;
    void (C::*memberFun)(Arg);
    void operator()(Arg result) {
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
    : QWidget(parent),
      mCurrentPrinter(Q_NULLPTR)
{
    mArticleViewerNg = customViewer;
    initializeLayout(ac);
}

ArticleViewerWebEngineWidgetNg::ArticleViewerWebEngineWidgetNg(KActionCollection *ac, QWidget *parent)
    : QWidget(parent),
      mArticleViewerNg(Q_NULLPTR),
      mCurrentPrinter(Q_NULLPTR)
{
    initializeLayout(ac);
}

void ArticleViewerWebEngineWidgetNg::initializeLayout(KActionCollection *ac)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    mTextToSpeechWidget = new KPIMTextEdit::TextToSpeechWidget(this);
    mTextToSpeechWidget->setObjectName(QStringLiteral("texttospeechwidget"));
    layout->addWidget(mTextToSpeechWidget);

    if (!mArticleViewerNg) {
        mArticleViewerNg = new ArticleViewerWebEngine(ac, this);
    }
    mArticleViewerNg->setObjectName(QStringLiteral("articleviewerng"));
    layout->addWidget(mArticleViewerNg);

    mArticleViewerNg->createViewerPluginToolManager(ac, this);

    mSliderContainer = new KPIMTextEdit::SlideContainer(this);
    mSliderContainer->setObjectName(QStringLiteral("slidercontainer"));
    mFindBarWebView = new WebEngineViewer::FindBarWebEngineView(mArticleViewerNg, this);
    mFindBarWebView->setObjectName(QStringLiteral("findbarwebview"));
    connect(mFindBarWebView, &WebEngineViewer::FindBarWebEngineView::hideFindBar, mSliderContainer, &KPIMTextEdit::SlideContainer::slideOut);
    mSliderContainer->setContent(mFindBarWebView);
    layout->addWidget(mSliderContainer);
    connect(articleViewerNg(), &ArticleViewerWebEngine::textToSpeech, this, &ArticleViewerWebEngineWidgetNg::slotSpeakText);
    connect(articleViewerNg(), &ArticleViewerWebEngine::findTextInHtml, this, &ArticleViewerWebEngineWidgetNg::slotFind);
}

ArticleViewerWebEngineWidgetNg::~ArticleViewerWebEngineWidgetNg()
{

}

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
    mTextToSpeechWidget->say(text);
}

void ArticleViewerWebEngineWidgetNg::saveCurrentPosition()
{
    mArticleViewerNg->saveRelativePosition();
}

void ArticleViewerWebEngineWidgetNg::restoreCurrentPosition()
{
    mArticleViewerNg->restoreCurrentPosition();
}

void ArticleViewerWebEngineWidgetNg::slotPrint()
{
#ifdef WEBENGINEVIEWER_PRINT_SUPPORT
    printRequested(mArticleViewerNg->page());
#else
    //Use the same code for the moment.
    slotPrintPreview();
#endif
}

void ArticleViewerWebEngineWidgetNg::printRequested(QWebEnginePage *page)
{
#ifdef WEBENGINEVIEWER_PRINT_SUPPORT
    if (mCurrentPrinter)
        return;
    mCurrentPrinter = new QPrinter();
    QPrintDialog *dialog = new QPrintDialog(mCurrentPrinter, this);
    dialog->setWindowTitle(i18n("Print Document"));
    if (dialog->exec() != QDialog::Accepted) {
        slotHandlePagePrinted(false);
        return;
    }
    page->print(mCurrentPrinter, invoke(this, &ArticleViewerWebEngineWidgetNg::slotHandlePagePrinted));
#endif
}

void ArticleViewerWebEngineWidgetNg::slotHandlePagePrinted(bool result)
{
    Q_UNUSED(result);
    delete mCurrentPrinter;
    mCurrentPrinter = Q_NULLPTR;
}


void ArticleViewerWebEngineWidgetNg::slotPrintPreview()
{
    QPointer<WebEngineViewer::WebEnginePrintMessageBox> dialog = new WebEngineViewer::WebEnginePrintMessageBox(this);
    connect(dialog.data(), &WebEngineViewer::WebEnginePrintMessageBox::openInBrowser, this, &ArticleViewerWebEngineWidgetNg::slotOpenInBrowser);
    dialog->exec();
    delete dialog;
}

void ArticleViewerWebEngineWidgetNg::slotOpenInBrowser()
{
    const QUrl currentUrl(mArticleViewerNg->url());
    if (currentUrl.isLocalFile()) {
        WebEngineViewer::WebEngineExportHtmlPageJob *job = new WebEngineViewer::WebEngineExportHtmlPageJob;
        job->setEngineView(mArticleViewerNg);
        connect(job, &WebEngineViewer::WebEngineExportHtmlPageJob::failed, this, &ArticleViewerWebEngineWidgetNg::slotExportHtmlPageFailed);
        connect(job, &WebEngineViewer::WebEngineExportHtmlPageJob::success, this, &ArticleViewerWebEngineWidgetNg::slotExportHtmlPageSuccess);
        job->start();
    } else {
        KRun::runUrl(currentUrl, QStringLiteral("text/html"), this);
    }
}

void ArticleViewerWebEngineWidgetNg::slotExportHtmlPageFailed()
{
    qCDebug(AKREGATOR_LOG) << " Failed to export as HTML";
}

void ArticleViewerWebEngineWidgetNg::slotExportHtmlPageSuccess(const QString &filename)
{
    const QUrl url(QUrl::fromLocalFile(filename));
    KRun::runUrl(url, QStringLiteral("text/html"), this, true);
}

