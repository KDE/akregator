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
    : QWidget(parent),
      mFindInMessageAction(Q_NULLPTR)
{
    mArticleViewerNg = customViewer;
    initializeLayout(ac);
}

ArticleViewerWidgetNg::ArticleViewerWidgetNg(KActionCollection *ac, QWidget *parent)
    : QWidget(parent),
      mArticleViewerNg(Q_NULLPTR),
      mFindInMessageAction(Q_NULLPTR)
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

    mSliderContainer = new KPIMTextEdit::SlideContainer(this);
    mSliderContainer->setObjectName(QStringLiteral("slidercontainer"));
    mFindBarWebView = new MessageViewer::FindBarWebView(mArticleViewerNg, this);
    mFindBarWebView->setObjectName(QStringLiteral("findbarwebview"));
    connect(mFindBarWebView, &MessageViewer::FindBarBase::hideFindBar, mSliderContainer, &KPIMTextEdit::SlideContainer::slideOut);
    mSliderContainer->setContent(mFindBarWebView);
    layout->addWidget(mSliderContainer);
    initializeActions(ac);
}

ArticleViewerWidgetNg::~ArticleViewerWidgetNg()
{

}

void ArticleViewerWidgetNg::initializeActions(KActionCollection *ac)
{
    mSpeakTextAction = new QAction(i18n("Speak Text"), this);
    mSpeakTextAction->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-text-to-speech")));
    ac->addAction(QStringLiteral("speak_text"), mSpeakTextAction);
    connect(mSpeakTextAction, &QAction::triggered, this, &ArticleViewerWidgetNg::slotSpeakText);

    mFindInMessageAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-find")), i18n("&Find in Message..."), this);
    ac->addAction(QStringLiteral("find_in_messages"), mFindInMessageAction);
    connect(mFindInMessageAction, &QAction::triggered, this, &ArticleViewerWidgetNg::slotFind);
    ac->setDefaultShortcut(mFindInMessageAction, KStandardShortcut::find().first());
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

}
