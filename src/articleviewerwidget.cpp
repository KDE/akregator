/*
    This file is part of Akregator.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "articleviewerwidget.h"

#include "articleviewer-ng/articlehtmlwriter.h"

#include "akregatorconfig.h"
#include "aboutdata.h"
#include "actionmanager.h"
#include "actions.h"
#include "article.h"
#include "articleformatter.h"
#include "articlejobs.h"
#include "articlematcher.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "utils.h"
#include "openurlrequest.h"
#include "akregator_debug.h"
#include "kdepim-version.h"

#include <QAction>
#include <kactioncollection.h>
#include <KLocalizedString>
#include <QMenu>
#include <kmessagebox.h>
#include <krun.h>
#include <kshell.h>

#include <kstandardaction.h>
#include <ktoolinvocation.h>
#include <kglobalsettings.h>
#include <kio/job.h>
#include <QUrl>
#include <grantleetheme/grantleethememanager.h>
#include <articleviewer-ng/articleviewerwidgetng.h>
#include <articleviewer-ng/articleviewerng.h>

#include <QClipboard>
#include <QGridLayout>
#include <QKeyEvent>
#include <QApplication>

#include <QStandardPaths>

using namespace Akregator;
using namespace Akregator::Filters;

namespace Akregator
{

ArticleViewerWidget::ArticleViewerWidget(KActionCollection *ac, QWidget *parent)
    : QWidget(parent),
      m_imageDir(QUrl::fromLocalFile(QString(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/akregator/Media/")))),
      m_node(0),
      m_viewMode(NormalView),
      m_articleViewerWidgetNg(new Akregator::ArticleViewerWidgetNg(ac, this)),
      m_normalViewFormatter(new DefaultNormalViewFormatter(m_imageDir, m_articleViewerWidgetNg->articleViewerNg())),
      m_combinedViewFormatter(new DefaultCombinedViewFormatter(m_imageDir, m_articleViewerWidgetNg->articleViewerNg()))
{
    QGridLayout *layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_articleViewerWidgetNg);

    m_articleHtmlWriter = new Akregator::ArticleHtmlWriter(m_articleViewerWidgetNg->articleViewerNg(), this);
    connect(m_articleViewerWidgetNg->articleViewerNg(), &ArticleViewerNg::signalOpenUrlRequest, this, &ArticleViewerWidget::signalOpenUrlRequest);
    updateCss();
#if 0
    layout->addWidget(m_part->widget(), 0, 0);

    setFocusProxy(m_part->widget());

    m_part->setFontScaleFactor(100);
    m_part->setZoomFactor(100);
    m_part->setJScriptEnabled(false);
    m_part->setJavaEnabled(false);
    m_part->setMetaRefreshEnabled(false);
    m_part->setPluginsEnabled(false);
    m_part->setDNDEnabled(true);
    m_part->setAutoloadImages(true);
    m_part->setStatusMessagesEnabled(false);
    m_part->view()->setAttribute(Qt::WA_InputMethodEnabled, true); //workaround to fix 216878
    m_part->view()->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    // change the cursor when loading stuff...
    connect(m_part, &KParts::ReadOnlyPart::started,
            this, &ArticleViewerWidget::slotStarted);
    connect(m_part, SIGNAL(completed()),
            this, SLOT(slotCompleted()));

    KParts::BrowserExtension *ext = m_part->browserExtension();
    connect(ext, SIGNAL(popupMenu(QPoint,QUrl,mode_t,KParts::OpenUrlArguments,KParts::BrowserArguments,KParts::BrowserExtension::PopupFlags,KParts::BrowserExtension::ActionGroupMap)),
            this, SLOT(slotPopupMenu(QPoint,QUrl,mode_t,KParts::OpenUrlArguments,KParts::BrowserArguments,KParts::BrowserExtension::PopupFlags))); // ActionGroupMap argument removed, unused by slot

    connect(ext, &KParts::BrowserExtension::openUrlRequestDelayed,
            this, &ArticleViewerWidget::slotOpenUrlRequestDelayed);

    connect(ext, &KParts::BrowserExtension::createNewWindow,
            this, &ArticleViewerWidget::slotCreateNewWindow);

    QAction *action = 0;

    action = m_part->actionCollection()->addAction(QStringLiteral("copylinkaddress"));
    action->setText(i18n("Copy &Link Address"));
    connect(action, &QAction::triggered, this, &ArticleViewerWidget::slotCopyLinkAddress);

    action = m_part->actionCollection()->addAction(QStringLiteral("savelinkas"));
    action->setText(i18n("&Save Link As..."));
    connect(action, &QAction::triggered, this, &ArticleViewerWidget::slotSaveLinkAs);

    updateCss();

    connect(this, &ArticleViewerWidget::selectionChanged, this, &ArticleViewerWidget::slotSelectionChanged);

    connect(KGlobalSettings::self(), &KGlobalSettings::kdisplayPaletteChanged, this, &ArticleViewerWidget::slotPaletteOrFontChanged);
    connect(KGlobalSettings::self(), &KGlobalSettings::kdisplayFontChanged, this, &ArticleViewerWidget::slotPaletteOrFontChanged);
#endif

    QAction *action = ac->addAction(QStringLiteral("copylinkaddress"));
    action->setText(i18n("Copy &Link Address"));
    connect(action, &QAction::triggered, this, &ArticleViewerWidget::slotCopyLinkAddress);

    action = ac->addAction(QStringLiteral("savelinkas"));
    action->setText(i18n("&Save Link As..."));
    connect(action, &QAction::triggered, this, &ArticleViewerWidget::slotSaveLinkAs);


    m_htmlFooter = QStringLiteral("</body></html>");
}

ArticleViewerWidget::~ArticleViewerWidget()
{
}

int ArticleViewerWidget::pointsToPixel(int pointSize) const
{
    return (pointSize * m_articleViewerWidgetNg->articleViewerNg()->logicalDpiY() + 36) / 72;
}
#if 0
void ArticleViewerWidget::slotOpenUrlRequestDelayed(const QUrl &url, const KParts::OpenUrlArguments &args, const KParts::BrowserArguments &browserArgs)
{
    OpenUrlRequest req(url);
    req.setArgs(args);
    req.setBrowserArgs(browserArgs);
    if (req.options() == OpenUrlRequest::None) {              // no explicit new window,
        req.setOptions(OpenUrlRequest::NewTab);    // so must open new tab
    }

    if (m_part->button() == Qt::LeftButton) {
        switch (Settings::lMBBehaviour()) {
        case Settings::EnumLMBBehaviour::OpenInExternalBrowser:
            req.setOptions(OpenUrlRequest::ExternalBrowser);
            break;
        case Settings::EnumLMBBehaviour::OpenInBackground:
            req.setOpenInBackground(true);
            break;
        default:
            break;
        }
    } else if (m_part->button() == Qt::MidButton) {
        switch (Settings::mMBBehaviour()) {
        case Settings::EnumMMBBehaviour::OpenInExternalBrowser:
            req.setOptions(OpenUrlRequest::ExternalBrowser);
            break;
        case Settings::EnumMMBBehaviour::OpenInBackground:
            req.setOpenInBackground(true);
            break;
        default:
            break;
        }
    }

    Q_EMIT signalOpenUrlRequest(req);
}

void ArticleViewerWidget::slotCreateNewWindow(const QUrl &url,
                                        const KParts::OpenUrlArguments &args,
                                        const KParts::BrowserArguments &browserArgs,
                                        const KParts::WindowArgs & /*windowArgs*/,
                                        KParts::ReadOnlyPart **part)
{
    OpenUrlRequest req;
    req.setUrl(url);
    req.setArgs(args);
    req.setBrowserArgs(browserArgs);
    req.setOptions(OpenUrlRequest::NewTab);

    Q_EMIT signalOpenUrlRequest(req);
    if (part) {
        *part = req.part();
    }
}
#endif

// taken from KDevelop
void ArticleViewerWidget::slotCopy()
{
#if 0
    QString text = m_part->selectedText();
    text.replace(QChar(0xa0), QLatin1Char(' '));
    QClipboard *const cb = QApplication::clipboard();
    Q_ASSERT(cb);
    cb->setText(text, QClipboard::Clipboard);
#endif
}

void ArticleViewerWidget::slotCopyLinkAddress()
{
    if (m_url.isEmpty()) {
        return;
    }
    QClipboard *cb = QApplication::clipboard();
    cb->setText(m_url.toString(), QClipboard::Clipboard);
    // don't set url to selection as it's a no-no according to a fd.o spec
    // which spec? Nobody seems to care (tested Firefox (3.5.10) Konqueror,and KMail (4.2.3)), so I re-enable the following line unless someone gives
    // a good reason to remove it again (bug 183022) --Frank
    cb->setText(m_url.toString(), QClipboard::Selection);
}

void ArticleViewerWidget::slotSelectionChanged()
{
    ActionManager::getInstance()->action(QStringLiteral("viewer_copy"))->setEnabled(!m_articleViewerWidgetNg->articleViewerNg()->selectedText().isEmpty());
}

void ArticleViewerWidget::slotOpenLinkInternal()
{
    openUrl(m_url);
}

void ArticleViewerWidget::slotOpenLinkInForegroundTab()
{
    OpenUrlRequest req(m_url);
    req.setOptions(OpenUrlRequest::NewTab);
    Q_EMIT signalOpenUrlRequest(req);
}

void ArticleViewerWidget::slotOpenLinkInBackgroundTab()
{
    OpenUrlRequest req(m_url);
    req.setOptions(OpenUrlRequest::NewTab);
    req.setOpenInBackground(true);
    Q_EMIT signalOpenUrlRequest(req);
}

void ArticleViewerWidget::slotOpenLinkInBrowser()
{
    OpenUrlRequest req(m_url);
    req.setOptions(OpenUrlRequest::ExternalBrowser);
    Q_EMIT signalOpenUrlRequest(req);
}

void ArticleViewerWidget::slotSaveLinkAs()
{
    QUrl tmp(m_url);

    if (tmp.fileName().isEmpty()) {
        tmp = tmp.adjusted(QUrl::RemoveFilename);
        tmp.setPath(tmp.path() + QLatin1String("index.html"));
    }
    //TODO
    //KParts::BrowserRun::simpleSave(tmp, tmp.fileName());
}

void ArticleViewerWidget::slotZoomIn(int id)
{
    #if 0 //TODO
    if (id != 0) {
        return;
    }

    int zf = m_part->fontScaleFactor();
    if (zf < 100) {
        zf = zf - (zf % 20) + 20;
        m_part->setFontScaleFactor(zf);
    } else {
        zf = zf - (zf % 50) + 50;
        m_part->setFontScaleFactor(zf < 300 ? zf : 300);
    }
#endif
}

void ArticleViewerWidget::slotZoomOut(int id)
{
    #if 0 //TODO
    if (id != 0) {
        return;
    }

    int zf = m_part->fontScaleFactor();
    if (zf <= 100) {
        zf = zf - (zf % 20) - 20;
        m_part->setFontScaleFactor(zf > 20 ? zf : 20);
    } else {
        zf = zf - (zf % 50) - 50;
        m_part->setFontScaleFactor(zf);
    }
#endif
}

void ArticleViewerWidget::slotSetZoomFactor(int percent)
{
    #if 0 //TODO
    m_part->setFontScaleFactor(percent);
#endif
}

// some code taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void ArticleViewerWidget::slotPrint()
{
    #if 0 //TODO
    m_part->view()->print();
#endif
}

void ArticleViewerWidget::connectToNode(TreeNode *node)
{
    if (node) {
        if (m_viewMode == CombinedView) {
            connect(node, &TreeNode::signalChanged, this, &ArticleViewerWidget::slotUpdateCombinedView);
            connect(node, &TreeNode::signalArticlesAdded, this, &ArticleViewerWidget::slotArticlesAdded);
            connect(node, &TreeNode::signalArticlesRemoved, this, &ArticleViewerWidget::slotArticlesRemoved);
            connect(node, &TreeNode::signalArticlesUpdated, this, &ArticleViewerWidget::slotArticlesUpdated);
        } else if (m_viewMode == SummaryView) {
            connect(node, &TreeNode::signalChanged, this, &ArticleViewerWidget::slotShowSummary);
        }

        connect(node, &TreeNode::signalDestroyed, this, &ArticleViewerWidget::slotClear);
    }
}

void ArticleViewerWidget::disconnectFromNode(TreeNode *node)
{
    if (node) {
        node->disconnect(this);
    }
}

void ArticleViewerWidget::renderContent(const QString &text)
{
    m_currentText = text;
    beginWriting();
    m_articleHtmlWriter->queue(text);
    endWriting();
}

void ArticleViewerWidget::beginWriting()
{
    m_articleHtmlWriter->begin();
    QString head = QStringLiteral("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n <html><head><title>.</title>");
    if (m_viewMode == CombinedView) {
        head += m_combinedModeCSS;
    } else {
        head += m_normalModeCSS;
    }

    head += QLatin1String("</style></head><body>");
    //m_part->view()->setContentsPos(0, 0);

    //pass link to the KHTMLPart to make relative links work
    //add a bogus query item to distinguish from m_link
    //fixes the Complete Story link if the url has an anchor (e.g. #reqRSS) in it
    //See bug 177754

    QUrl url(m_link);
    url.addQueryItem(QStringLiteral("akregatorPreviewMode"), QStringLiteral("true"));
    m_articleHtmlWriter->queue(head);
}

void ArticleViewerWidget::endWriting()
{
    m_articleHtmlWriter->queue(m_htmlFooter);
    m_articleHtmlWriter->end();
}

void ArticleViewerWidget::slotShowSummary(TreeNode *node)
{
    m_viewMode = SummaryView;

    if (!node) {
        slotClear();
        return;
    }

    if (node != m_node) {
        disconnectFromNode(m_node);
        connectToNode(node);
        m_node = node;
    }

    QString summary = m_normalViewFormatter->formatSummary(node);
    m_link.clear();
    renderContent(summary);

    setArticleActionsEnabled(false);
}

void ArticleViewerWidget::showArticle(const Akregator::Article &article)
{
    if (article.isNull() || article.isDeleted()) {
        slotClear();
        return;
    }

    m_viewMode = NormalView;
    disconnectFromNode(m_node);
    m_article = article;
    m_node = 0;
    m_link = article.link();
    if (article.feed()->loadLinkedWebsite()) {
        openUrl(article.link());
    } else {
        renderContent(m_normalViewFormatter->formatArticle(article, ArticleFormatter::ShowIcon));
    }

    setArticleActionsEnabled(true);
}

bool ArticleViewerWidget::openUrl(const QUrl &url)
{
#if 0
    if (!m_article.isNull() && m_article.feed()->loadLinkedWebsite()) {
        return m_part->openUrl(url);
    } else {
        reload();
        return true;
    }
#else
    return false;
#endif
}

void ArticleViewerWidget::setFilters(const std::vector< QSharedPointer<const AbstractMatcher> > &filters)
{
    if (filters == m_filters) {
        return;
    }

    m_filters = filters;

    slotUpdateCombinedView();
}

void ArticleViewerWidget::slotUpdateCombinedView()
{
    if (m_viewMode != CombinedView) {
        return;
    }

    if (!m_node) {
        return slotClear();
    }

    QString text;

    int num = 0;
    QTime spent;
    spent.start();

    const std::vector< QSharedPointer<const AbstractMatcher> >::const_iterator filterEnd = m_filters.cend();

    Q_FOREACH (const Article &i, m_articles) {
        if (i.isDeleted()) {
            continue;
        }

        auto func = [i](const QSharedPointer<const Filters::AbstractMatcher> &matcher) -> bool { return !matcher->matches(i); };
        if (std::find_if(m_filters.cbegin(), filterEnd, func) != filterEnd) {
            continue;
        }

        text += QLatin1String("<p><div class=\"article\">") + m_combinedViewFormatter->formatArticle(i, ArticleFormatter::NoIcon) + QLatin1String("</div><p>");
        ++num;
    }

    qCDebug(AKREGATOR_LOG) << "Combined view rendering: (" << num << " articles):" << "generating HTML:" << spent.elapsed() << "ms";
    renderContent(text);
    qCDebug(AKREGATOR_LOG) << "HTML rendering:" << spent.elapsed() << "ms";
}

void ArticleViewerWidget::slotArticlesUpdated(TreeNode * /*node*/, const QVector<Article> & /*list*/)
{
    if (m_viewMode == CombinedView) {
        //TODO
        slotUpdateCombinedView();
    }
}

void ArticleViewerWidget::slotArticlesAdded(TreeNode * /*node*/, const QVector<Article> &list)
{
    if (m_viewMode == CombinedView) {
        //TODO sort list, then merge
        m_articles << list;
        std::sort(m_articles.begin(), m_articles.end());
        slotUpdateCombinedView();
    }
}

void ArticleViewerWidget::slotArticlesRemoved(TreeNode * /*node*/, const QVector<Article> &list)
{
    Q_UNUSED(list)

    if (m_viewMode == CombinedView) {
        //TODO
        slotUpdateCombinedView();
    }
}

void ArticleViewerWidget::slotClear()
{
    disconnectFromNode(m_node);
    m_node = 0;
    m_article = Article();
    m_articles.clear();

    renderContent(QString());
}

void ArticleViewerWidget::showNode(TreeNode *node)
{
    m_viewMode = CombinedView;

    if (node != m_node) {
        disconnectFromNode(m_node);
    }

    connectToNode(node);

    m_articles.clear();
    m_article = Article();
    m_node = node;

    delete m_listJob;

    m_listJob = node->createListJob();
    connect(m_listJob.data(), &ArticleListJob::finished, this, &ArticleViewerWidget::slotArticlesListed);
    m_listJob->start();

    slotUpdateCombinedView();
}

void ArticleViewerWidget::slotArticlesListed(KJob *job)
{
    Q_ASSERT(job);
    Q_ASSERT(job == m_listJob);

    TreeNode *node = m_listJob->node();

    if (job->error() || !node) {
        if (!node) {
            qCWarning(AKREGATOR_LOG) << "Node to be listed is already deleted";
        } else {
            qCWarning(AKREGATOR_LOG) << job->errorText();
        }
        slotUpdateCombinedView();
        return;
    }

    m_articles = m_listJob->articles();
    std::sort(m_articles.begin(), m_articles.end());

    if (node && !m_articles.isEmpty()) {
        m_link = m_articles.first().link();
    } else {
        m_link = QUrl();
    }

    slotUpdateCombinedView();
}

void ArticleViewerWidget::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}

void ArticleViewerWidget::slotPaletteOrFontChanged()
{
    updateCss();
    reload();
}

void ArticleViewerWidget::reload()
{
#if 0
    beginWriting();
    m_part->write(m_currentText);
    endWriting();
#endif
}

QSize ArticleViewerWidget::sizeHint() const
{
    // Increase height a bit so that we can (roughly) read 25 lines of text
    QSize sh = QWidget::sizeHint();
    sh.setHeight(qMax(sh.height(), 25 * fontMetrics().height()));
    return sh;
}


void ArticleViewerWidget::displayAboutPage()
{
    m_articleViewerWidgetNg->articleViewerNg()->showAboutPage();
#if 0
    QString location = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("akregator/about/main.html"));

    m_part->begin(QUrl::fromLocalFile(location));
    QString info =
        i18nc("%1: Akregator version; %2: homepage URL; "
              "--- end of comment ---",
              "<h2 style='margin-top: 0px;'>Welcome to Akregator %1</h2>"
              "<p>Akregator is a KDE news feed reader. "
              "Feed readers provide a convenient way to browse different kinds of "
              "content, including news, blogs, and other content from online sites. "
              "Instead of checking all your favorite web sites manually for updates, "
              "Akregator collects the content for you.</p>"
              "<p>For more information about using Akregator, check the "
              "<a href=\"%2\">Akregator website</a>. If you do not want to see this page "
              "anymore, <a href=\"config:/disable_introduction\">click here</a>.</p>"
              "<p>We hope that you will enjoy Akregator.</p>\n"
              "<p>Thank you,</p>\n"
              "<p style='margin-bottom: 0px'>&nbsp; &nbsp; The Akregator Team</p>\n",
              QLatin1String(KDEPIM_VERSION),
              QStringLiteral("http://akregator.kde.org/")); // Akregator homepage URL

    QString fontSize = QString::number(pointsToPixel(Settings::mediumFontSize()));
    QString appTitle = i18n("Akregator");
    QString catchPhrase; //not enough space for a catch phrase at default window size i18n("Part of the Kontact Suite");
    QString quickDescription = i18n("A KDE news feed reader.");

    QFile f(location);
    if (!f.open(QIODevice::ReadOnly)) {
        qCWarning(AKREGATOR_LOG) << "Cannot load about page: " << f.errorString();
        m_part->end();
        return;
    }
    QString content = QString::fromLocal8Bit(f.readAll());
    f.close();

    QString infocss = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kf5/infopage/kde_infopage.css"));
    QString rtl = (QApplication::layoutDirection() == Qt::RightToLeft)
                  ? QStringLiteral("@import \"%1\";").arg(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kf5/infopage/kde_infopage_rtl.css")))
                  : QString();

    m_part->write(content.arg(infocss, rtl, fontSize, appTitle, catchPhrase, quickDescription, info));
    m_part->end();

#endif
}

void ArticleViewerWidget::updateCss()
{
    m_normalModeCSS =  m_normalViewFormatter->getCss();
    m_combinedModeCSS = m_combinedViewFormatter->getCss();
}

void ArticleViewerWidget::setArticleActionsEnabled(bool enabled)
{
    ActionManager::getInstance()->setArticleActionsEnabled(enabled);
}

} // namespace Akregator

