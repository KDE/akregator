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

#include "articleviewer.h"
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

#include "kdepim-version.h"

#include <KPIMUtils/kpimutils/kfileio.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <QMenu>
#include <kmessagebox.h>
#include <krun.h>
#include <kshell.h>

#include <kstandardaction.h>
#include <ktoolinvocation.h>
#include <kurl.h>
#include <kglobalsettings.h>
#include <kparts/browserextension.h>
#include <kparts/browserrun.h>
#include <KGlobal>
#include <KUrl>

#include <QClipboard>
#include <QGridLayout>
#include <QKeyEvent>
#include <boost/bind.hpp>

#include <memory>
#include <cassert>
#include <QStandardPaths>

using namespace boost;
using namespace Akregator;
using namespace Akregator::Filters;

namespace Akregator {

ArticleViewer::ArticleViewer(QWidget *parent)
    : QWidget(parent),
      m_url(0),
      m_htmlFooter(),
      m_currentText(),
      m_imageDir( QUrl::fromLocalFile( QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1Char('/') + "akregator/Media/" )),
      m_node(0),
      m_viewMode(NormalView),
      m_part( new ArticleViewerPart( this ) ),
      m_normalViewFormatter( new DefaultNormalViewFormatter( m_imageDir, m_part->view() ) ),
      m_combinedViewFormatter( new DefaultCombinedViewFormatter( m_imageDir, m_part->view() ) )
{
    QGridLayout* layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_part->widget(), 0, 0);

    setFocusProxy( m_part->widget() );

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
    m_part->view()->setFrameStyle( QFrame::StyledPanel|QFrame::Sunken );

    // change the cursor when loading stuff...
    connect( m_part, SIGNAL(started(KIO::Job*)),
             this, SLOT(slotStarted(KIO::Job*)));
    connect( m_part, SIGNAL(completed()),
             this, SLOT(slotCompleted()));

    KParts::BrowserExtension* ext = m_part->browserExtension();
    connect(ext, SIGNAL(popupMenu(QPoint,KUrl,mode_t,KParts::OpenUrlArguments,KParts::BrowserArguments,KParts::BrowserExtension::PopupFlags,KParts::BrowserExtension::ActionGroupMap)),
             this, SLOT(slotPopupMenu(QPoint,KUrl,mode_t,KParts::OpenUrlArguments,KParts::BrowserArguments,KParts::BrowserExtension::PopupFlags))); // ActionGroupMap argument removed, unused by slot

    connect( ext, SIGNAL(openUrlRequestDelayed(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)),
             this, SLOT(slotOpenUrlRequestDelayed(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)) );

    connect(ext, SIGNAL(createNewWindow(KUrl,
            KParts::OpenUrlArguments,
            KParts::BrowserArguments,
            KParts::WindowArgs,
            KParts::ReadOnlyPart**)),
            this, SLOT(slotCreateNewWindow(KUrl,
                         KParts::OpenUrlArguments,
                         KParts::BrowserArguments,
                         KParts::WindowArgs,
                         KParts::ReadOnlyPart**)));

    QAction * action = 0;

    action = m_part->actionCollection()->addAction("copylinkaddress");
    action->setText(i18n("Copy &Link Address"));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotCopyLinkAddress()));

    action = m_part->actionCollection()->addAction("savelinkas");
    action->setText(i18n("&Save Link As..."));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotSaveLinkAs()));

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
    if (req.options() == OpenUrlRequest::None)                // no explicit new window,
        req.setOptions(OpenUrlRequest::NewTab);               // so must open new tab

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
    if ( part )
        *part = req.part();
}

void ArticleViewer::slotPopupMenu(const QPoint& p, const KUrl& kurl, mode_t, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&, KParts::BrowserExtension::PopupFlags kpf)
{
    const bool isLink = (kpf & KParts::BrowserExtension::ShowNavigationItems) == 0; // ## why not use kpf & IsLink ?
    const bool isSelection = (kpf & KParts::BrowserExtension::ShowTextSelectionItems) != 0;

    QString url = kurl.url();

    m_url = url;
    QMenu popup;

    if (isLink && !isSelection)
    {
        popup.addAction( createOpenLinkInNewTabAction( kurl, this, SLOT(slotOpenLinkInForegroundTab()), &popup ) );
        popup.addAction( createOpenLinkInExternalBrowserAction( kurl, this, SLOT(slotOpenLinkInBrowser()), &popup ) );
        popup.addSeparator();
        popup.addAction( m_part->action("savelinkas") );
        popup.addAction( m_part->action("copylinkaddress") );
    }
    else
    {
        if (isSelection)
        {
            popup.addAction( ActionManager::getInstance()->action("viewer_copy") );
            popup.addSeparator();
        }
        popup.addAction( ActionManager::getInstance()->action("viewer_print") );
       //QAction *ac = action("setEncoding");
       //if (ac)
       //     ac->plug(&popup);
        popup.addSeparator();
        popup.addAction( ActionManager::getInstance()->action("inc_font_sizes") );
        popup.addAction( ActionManager::getInstance()->action("dec_font_sizes") );
    }
    popup.exec(p);
}

// taken from KDevelop
void ArticleViewer::slotCopy()
{
    QString text = m_part->selectedText();
    text.replace( QChar( 0xa0 ), ' ' );
    QClipboard* const cb = QApplication::clipboard();
    assert( cb );
    cb->setText( text, QClipboard::Clipboard );
}

void ArticleViewer::slotCopyLinkAddress()
{
    if(m_url.isEmpty()) return;
    QClipboard *cb = QApplication::clipboard();
    cb->setText(m_url.prettyUrl(), QClipboard::Clipboard);
    // don't set url to selection as it's a no-no according to a fd.o spec
    // which spec? Nobody seems to care (tested Firefox (3.5.10) Konqueror,and KMail (4.2.3)), so I re-enable the following line unless someone gives
    // a good reason to remove it again (bug 183022) --Frank
    cb->setText(m_url.prettyUrl(), QClipboard::Selection);
}

void ArticleViewer::slotSelectionChanged()
{
    ActionManager::getInstance()->action("viewer_copy")->setEnabled(!m_part->selectedText().isEmpty());
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

void ArticleViewer::slotZoomIn(int id)
{
    if (id != 0)
        return;

    int zf = m_part->fontScaleFactor();
    if (zf < 100)
    {
        zf = zf - (zf % 20) + 20;
        m_part->setFontScaleFactor(zf);
    }
    else
    {
        zf = zf - (zf % 50) + 50;
        m_part->setFontScaleFactor(zf < 300 ? zf : 300);
    }
}

void ArticleViewer::slotZoomOut(int id)
{
    if (id != 0)
        return;

    int zf = m_part->fontScaleFactor();
    if (zf <= 100)
    {
        zf = zf - (zf % 20) - 20;
        m_part->setFontScaleFactor(zf > 20 ? zf : 20);
    }
    else
    {
        zf = zf - (zf % 50) - 50;
        m_part->setFontScaleFactor(zf);
    }
}

void ArticleViewer::slotSetZoomFactor(int percent)
{
    m_part->setFontScaleFactor(percent);
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
            connect( node, SIGNAL(signalChanged(Akregator::TreeNode*)), this, SLOT(slotUpdateCombinedView()) );
            connect( node, SIGNAL(signalArticlesAdded(Akregator::TreeNode*,QList<Akregator::Article>)), this, SLOT(slotArticlesAdded(Akregator::TreeNode*,QList<Akregator::Article>)));
            connect( node, SIGNAL(signalArticlesRemoved(Akregator::TreeNode*,QList<Akregator::Article>)), this, SLOT(slotArticlesRemoved(Akregator::TreeNode*,QList<Akregator::Article>)));
            connect( node, SIGNAL(signalArticlesUpdated(Akregator::TreeNode*,QList<Akregator::Article>)), this, SLOT(slotArticlesUpdated(Akregator::TreeNode*,QList<Akregator::Article>)));
        }
        if (m_viewMode == SummaryView)
            connect( node, SIGNAL(signalChanged(Akregator::TreeNode*)), this, SLOT(slotShowSummary(Akregator::TreeNode*)) );

        connect( node, SIGNAL(signalDestroyed(Akregator::TreeNode*)), this, SLOT(slotClear()) );
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
    //qDebug() << text;
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

    //pass link to the KHTMLPart to make relative links work
    //add a bogus query item to distinguish from m_link
    //fixes the Complete Story link if the url has an anchor (e.g. #reqRSS) in it
    //See bug 177754

    KUrl url(m_link);
    url.addQueryItem("akregatorPreviewMode", "true");
    m_part->begin(url);
    m_part->write(head);
}

void ArticleViewer::endWriting()
{
    m_part->write(m_htmlFooter);
    //qDebug() << m_htmlFooter;
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
    m_link.clear();
    renderContent(summary);

    setArticleActionsEnabled(false);
}

void ArticleViewer::showArticle( const Akregator::Article& article )
{
    if ( article.isNull() || article.isDeleted() )
    {
        slotClear();
        return;
    }

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

    setArticleActionsEnabled(true);
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

void ArticleViewer::setFilters(const std::vector< shared_ptr<const AbstractMatcher> >& filters )
{
    if ( filters == m_filters )
        return;

    m_filters = filters;

    slotUpdateCombinedView();
}

void ArticleViewer::slotUpdateCombinedView()
{
    if (m_viewMode != CombinedView)
        return;

    if (!m_node)
        return slotClear();


   QString text;

   int num = 0;
   QTime spent;
   spent.start();

   const std::vector< shared_ptr<const AbstractMatcher> >::const_iterator filterEnd = m_filters.end();

   Q_FOREACH( const Article& i, m_articles )
   {
       if ( i.isDeleted() )
           continue;

       if ( std::find_if( m_filters.begin(), m_filters.end(), !bind( &AbstractMatcher::matches, _1, i ) ) != filterEnd )
           continue;

       text += "<p><div class=\"article\">"+m_combinedViewFormatter->formatArticle( i, ArticleFormatter::NoIcon)+"</div><p>";
       ++num;
   }

   qDebug() <<"Combined view rendering: (" << num <<" articles):" <<"generating HTML:" << spent.elapsed() <<"ms";
   renderContent(text);
   qDebug() <<"HTML rendering:" << spent.elapsed() <<"ms";
}

void ArticleViewer::slotArticlesUpdated(TreeNode* /*node*/, const QList<Article>& /*list*/)
{
    if (m_viewMode == CombinedView) {
        //TODO
        slotUpdateCombinedView();
    }
}

void ArticleViewer::slotArticlesAdded(TreeNode* /*node*/, const QList<Article>& list)
{
    if (m_viewMode == CombinedView) {
        //TODO sort list, then merge
        m_articles << list;
        std::sort( m_articles.begin(), m_articles.end() );
        slotUpdateCombinedView();
    }
}

void ArticleViewer::slotArticlesRemoved(TreeNode* /*node*/, const QList<Article>& list )
{
    Q_UNUSED(list)

    if (m_viewMode == CombinedView) {
        //TODO
        slotUpdateCombinedView();
    }
}

void ArticleViewer::slotClear()
{
    disconnectFromNode(m_node);
    m_node = 0;
    m_article = Article();
    m_articles.clear();

    renderContent(QString());
}

void ArticleViewer::showNode(TreeNode* node)
{
    m_viewMode = CombinedView;

    if (node != m_node)
        disconnectFromNode(m_node);

    connectToNode(node);

    m_articles.clear();
    m_article = Article();
    m_node = node;

    delete m_listJob;

    m_listJob = node->createListJob();
    connect( m_listJob, SIGNAL(finished(KJob*)), this, SLOT(slotArticlesListed(KJob*)));
    m_listJob->start();



    slotUpdateCombinedView();
}

void ArticleViewer::slotArticlesListed( KJob* job ) {
    assert( job );
    assert( job == m_listJob );

    TreeNode* node = m_listJob->node();

    if ( job->error() || !node ) {
        if ( !node )
            qWarning() << "Node to be listed is already deleted";
        else
            qWarning() << job->errorText();
        slotUpdateCombinedView();
        return;
    }

    m_articles = m_listJob->articles();
    std::sort( m_articles.begin(), m_articles.end() );

    if (node && !m_articles.isEmpty())
        m_link = m_articles.first().link();
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

QSize ArticleViewer::sizeHint() const
{
    // Increase height a bit so that we can (roughly) read 25 lines of text
    QSize sh = QWidget::sizeHint();
    sh.setHeight(qMax(sh.height(), 25 * fontMetrics().height()));
    return sh;
}

void ArticleViewer::displayAboutPage()
{
    QString location = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "akregator/about/main.html");

    m_part->begin(QUrl::fromLocalFile( location ));
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
    KDEPIM_VERSION,
    "http://akregator.kde.org/"); // Akregator homepage URL

    QString fontSize = QString::number( pointsToPixel( Settings::mediumFontSize() ));
    QString appTitle = i18n("Akregator");
    QString catchPhrase = ""; //not enough space for a catch phrase at default window size i18n("Part of the Kontact Suite");
    QString quickDescription = i18n("A KDE news feed reader.");

    QString content = KPIMUtils::kFileToByteArray(location);

    QString infocss = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kdeui/about/kde_infopage.css" );
    QString rtl = kapp->isRightToLeft() ? QString("@import \"%1\";" ).arg( QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kdeui/about/kde_infopage_rtl.css" )) : QString();

    m_part->write( content.arg( infocss, rtl, fontSize, appTitle, catchPhrase, quickDescription, info ) );
    m_part->end();
}

ArticleViewerPart::ArticleViewerPart(QWidget* parent) : KHTMLPart(parent),
     m_button(-1)
{
    setXMLFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "akregator/articleviewer.rc"), true);
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
    if(url == "config:/disable_introduction")
    {
        KGuiItem yesButton(KStandardGuiItem::yes());
        yesButton.setText(i18n("Disable"));
        KGuiItem noButton(KStandardGuiItem::no());
        noButton.setText(i18n("Keep Enabled"));
        if(KMessageBox::questionYesNo( widget(), i18n("Are you sure you want to disable this introduction page?"), i18n("Disable Introduction Page"), yesButton, noButton) == KMessageBox::Yes)
        {
            KConfigGroup conf(Settings::self()->config(), "General");
            conf.writeEntry("Disable Introduction", "true");
            conf.sync();
            return true;
        }

        return false;
    }
    else
        return KHTMLPart::urlSelected(url,button,state,_target,args,browserArgs);
}

void ArticleViewer::updateCss()
{
    m_normalModeCSS =  m_normalViewFormatter->getCss();
    m_combinedModeCSS = m_combinedViewFormatter->getCss();
}

void ArticleViewer::setNormalViewFormatter( const shared_ptr<ArticleFormatter>& formatter )
{
    assert( formatter );
    m_normalViewFormatter = formatter;
    m_normalViewFormatter->setPaintDevice(m_part->view());
}

void ArticleViewer::setCombinedViewFormatter( const shared_ptr<ArticleFormatter>& formatter )
{
    assert( formatter );
    m_combinedViewFormatter = formatter;
    m_combinedViewFormatter->setPaintDevice(m_part->view());
}

void ArticleViewer::setArticleActionsEnabled(bool enabled)
{
    ActionManager::getInstance()->setArticleActionsEnabled(enabled);
}

} // namespace Akregator


