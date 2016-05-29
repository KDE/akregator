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

#include <MimeTreeParser/AttachmentTemporaryFilesDirs>

#include <WebEngineViewer/WebEnginePrintMessageBox>
#include <WebEngineViewer/FindBarWebEngineView>
#include <webengineviewer/config-webengineviewer.h>
#include <WebEngineViewer/WebEngineExportHtmlPageJob>
#ifdef WEBENGINEVIEWER_PRINTPREVIEW_SUPPORT
#include <WebEngineViewer/PrintPreviewDialog>
#include <WebEngineViewer/PrintWebEngineViewJob>
#include <WebEngineViewer/PrintConfigureDialog>
#endif


using namespace Akregator;

ArticleViewerWebEngineWidgetNg::ArticleViewerWebEngineWidgetNg(ArticleViewerWebEngine *customViewer, KActionCollection *ac, QWidget *parent)
    : QWidget(parent)
{
    mArticleViewerNg = customViewer;
    initializeLayout(ac);
}

ArticleViewerWebEngineWidgetNg::ArticleViewerWebEngineWidgetNg(KActionCollection *ac, QWidget *parent)
    : QWidget(parent),
      mArticleViewerNg(Q_NULLPTR)
{
    initializeLayout(ac);
}

void ArticleViewerWebEngineWidgetNg::initializeLayout(KActionCollection *ac)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

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
    //Use the same code for the moment.
    slotPrintPreview();
}

void ArticleViewerWebEngineWidgetNg::slotPrintPreview()
{
    QPointer<WebEngineViewer::WebEnginePrintMessageBox> dialog = new WebEngineViewer::WebEnginePrintMessageBox(this);
    connect(dialog.data(), &WebEngineViewer::WebEnginePrintMessageBox::openInBrowser, this, &ArticleViewerWebEngineWidgetNg::slotOpenInBrowser);
    connect(dialog.data(), &WebEngineViewer::WebEnginePrintMessageBox::openPrintPreview, this, &ArticleViewerWebEngineWidgetNg::slotOpenPrintPreviewDialog);
    dialog->exec();
    delete dialog;
}

void ArticleViewerWebEngineWidgetNg::slotOpenInBrowser()
{
    WebEngineViewer::WebEngineExportHtmlPageJob *job = new WebEngineViewer::WebEngineExportHtmlPageJob;
    job->setEngineView(mArticleViewerNg);
    connect(job, &WebEngineViewer::WebEngineExportHtmlPageJob::failed, this, &ArticleViewerWebEngineWidgetNg::slotExportHtmlPageFailed);
    connect(job, &WebEngineViewer::WebEngineExportHtmlPageJob::success, this, &ArticleViewerWebEngineWidgetNg::slotExportHtmlPageSuccess);
    job->start();
}

void ArticleViewerWebEngineWidgetNg::slotExportHtmlPageFailed()
{
    qCDebug(AKREGATOR_LOG) << " Failed to export as HTML";
}

void ArticleViewerWebEngineWidgetNg::slotExportHtmlPageSuccess(const QString &filename)
{
    MimeTreeParser::AttachmentTemporaryFilesDirs *browserTemporaryFile = new MimeTreeParser::AttachmentTemporaryFilesDirs;
    browserTemporaryFile->addTempFile(filename);
    const QUrl url(QUrl::fromLocalFile(filename));
    KRun::runUrl(url, QStringLiteral("text/html"), this);
    browserTemporaryFile->removeTempFiles();
    browserTemporaryFile = Q_NULLPTR;
}

void ArticleViewerWebEngineWidgetNg::slotOpenPrintPreviewDialog()
{
#ifdef WEBENGINEVIEWER_PRINTPREVIEW_SUPPORT
    QPointer<WebEngineViewer::PrintConfigureDialog> dlg = new WebEngineViewer::PrintConfigureDialog(this);
    QPageLayout pageLayout;
    if (dlg->exec()) {
        pageLayout = dlg->currentPageLayout();
    }
    delete dlg;
    if (pageLayout.isValid()) {
        WebEngineViewer::PrintWebEngineViewJob *job = new WebEngineViewer::PrintWebEngineViewJob(this);
        job->setEngineView(mArticleViewerNg);
        job->setPageLayout(pageLayout);
        connect(job, &WebEngineViewer::PrintWebEngineViewJob::failed, this, &ArticleViewerWebEngineWidgetNg::slotPdfFailed);
        connect(job, &WebEngineViewer::PrintWebEngineViewJob::success, this, &ArticleViewerWebEngineWidgetNg::slotPdfCreated);
        job->start();
    }
#endif
}

void ArticleViewerWebEngineWidgetNg::slotPdfCreated(const QString &filename)
{
#ifdef WEBENGINEVIEWER_PRINTPREVIEW_SUPPORT
    WebEngineViewer::PrintPreviewDialog dlg(this);
    dlg.loadFile(filename, true);
    dlg.exec();
#endif
}

void ArticleViewerWebEngineWidgetNg::slotPdfFailed()
{
    qCDebug(AKREGATOR_LOG) << "Print to pdf Failed";
}


