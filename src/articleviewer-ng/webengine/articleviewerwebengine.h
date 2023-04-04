/*
   SPDX-FileCopyrightText: 2015-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <MessageViewer/ViewerPluginInterface>
#include <PimCommon/ShareServiceUrlManager>
#include <WebEngineViewer/CheckPhishingUrlJob>
#include <WebEngineViewer/WebEngineView>
#include <openurlrequest.h>

class KActionCollection;
namespace WebEngineViewer
{
class WebHitTestResult;
class WebEngineAccessKey;
class InterceptorManager;
class LoadExternalReferencesUrlInterceptor;
class BlockExternalResourcesUrlInterceptor;
}
namespace MessageViewer
{
class ViewerPluginToolManager;
class ViewerPluginInterface;
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

    virtual void updateSecurity();

    void printPreviewPage(QPrinter *printer);

protected:
    QUrl mCurrentUrl;
    KActionCollection *const mActionCollection;
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
    void trackingFound(const WebEngineViewer::BlockTrackingUrlInterceptor::TrackerBlackList &);

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
    WebEngineViewer::LoadExternalReferencesUrlInterceptor *mExternalReference = nullptr;
    WebEngineViewer::BlockExternalResourcesUrlInterceptor *mBlockExternalReference = nullptr;

private:
    void openSafeUrl(const QUrl &url);
    bool urlIsAMalwareButContinue();
    MousePressedButtonType mLastButtonClicked = LeftButton;
    MessageViewer::ViewerPluginToolManager *mViewerPluginToolManager = nullptr;
    WebEngineViewer::WebEngineAccessKey *mWebEngineViewAccessKey = nullptr;
};
}
