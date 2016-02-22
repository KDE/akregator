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

#ifndef ARTICLEVIEWERWEBENGINE_H
#define ARTICLEVIEWERWEBENGINE_H

#include <QWebEngineView>
#include <openurlrequest.h>
#include <shareserviceurlmanager.h>

class KActionCollection;
namespace MessageViewer
{
class WebHitTestResult;
}
namespace PimCommon
{
class ShareServiceUrlManager;
}
namespace KIO
{
class KUriFilterSearchProviderActions;
}
namespace Akregator
{
class AKREGATOR_EXPORT ArticleViewerWebEngine : public QWebEngineView
{
    Q_OBJECT
public:
    enum ArticleAction {
        DeleteAction = 0,
        MarkAsRead,
        MarkAsUnRead,
        MarkAsImportant,
        SendUrlArticle,
        SendFileArticle,
        OpenInExternalBrowser,
        Share,
        OpenInBackgroundTab
    };
    explicit ArticleViewerWebEngine(KActionCollection *ac, QWidget *parent);
    ~ArticleViewerWebEngine();

    void showAboutPage();

    void disableIntroduction();
    void setArticleAction(ArticleViewerWebEngine::ArticleAction type, const QString &articleId, const QString &feed);

    bool zoomTextOnlyInFrame() const;

protected:
    QUrl mCurrentUrl;
    KActionCollection *mActionCollection;
    PimCommon::ShareServiceUrlManager *mShareServiceManager;
    KIO::KUriFilterSearchProviderActions *mWebShortcutMenuManager;

private:
    enum MousePressedButtonType {
        RightButton = 0,
        LeftButton,
        MiddleButton,
    };

    void paintAboutScreen(const QString &templateName, const QVariantHash &data);
    QVariantHash introductionData() const;

public Q_SLOTS:
    void slotPrintPreview();
    void slotPrint();
    void slotCopy();
    void slotZoomTextOnlyInFrame(bool textOnlyInFrame);
    void slotSaveLinkAs();
    void slotCopyLinkAddress();
    void slotSaveImageOnDiskInFrame();
    void slotCopyImageLocationInFrame();
    void slotBlockImage();
Q_SIGNALS:
    void signalOpenUrlRequest(Akregator::OpenUrlRequest &);
    void showStatusBarMessage(const QString &link);
    void showContextMenu(const QPoint &pos);
    void articleAction(Akregator::ArticleViewerWebEngine::ArticleAction type, const QString &articleId, const QString &feed);
    void findTextInHtml();
    void textToSpeech();

protected:
    void keyReleaseEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    virtual void displayContextMenu(const QPoint &pos);

private Q_SLOTS:
    void slotServiceUrlSelected(PimCommon::ShareServiceUrlManager::ServiceType type);
    void slotLinkHovered(const QString &link);
    void slotLoadStarted();
    void slotLoadFinished();
    void slotLinkClicked(const QUrl &url);
    void slotOpenLinkInForegroundTab();
    void slotOpenLinkInBackgroundTab();
    void slotOpenLinkInBrowser();
    void slotShowContextMenu(const QPoint &pos);
protected:
    MessageViewer::WebHitTestResult *mContextMenuHitResult;

    bool adblockEnabled() const;

    void contextMenuEvent(QContextMenuEvent *e) Q_DECL_OVERRIDE;
private:
    QUrl linkOrImageUrlAt(const QPoint &global) const;
    MousePressedButtonType mLastButtonClicked;
    //MessageViewer::WebViewAccessKey *mWebViewAccessKey;
};
}

#endif // ARTICLEVIEWERWEBENGINE_H
