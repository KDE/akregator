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


#ifndef ARTICLEVIEWERWIDGETNG_H
#define ARTICLEVIEWERWIDGETNG_H

#include <QWidget>
#include "articleviewerng.h"
class KActionCollection;

namespace MessageViewer
{
class FindBarWebView;
}

namespace KPIMTextEdit
{
class TextToSpeechWidget;
}

namespace Akregator
{
class ArticleViewerWidgetNg : public QWidget
{
    Q_OBJECT
public:
    explicit ArticleViewerWidgetNg(KActionCollection *ac, QWidget *parent = Q_NULLPTR);
    ~ArticleViewerWidgetNg();

    ArticleViewerNg *articleViewerNg() const;

Q_SIGNALS:
    void loadUrl(const QUrl &, ArticleViewerNg::OpenUrlType type);

private:
    void initializeActions(KActionCollection *ac);
    MessageViewer::FindBarWebView *mFindBarWebView;
    ArticleViewerNg *mArticleViewerNg;
    KPIMTextEdit::TextToSpeechWidget *mTextToSpeechWidget;
};
}

#endif // ARTICLEVIEWERWIDGETNG_H
