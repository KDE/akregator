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

#ifndef ARTICLEVIEWERWEBENGINEWIDGETNG_H
#define ARTICLEVIEWERWEBENGINEWIDGETNG_H

#include <QWidget>
#include "akregator_export.h"
#include "articleviewerwebengine.h"
class KActionCollection;

namespace KPIMTextEdit
{
class SlideContainer;
class TextToSpeechWidget;
}

namespace WebEngineViewer
{
class FindBarWebEngineView;
}

namespace Akregator
{
class AKREGATOR_EXPORT ArticleViewerWebEngineWidgetNg : public QWidget
{
    Q_OBJECT
public:
    explicit ArticleViewerWebEngineWidgetNg(ArticleViewerWebEngine *customViewer, KActionCollection *ac, QWidget *parent);
    explicit ArticleViewerWebEngineWidgetNg(KActionCollection *ac, QWidget *parent = Q_NULLPTR);
    ~ArticleViewerWebEngineWidgetNg();

    ArticleViewerWebEngine *articleViewerNg() const;

    void saveCurrentPosition();
    void restoreCurrentPosition();
public Q_SLOTS:
    void slotSpeakText();
    void slotFind();
    void slotPrint();
    void slotPrintPreview();

private Q_SLOTS:
    void slotPdfFailed();
    void slotPdfCreated(const QString &filename);
    void slotOpenPrintPreviewDialog();
    void slotOpenInBrowser();
    void slotExportHtmlPageSuccess(const QString &filename);
    void slotExportHtmlPageFailed();
private:
    void initializeActions(KActionCollection *ac);
    void initializeLayout(KActionCollection *ac);
    WebEngineViewer::FindBarWebEngineView *mFindBarWebView;
    ArticleViewerWebEngine *mArticleViewerNg;
    KPIMTextEdit::TextToSpeechWidget *mTextToSpeechWidget;
    KPIMTextEdit::SlideContainer *mSliderContainer;
};
}
#endif // ARTICLEVIEWERWEBENGINEWIDGETNG_H
