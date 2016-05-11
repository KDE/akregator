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

#include <KActionCollection>
#include <KLocalizedString>
#include <QVBoxLayout>
#include <QAction>
#include <viewerplugintoolmanager.h>

#include <KPIMTextEdit/kpimtextedit/texttospeechwidget.h>

#include <kpimtextedit/slidecontainer.h>

#include <WebEngineViewer/FindBarWebEngineView>

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
