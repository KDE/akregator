/*
   Copyright (c) 2015-2020 Laurent Montel <montel@kde.org>

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

#ifndef ARTICLEVIEWERWEBENGINE_H
#define ARTICLEVIEWERWEBENGINE_H

#include <WebEngineViewer/WebEngineView>
#include <openurlrequest.h>
#include <shareserviceurlmanager.h>
#include <MessageViewer/ViewerPluginInterface>
#include <WebEngineViewer/CheckPhishingUrlJob>

class KActionCollection;
namespace WebEngineViewer {
class WebHitTestResult;
class WebEngineAccessKey;
class InterceptorManager;
}
namespace MessageViewer {
class ViewerPluginToolManager;
class ViewerPluginInterface;
}
namespace PimCommon {
class ShareServiceUrlManager;
}
namespace KIO {
class KUriFilterSearchProviderActions;
}
namespace Akregator {
class ArticleViewerWebEnginePage;
class AKREGATOR_EXPORT ArticleViewerWebEngine : public WebEngineViewer::WebEngineView
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
        OpenInBackgroundTab
    };
    explicit ArticleViewerWebEngine(KActionCollection *ac, QWidget *parent);
    ~ArticleViewerWebEngine() override;

    void showAboutPage();

    void disableIntroduction();
    void setArticleAction(ArticleViewerWebEngine::ArticleAction type, const QString &articleId, const QString &feed);
    void restoreCurrentPosition();

    void createViewerPluginToolManager(KActionCollection *ac, QWidget *parent);

    void execPrintPreviewPage(QPrinter *printer, int timeout);
protected:
    QUrl mCurrentUrl;
    KActionCollection *mActionCollection = nullptr;
    PimCommon::ShareServiceUrlManager *mShareServiceManager = nullptr;
    KIO::KUriFilterSearchProviderActions *mWebShortcutMenuManager = nullptr;

private:
    enum MousePressedButtonType {
        RightButton = 0,
        LeftButton,
        MiddleButton,
    };

    void paintAboutScreen(const QString &templateName, const QVariantHash &data);
    QVariantHash introductionData() const;

public Q_SLOTS:
    void slotCopy();
    void slotSaveLinkAs();
    void slotCopyLinkAddress();
    void slotSaveImageOnDiskInFrame();
    void slotCopyImageLocationInFrame();
    void slotMute(bool mute);
Q_SIGNALS:
    void signalOpenUrlRequest(Akregator::OpenUrlRequest &);
    void showStatusBarMessage(const QString &link);
    void showContextMenu(const QPoint &pos);
    void articleAction(Akregator::ArticleViewerWebEngine::ArticleAction type, const QString &articleId, const QString &feed);
    void findTextInHtml();
    void textToSpeech();
    void webPageMutedOrAudibleChanged(bool isAudioMuted, bool wasRecentlyAudible);

protected:
    void resizeEvent(QResizeEvent *e) override;

    virtual void displayContextMenu(const QPoint &pos);
    void forwardKeyReleaseEvent(QKeyEvent *e) override;
    void forwardKeyPressEvent(QKeyEvent *e) override;
    void forwardWheelEvent(QWheelEvent *e) override;
    void forwardMouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private Q_SLOTS:
    void slotServiceUrlSelected(PimCommon::ShareServiceUrlManager::ServiceType type);
    void slotLinkHovered(const QString &link);
    void slotLoadStarted();
    void slotLoadFinished();
    void slotLinkClicked(const QUrl &url);
    void slotShowContextMenu(const QPoint &pos);
    void slotWebHitFinished(const WebEngineViewer::WebHitTestResult &result);
    void slotActivatePlugin(MessageViewer::ViewerPluginInterface *interface);
    void slotWebPageMutedOrAudibleChanged();
    void slotCheckedUrlFinished(const QUrl &url, WebEngineViewer::CheckPhishingUrlUtil::UrlStatus status);
protected:

    void slotOpenLinkInBrowser();
    void slotOpenLinkInForegroundTab();
    void slotOpenLinkInBackgroundTab();
    ArticleViewerWebEnginePage *mPageEngine = nullptr;
    QList<QAction *> viewerPluginActionList(MessageViewer::ViewerPluginInterface::SpecificFeatureTypes features);
    WebEngineViewer::InterceptorManager *mNetworkAccessManager = nullptr;
private:
    void openSafeUrl(const QUrl &url);
    bool urlIsAMalwareButContinue();
    MousePressedButtonType mLastButtonClicked;
    MessageViewer::ViewerPluginToolManager *mViewerPluginToolManager = nullptr;
    WebEngineViewer::WebEngineAccessKey *mWebEngineViewAccessKey = nullptr;
};
}

#endif // ARTICLEVIEWERWEBENGINE_H
