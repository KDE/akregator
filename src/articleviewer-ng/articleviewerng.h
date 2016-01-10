/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

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

#ifndef ARTICLEVIEWERNG_H
#define ARTICLEVIEWERNG_H

#include <KWebView>
#include "akregator_export.h"
#include <QVariantHash>
#include <QWebHitTestResult>
#include <openurlrequest.h>
class KActionCollection;
namespace MessageViewer
{
class WebViewAccessKey;
}
namespace Akregator
{
class AKREGATOR_EXPORT ArticleViewerNg : public KWebView
{
    Q_OBJECT
public:
    enum ArticleAction {
        DeleteAction = 0,
        MarkAsRead,
        MarkAsUnRead,
        MarkAsImportant,
        SendUrlArticle,
        OpenInExternalBrowser,
        Share
    };

    explicit ArticleViewerNg(KActionCollection *ac, QWidget *parent = Q_NULLPTR);
    ~ArticleViewerNg();

    void showAboutPage();

    bool canGoForward() const;
    bool canGoBack() const;

    void disableIntroduction();

    void setArticleAction(ArticleViewerNg::ArticleAction type, const QString &articleId);

public Q_SLOTS:
    void slotPrintPreview();
    void slotPrint();
    void slotCopy();
    void slotZoomTextOnlyInFrame(bool textOnlyInFrame);
Q_SIGNALS:
    void signalOpenUrlRequest(Akregator::OpenUrlRequest &);
    void showStatusBarMessage(const QString &link);

protected:
    void keyReleaseEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotLinkHovered(const QString &link, const QString &title, const QString &textContent);
    void slotLoadStarted();
    void slotLoadFinished();
    void slotLinkClicked(const QUrl &url);
    void slotOpenLinkInForegroundTab();
    void slotOpenLinkInBackgroundTab();
    void slotOpenLinkInBrowser();

protected:
    QUrl mCurrentUrl;
    QWebHitTestResult mContextMenuHitResult;
    KActionCollection *mActionCollection;

private:
    void paintAboutScreen(const QString &templateName, const QVariantHash &data);
    QUrl linkOrImageUrlAt(const QPoint &global) const;
    QVariantHash introductionData();
    MessageViewer::WebViewAccessKey *mWebViewAccessKey;
};
}

#endif // ARTICLEVIEWERNG_H
