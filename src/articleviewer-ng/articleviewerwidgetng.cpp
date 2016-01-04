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

using namespace Akregator;

ArticleViewerWidgetNg::ArticleViewerWidgetNg(KActionCollection *ac, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    mArticleViewerNg = new ArticleViewerNg(ac, this);
    mArticleViewerNg->setObjectName(QStringLiteral("articleviewerng"));
    layout->addWidget(mArticleViewerNg);

    mFindBarWebView = new MessageViewer::FindBarWebView(mArticleViewerNg, this);
    mFindBarWebView->setObjectName(QStringLiteral("findbarwebview"));
    layout->addWidget(mFindBarWebView);
}

ArticleViewerWidgetNg::~ArticleViewerWidgetNg()
{

}
