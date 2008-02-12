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

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <khtmlview.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kshell.h>
#include <kstandarddirs.h>
#include <kstandardaction.h>
#include <ktoolinvocation.h>
#include <kurl.h>
#include <kglobalsettings.h>
#include <kparts/browserextension.h>
#include <kparts/browserrun.h>

#include <QClipboard>
#include <QKeySequence>
#include <QGridLayout>

#include <kpimutils/kfileio.h>

#include "akregatorconfig.h"
#include "openurlrequest.h"

// TODO: remove unneeded includes
#include "aboutdata.h"
#include "article.h"
#include "articleformatter.h"
#include "articleviewer.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "utils.h"

namespace Akregator {

ArticleViewer::ArticleViewer(QWidget *parent)
    : QWidget(parent), m_url(0), m_htmlFooter(), m_currentText(), m_node(0),
      m_viewMode(NormalView)
{
    m_part = new ArticleViewerPart(this);
    QGridLayout* layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_part->widget(), 0, 0);

    m_part->setZoomFactor(100);
    m_part->setJScriptEnabled(false);
    m_part->setJavaEnabled(false);
    m_part->setMetaRefreshEnabled(false);
    m_part->setPluginsEnabled(false);
    m_part->setDNDEnabled(true);
    m_part->setAutoloadImages(true);
    m_part->setStatusMessagesEnabled(false);

    // change the cursor when loading stuff...
    connect( m_part, SIGNAL(started(KIO::Job *)),
             this, SLOT(slotStarted(KIO::Job *)));
    connect( m_part, SIGNAL(completed()),
             this, SLOT(slotCompleted()));

    KParts::BrowserExtension* ext = m_part->browserExtension();
    connect(ext, SIGNAL(popupMenu (QPoint, KUrl, mode_t, KParts::OpenUrlArguments, KParts::BrowserArguments, KParts::BrowserExtension::PopupFlags, KParts::BrowserExtension::ActionGroupMap)),
             this, SLOT(slotPopupMenu(QPoint, KUrl, mode_t, KParts::OpenUrlArguments, KParts::BrowserArguments, KParts::BrowserExtension::PopupFlags))); // ActionGroupMap argument removed, unused by slot

    connect( ext, SIGNAL(openUrlRequestDelayed(KUrl, KParts::OpenUrlArguments, KParts::BrowserArguments)),
             this, SLOT(slotOpenUrlRequestDelayed(KUrl, KParts::OpenUrlArguments, KParts::BrowserArguments)) );

    connect( ext, SIGNAL(createNewWindow(KUrl, KParts::OpenUrlArguments, KParts::BrowserArguments)),
             this, SLOT(slotCreateNewWindow(KUrl, KParts::OpenUrlArguments, KParts::BrowserArguments)) );

    connect(ext, SIGNAL(createNewWindow(KUrl,
            KParts::OpenUrlArguments,
            KParts::BrowserArguments
            KParts::WindowArgs,
            KParts::ReadOnlyPart**)),
            this, SLOT(slotCreateNewWindow(KUrl,
                         KParts::OpenUrlArguments,
                         KParts::BrowserArguments
                         KParts::WindowArgs,
                         KParts::ReadOnlyPart**)));

    QAction* action = 0;
    action = KStandardAction::print(this, SLOT(slotPrint()), m_part->actionCollection());
    m_part->actionCollection()->addAction("viewer_print", action);

    action = KStandardAction::copy(this, SLOT(slotCopy()), m_part->actionCollection());
    m_part->actionCollection()->addAction("viewer_copy", action);

    action = m_part->actionCollection()->addAction("incFontSizes");
    action->setIcon(KIcon("zoom-in"));
    action->setText(i18n("&Increase Font Sizes"));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotZoomIn()));
    action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_Plus ) );

    action = m_part->actionCollection()->addAction("decFontSizes");
    action->setIcon(KIcon("zoom-out"));
    action->setText(i18n("&Decrease Font Sizes"));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotZoomOut()));
    action->setShortcut( QKeySequence(  Qt::CTRL + Qt::Key_Minus ) );

    action = m_part->actionCollection()->addAction("copylinkaddress");
    action->setText(i18n("Copy &Link Address"));
    connect(action, SIGNAL(triggered(bool) ), SLOT(slotCopyLinkAddress()));

    action = m_part->actionCollection()->addAction("savelinkas");
    action->setText(i18n("&Save Link As..."));
    connect(action, SIGNAL(triggered(bool) ), SLOT(slotSaveLinkAs()));

    action = m_part->actionCollection()->addAction("articleviewer_scroll_up");
    action->setText(i18n("&Scroll Up"));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotScrollUp()));
    action->setShortcuts(KShortcut( "Up" ));

    action = m_part->actionCollection()->addAction("articleviewer_scroll_down");
    action->setText(i18n("&Scroll Down"));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotScrollDown()));
    action->setShortcuts(KShortcut( "Down" ));

    m_imageDir.setPath(KGlobal::dirs()->saveLocation("cache", "akregator/Media/"));

    setNormalViewFormatter(ArticleFormatterPtr(new DefaultNormalViewFormatter(m_imageDir)));
    setCombinedViewFormatter(ArticleFormatterPtr(new DefaultCombinedViewFormatter(m_imageDir)));

    updateCss();

    connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));

    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(slotPaletteOrFontChanged()) );
    connect(KGlobalSettings::self(), SIGNAL(kdisplayFontChanged()), this, SLOT(slotPaletteOrFontChanged()) );

    m_htmlFooter = "</body></html>";
}

ArticleViewer::~ArticleViewer()
{
}

KParts::ReadOnlyPart* ArticleViewer::part() const
{
    return m_part;
}

int ArticleViewer::pointsToPixel(int pointSize) const
{
    return ( pointSize * m_part->view()->logicalDpiY() + 36 ) / 72 ;
}

void ArticleViewer::slotOpenUrlRequestDelayed(const KUrl& url, const KParts::OpenUrlArguments& args, const KParts::BrowserArguments& browserArgs)
{
    OpenUrlRequest req(url);
    req.setArgs(args);
    req.setBrowserArgs(browserArgs);
    req.setOptions(OpenUrlRequest::NewTab);

    if (m_part->button() == Qt::LeftButton)
    {
        switch (Settings::lMBBehaviour())
        {
            case Settings::EnumLMBBehaviour::OpenInExternalBrowser:
                req.setOptions(OpenUrlRequest::ExternalBrowser);
                break;
            case Settings::EnumLMBBehaviour::OpenInBackground:
                req.setOpenInBackground(true);
                break;
            default:
                break;
        }
    }
    else if (m_part->button() == Qt::MidButton)
    {
        switch (Settings::mMBBehaviour())
        {
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

    emit signalOpenUrlRequest(req);
}

void ArticleViewer::slotCreateNewWindow(const KUrl& url, const KParts::OpenUrlArguments& args, const KParts::BrowserArguments& browserArgs)
{
    OpenUrlRequest req(url);
    req.setArgs(args);
    req.setBrowserArgs(browserArgs);
    req.setOptions(OpenUrlRequest::NewTab);

    emit signalOpenUrlRequest(req);
}

void ArticleViewer::slotCreateNewWindow(const KUrl& url,
                                       const KParts::OpenUrlArguments& args,
                                       const KParts::BrowserArguments& browserArgs,
                                       const KParts::WindowArgs& /*windowArgs*/,
                                       KParts::ReadOnlyPart** part)
{
    OpenUrlRequest req;
    req.setUrl(url);
    req.setArgs(args);
    req.setBrowserArgs(browserArgs);
    req.setOptions(OpenUrlRequest::NewTab);

    emit signalOpenUrlRequest(req);
    *part = req.part();
}

void ArticleViewer::slotPopupMenu(const QPoint& p, const KUrl& kurl, mode_t, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&, KParts::BrowserExtension::PopupFlags kpf)
{
    const bool isLink = (kpf & KParts::BrowserExtension::ShowNavigationItems) == 0; // ## why not use kpf & IsLink ?
    const bool isSelection = (kpf & KParts::BrowserExtension::ShowTextSelectionItems) != 0;

    QString url = kurl.url();

    m_url = url;
    KMenu popup;

    if (isLink && !isSelection)
    {
        popup.addAction(KIcon("tab-new"), i18n("Open Link in New &Tab"), this, SLOT(slotOpenLinkInForegroundTab()));
        popup.addAction(KIcon("window-new"), i18n("Open Link in External &Browser"), this, SLOT(slotOpenLinkInBrowser()));
        popup.addSeparator();
        popup.addAction( m_part->action("savelinkas") );
        popup.addAction( m_part->action("copylinkaddress") );
    }
    else
    {
        if (isSelection)
        {
            popup.addAction( m_part->action("viewer_copy") );
            popup.addSeparator();
        }
        popup.addAction( m_part->action("viewer_print") );
       //KAction *ac = action("setEncoding");
       //if (ac)
       //     ac->plug(&popup);
    }
    popup.exec(p);
}

// taken from KDevelop
void ArticleViewer::slotCopy()
{
    QString text = m_part->selectedText();
    text.replace( QChar( 0xa0 ), ' ' );
    QApplication::clipboard()->disconnect( this );
}

void ArticleViewer::slotCopyLinkAddress()
{
    if(m_url.isEmpty()) return;
    QClipboard *cb = QApplication::clipboard();
    cb->setText(m_url.prettyUrl(), QClipboard::Clipboard);
    // don't set url to selection as it's a no-no according to a fd.o spec
    //cb->setText(m_url.prettyUrl(), QClipboard::Selection);
}

void ArticleViewer::slotSelectionChanged()
{
    m_part->action("viewer_copy")->setEnabled(!m_part->selectedText().isEmpty());
}

void ArticleViewer::slotOpenLinkInternal()
{
    openUrl(m_url);
}

void ArticleViewer::slotOpenLinkInForegroundTab()
{
    OpenUrlRequest req(m_url);
    req.setOptions(OpenUrlRequest::NewTab);
    emit signalOpenUrlRequest(req);
}

void ArticleViewer::slotOpenLinkInBackgroundTab()
{
    OpenUrlRequest req(m_url);
    req.setOptions(OpenUrlRequest::NewTab);
    req.setOpenInBackground(true);
    emit signalOpenUrlRequest(req);
}

void ArticleViewer::slotOpenLinkInBrowser()
{
    OpenUrlRequest req(m_url);
    req.setOptions(OpenUrlRequest::ExternalBrowser);
    emit signalOpenUrlRequest(req);
}

void ArticleViewer::slotSaveLinkAs()
{
    KUrl tmp( m_url );

    if ( tmp.fileName(KUrl::ObeyTrailingSlash).isEmpty() )
        tmp.setFileName( "index.html" );
    KParts::BrowserRun::simpleSave(tmp, tmp.fileName());
}

void ArticleViewer::slotStarted(KIO::Job* job)
{
    m_part->widget()->setCursor( Qt::WaitCursor );
    emit started(job);
}

void ArticleViewer::slotCompleted()
{
    m_part->widget()->unsetCursor();
    emit completed();
}

void ArticleViewer::slotScrollUp()
{
    m_part->view()->scrollBy(0,-10);
}

void ArticleViewer::slotScrollDown()
{
    m_part->view()->scrollBy(0,10);
}

void ArticleViewer::slotZoomIn()
{
    int zf = m_part->zoomFactor();
    if (zf < 100)
    {
        zf = zf - (zf % 20) + 20;
        m_part->setZoomFactor(zf);
    }
    else
    {
        zf = zf - (zf % 50) + 50;
        m_part->setZoomFactor(zf < 300 ? zf : 300);
    }
}

void ArticleViewer::slotZoomOut()
{
    int zf = m_part->zoomFactor();
    if (zf <= 100)
    {
        zf = zf - (zf % 20) - 20;
        m_part->setZoomFactor(zf > 20 ? zf : 20);
    }
    else
    {
        zf = zf - (zf % 50) - 50;
        m_part->setZoomFactor(zf);
    }
}

void ArticleViewer::slotSetZoomFactor(int percent)
{
    m_part->setZoomFactor(percent);
}

// some code taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void ArticleViewer::slotPrint( )
{
    m_part->view()->print();
}



void ArticleViewer::connectToNode(TreeNode* node)
{
    if (node)
    {
        if (m_viewMode == CombinedView)
        {
            connect( node, SIGNAL(signalChanged(Akregator::TreeNode*)), this, SLOT(slotUpdateCombinedView() ) );
            connect( node, SIGNAL(signalArticlesAdded(Akregator::TreeNode*, QList<Akregator::Article>)), this, SLOT(slotArticlesAdded(Akregator::TreeNode*, QList<Akregator::Article>)));
            connect( node, SIGNAL(signalArticlesRemoved(Akregator::TreeNode*, QList<Akregator::Article>)), this, SLOT(slotArticlesRemoved(Akregator::TreeNode*, QList<Akregator::Article>)));
            connect( node, SIGNAL(signalArticlesUpdated(Akregator::TreeNode*, QList<Akregator::Article>)), this, SLOT(slotArticlesUpdated(Akregator::TreeNode*, QList<Akregator::Article>)));
        }
        if (m_viewMode == SummaryView)
            connect( node, SIGNAL(signalChanged(Akregator::TreeNode*)), this, SLOT(slotShowSummary(Akregator::TreeNode*) ) );

        connect( node, SIGNAL(signalDestroyed(Akregator::TreeNode*)), this, SLOT(slotClear() ) );
    }
}

void ArticleViewer::disconnectFromNode(TreeNode* node)
{
    if (node)
        node->disconnect( this );
}

void ArticleViewer::renderContent(const QString& text)
{
    m_part->closeUrl();
    m_currentText = text;
    beginWriting();
    //kDebug() << text;
    m_part->write(text);
    endWriting();
}

void ArticleViewer::beginWriting()
{
    QString head = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n <html><head><title>.</title>");

    if (m_viewMode == CombinedView)
        head += m_combinedModeCSS;
    else
        head += m_normalModeCSS;

    head += "</style></head><body>";
    m_part->view()->setContentsPos(0,0);
    m_part->begin(m_link);
    m_part->write(head);
}

void ArticleViewer::endWriting()
{
    m_part->write(m_htmlFooter);
    //kDebug() << m_htmlFooter;
    m_part->end();
}


void ArticleViewer::slotShowSummary(TreeNode* node)
{
    m_viewMode = SummaryView;

    if (!node)
    {
        slotClear();
        return;
    }

    if (node != m_node)
    {
        disconnectFromNode(m_node);
        connectToNode(node);
        m_node = node;
    }

    QString summary = m_normalViewFormatter->formatSummary(node);
    m_link = QString();
    renderContent(summary);
}

void ArticleViewer::showArticle( const Akregator::Article& article )
{
    m_viewMode = NormalView;
    disconnectFromNode(m_node);
    m_article = article;
    m_node = 0;
    m_link = article.link();
    if (article.feed()->loadLinkedWebsite())
    {
        openUrl(article.link());
    }
    else
    {
        renderContent( m_normalViewFormatter->formatArticle(article, ArticleFormatter::ShowIcon) );
    }
}

bool ArticleViewer::openUrl(const KUrl& url)
{
    if (!m_article.isNull() && m_article.feed()->loadLinkedWebsite())
    {
        return m_part->openUrl(url);
    }
    else
    {
        reload();
        return true;
    }
}

void ArticleViewer::slotSetFilter(const Akregator::Filters::ArticleMatcher& textFilter, const Akregator::Filters::ArticleMatcher& statusFilter)
{
    if (m_statusFilter == statusFilter && m_textFilter == textFilter)
        return;

    m_textFilter = textFilter;
    m_statusFilter = statusFilter;

    slotUpdateCombinedView();
}

void ArticleViewer::slotUpdateCombinedView()
{
    if (m_viewMode != CombinedView)
        return;

    if (!m_node)
        return slotClear();

    QList<Article> articles = m_node->articles();
    qSort(articles);
    QList<Article>::ConstIterator end = articles.end();
    QList<Article>::ConstIterator it = articles.begin();

    QString text;

    int num = 0;
    QTime spent;
    spent.start();

    for ( ; it != end; ++it)
    {
        if ( !(*it).isDeleted() && m_textFilter.matches(*it) && m_statusFilter.matches(*it) )
        {
            text += "<p><div class=\"article\">"+m_combinedViewFormatter->formatArticle(*it, ArticleFormatter::NoIcon)+"</div><p>";
            ++num;
        }
    }
    kDebug() <<"Combined view rendering: (" << num <<" articles):" <<"generating HTML:" << spent.elapsed() <<"ms";
    renderContent(text);
    kDebug() <<"HTML rendering:" << spent.elapsed() <<"ms";
}

void ArticleViewer::slotArticlesUpdated(TreeNode* /*node*/, const QList<Article>& /*list*/)
{
    if (m_viewMode == CombinedView)
        slotUpdateCombinedView();
}

void ArticleViewer::slotArticlesAdded(TreeNode* /*node*/, const QList<Article>& /*list*/)
{
}

void ArticleViewer::slotArticlesRemoved(TreeNode* /*node*/, const QList<Article>& /*list*/)
{
}

void ArticleViewer::slotClear()
{
    disconnectFromNode(m_node);
    m_node = 0;
    m_article = Article();

    renderContent(QString());
}

void ArticleViewer::showNode(TreeNode* node)
{
    m_viewMode = CombinedView;

    if (node != m_node)
        disconnectFromNode(m_node);

    connectToNode(node);

    m_article = Article();
    m_node = node;

    if (node && !node->articles().isEmpty())
        m_link = node->articles().first().link();
    else
        m_link = KUrl();

    slotUpdateCombinedView();
}

void ArticleViewer::keyPressEvent(QKeyEvent* e)
{
    e->ignore();
}

void ArticleViewer::slotPaletteOrFontChanged()
{
    updateCss();
    reload();
}

void ArticleViewer::reload()
{
    beginWriting();
    m_part->write(m_currentText);
    endWriting();
}

void ArticleViewer::displayAboutPage()
{
    QString location = KStandardDirs::locate("data", "akregator/about/main.html");

    m_part->begin(KUrl::fromPath( location ));
    QString info =
            i18nc("%1: Akregator version; %2: homepage URL; "
            "--- end of comment ---",
    "<h2 style='margin-top: 0px;'>Welcome to Akregator %1</h2>"
            "<p>Akregator is a feed reader for the K Desktop Environment. "
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
    AKREGATOR_VERSION, // Akregator version
    "http://akregator.kde.org/"); // Akregator homepage URL

    QString fontSize = QString::number( pointsToPixel( Settings::mediumFontSize() ));
    QString appTitle = i18n("Akregator");
    QString catchPhrase = ""; //not enough space for a catch phrase at default window size i18n("Part of the Kontact Suite");
    QString quickDescription = i18n("An RSS feed reader for the K Desktop Environment.");

    QString content = KPIMUtils::kFileToByteArray(location);

    QString infocss = KStandardDirs::locate( "data", "libkdepim/about/kde_infopage.css" );
    QString rtl = kapp->isRightToLeft() ? QString("@import \"%1\";" ).arg( KStandardDirs::locate( "data", "libkdepim/about/kde_infopage_rtl.css" )) : QString();

    m_part->write(content.arg(infocss).arg(rtl).arg(fontSize).arg(appTitle).arg(catchPhrase).arg(quickDescription).arg(info));
    m_part->end();
}

ArticleViewerPart::ArticleViewerPart(QWidget* parent) : KHTMLPart(parent),
     m_button(-1)
{
    setXMLFile(KStandardDirs::locate("data", "akregator/articleviewer.rc"), true);
}

int ArticleViewerPart::button() const
{
    return m_button;
}

bool ArticleViewerPart::closeUrl()
{
    emit browserExtension()->loadingProgress(-1);
    emit canceled(QString());
    return KHTMLPart::closeUrl();
}

bool ArticleViewerPart::urlSelected(const QString &url, int button, int state, const QString &_target,
                                    const KParts::OpenUrlArguments& args,
                                    const KParts::BrowserArguments& browserArgs)
{
    m_button = button;
    return KHTMLPart::urlSelected(url,button,state,_target,args,browserArgs);
#ifdef __GNUC__
#warning port disable_introduction
#endif
    /*
    if (url == "config:/disable_introduction")
    {
        if(KMessageBox::questionYesNo( widget(), i18n("Are you sure you want to disable this introduction page?"), i18n("Disable Introduction Page"), i18n("Disable"), i18n("Keep Enabled") ) == KMessageBox::Yes)
        {
            KConfig* conf = Settings::self()->config();
            conf->setGroup("General");
            conf->writeEntry("Disable Introduction", "true");
        }
    }
    else
    {
        m_url = completeURL(url);
        browserExtension()->setURLArgs(args);
        if (button == Qt::LeftButton)
        {
            switch (Settings::lMBBehaviour())
            {
                case Settings::EnumLMBBehaviour::OpenInExternalBrowser:
                    slotOpenLinkInBrowser();
                    break;
                case Settings::EnumLMBBehaviour::OpenInBackground:
                    slotOpenLinkInBackgroundTab();
                    break;
                default:
                    slotOpenLinkInForegroundTab();
                    break;
            }
            return;
        }
        else if (button == Qt::MidButton)
        {
            switch (Settings::mMBBehaviour())
            {
                case Settings::EnumMMBBehaviour::OpenInExternalBrowser:
                    slotOpenLinkInBrowser();
                    break;
                case Settings::EnumMMBBehaviour::OpenInBackground:
                    slotOpenLinkInBackgroundTab();
                    break;
                default:
                    slotOpenLinkInForegroundTab();
                    break;
            }
            return;
        }
        KHTMLPart::urlSelected(url,button,state,_target,args);
    }
    */
}

void ArticleViewer::updateCss()
{
    m_normalModeCSS =  m_normalViewFormatter->getCss();
    m_combinedModeCSS = m_combinedViewFormatter->getCss();
}

void ArticleViewer::setNormalViewFormatter(ArticleFormatterPtr formatter)
{
    m_normalViewFormatter = formatter;
    m_normalViewFormatter->setPaintDevice(m_part->view());
}

void ArticleViewer::setCombinedViewFormatter(ArticleFormatterPtr formatter)
{
    m_combinedViewFormatter = formatter;
    m_combinedViewFormatter->setPaintDevice(m_part->view());
}

} // namespace Akregator

#include "articleviewer.moc"

