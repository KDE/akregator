/*
   Copyright (C) 2016-2017 Montel Laurent <montel@kde.org>

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
#include <viewerplugintoolmanager.h>
#include <KRun>

#include <KPIMTextEdit/kpimtextedit/texttospeechwidget.h>

#include <kpimtextedit/slidecontainer.h>

#include <WebEngineViewer/FindBarWebEngineView>
#include <WebEngineViewer/WebEngineExportHtmlPageJob>

#include <QApplication>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>

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
    , mCurrentPrinter(nullptr)
{
    mArticleViewerNg = customViewer;
    initializeLayout(ac);
}

ArticleViewerWebEngineWidgetNg::ArticleViewerWebEngineWidgetNg(KActionCollection *ac, QWidget *parent)
    : QWidget(parent)
    , mArticleViewerNg(nullptr)
    , mCurrentPrinter(nullptr)
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

void ArticleViewerWebEngineWidgetNg::slotPrint()
{
    printRequested(mArticleViewerNg->page());
}

void ArticleViewerWebEngineWidgetNg::printRequested(QWebEnginePage *page)
{
    if (mCurrentPrinter) {
        return;
    }
    mCurrentPrinter = new QPrinter();
    QPointer<QPrintDialog> dialog = new QPrintDialog(mCurrentPrinter, this);
    dialog->setWindowTitle(i18n("Print Document"));
    if (dialog->exec() != QDialog::Accepted) {
        slotHandlePagePrinted(false);
        delete dialog;
        return;
    }
    delete dialog;
    page->print(mCurrentPrinter, invoke(this, &ArticleViewerWebEngineWidgetNg::slotHandlePagePrinted));
}

void ArticleViewerWebEngineWidgetNg::slotHandlePagePrinted(bool result)
{
    Q_UNUSED(result);
    delete mCurrentPrinter;
    mCurrentPrinter = nullptr;
}

void ArticleViewerWebEngineWidgetNg::slotPrintPreview()
{
    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->resize(800, 750);

    connect(dialog, &QPrintPreviewDialog::paintRequested, this, [=](QPrinter *printing) {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        mArticleViewerNg->execPrintPreviewPage(printing, 10*1000);
        QApplication::restoreOverrideCursor();
    });

    dialog->open();
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
        KRun::RunFlags flags;
        flags |= KRun::RunExecutables;
        KRun::runUrl(currentUrl, QStringLiteral("text/html"), this, flags);
    }
}

void ArticleViewerWebEngineWidgetNg::slotExportHtmlPageFailed()
{
    qCDebug(AKREGATOR_LOG) << " Failed to export as HTML";
}

void ArticleViewerWebEngineWidgetNg::slotExportHtmlPageSuccess(const QString &filename)
{
    const QUrl url(QUrl::fromLocalFile(filename));
    KRun::RunFlags flags;
    flags |= KRun::DeleteTemporaryFiles;
    KRun::runUrl(url, QStringLiteral("text/html"), this, flags);
}
