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

#include "articleviewerwidgetng.h"
#include "articleviewerng.h"
#include <MessageViewer/FindBarWebView>

#include <KActionCollection>
#include <KLocalizedString>
#include <QVBoxLayout>
#include <QAction>

#include <KPIMTextEdit/kpimtextedit/texttospeechwidget.h>

#include <kpimtextedit/slidecontainer.h>

using namespace Akregator;

ArticleViewerWidgetNg::ArticleViewerWidgetNg(ArticleViewerNg *customViewer, KActionCollection *ac, QWidget *parent)
    : QWidget(parent)
{
    mArticleViewerNg = customViewer;
    initializeLayout(ac);
}

ArticleViewerWidgetNg::ArticleViewerWidgetNg(KActionCollection *ac, QWidget *parent)
    : QWidget(parent),
      mArticleViewerNg(Q_NULLPTR)
{
    initializeLayout(ac);
}

void ArticleViewerWidgetNg::initializeLayout(KActionCollection *ac)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    mTextToSpeechWidget = new KPIMTextEdit::TextToSpeechWidget(this);
    mTextToSpeechWidget->setObjectName(QStringLiteral("texttospeechwidget"));
    layout->addWidget(mTextToSpeechWidget);

    if (!mArticleViewerNg) {
        mArticleViewerNg = new ArticleViewerNg(ac, this);
    }
    mArticleViewerNg->setObjectName(QStringLiteral("articleviewerng"));
    layout->addWidget(mArticleViewerNg);

    mArticleViewerNg->createViewerPluginToolManager(ac, this);

    mSliderContainer = new KPIMTextEdit::SlideContainer(this);
    mSliderContainer->setObjectName(QStringLiteral("slidercontainer"));
    mFindBarWebView = new MessageViewer::FindBarWebView(mArticleViewerNg, this);
    mFindBarWebView->setObjectName(QStringLiteral("findbarwebview"));
    connect(mFindBarWebView, &MessageViewer::FindBarBase::hideFindBar, mSliderContainer, &KPIMTextEdit::SlideContainer::slideOut);
    mSliderContainer->setContent(mFindBarWebView);
    layout->addWidget(mSliderContainer);
    connect(articleViewerNg(), &ArticleViewerNg::textToSpeech, this, &ArticleViewerWidgetNg::slotSpeakText);
    connect(articleViewerNg(), &ArticleViewerNg::findTextInHtml, this, &ArticleViewerWidgetNg::slotFind);
}

ArticleViewerWidgetNg::~ArticleViewerWidgetNg()
{

}

ArticleViewerNg *ArticleViewerWidgetNg::articleViewerNg() const
{
    return mArticleViewerNg;
}

void ArticleViewerWidgetNg::slotFind()
{
    if (mArticleViewerNg->hasSelection()) {
        mFindBarWebView->setText(mArticleViewerNg->selectedText());
    }
    mSliderContainer->slideIn();
    mFindBarWebView->focusAndSetCursor();
}

void ArticleViewerWidgetNg::slotSpeakText()
{
    const QString text = mArticleViewerNg->selectedText();
    mTextToSpeechWidget->say(text);
}
