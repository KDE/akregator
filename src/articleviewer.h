/*
    This file is part of Akregator.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>

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

#ifndef AKREGATOR_ARTICLEVIEWER_H
#define AKREGATOR_ARTICLEVIEWER_H

#include "article.h"
#include "articlefilter.h"
#include "sharedptr.h"

#include <QWidget>

#include <khtml_part.h>

class KUrl;

namespace KIO
{
    class MetaData;
}

namespace Akregator {

class ArticleFormatter;
class Feed;
class Folder;
class OpenUrlRequest;
class TreeNode;

class ArticleViewerPart;

class ArticleViewer : public QWidget
{
    Q_OBJECT
    public:
        explicit ArticleViewer(QWidget* parent);
        ~ArticleViewer();

                
        /** Repaints the view. */
        void reload();
    
        void displayAboutPage();
    
        KParts::ReadOnlyPart* part() const;
        
        void setNormalViewFormatter(SharedPtr<ArticleFormatter> formatter);
        
        void setCombinedViewFormatter(SharedPtr<ArticleFormatter> formatter);
        
    public slots:
    
        void slotScrollUp(); 
        void slotScrollDown();
        void slotZoomIn();
        void slotZoomOut();
        void slotSetZoomFactor(int percent);
        void slotPrint();

    // Commandment: We are your interfaces.
    // You shall not use strange interfaces before us.

        /** Show single article (normal view) 
         * @param article the article to render
         */
        void slotShowArticle(const Akregator::Article& article);
    
        /** Shows the articles of the tree node @c node (combined view).
         * Changes in the node will update the view automatically.
         * 
         *  @param node The node to observe */
        void slotShowNode(Akregator::TreeNode* node);
    
        /** Set filters @c textFilter and @c statusFilter which will be used
         * if the viewer is in combined view mode
         * 
         * @param textFilter text filter
         * @param statusFilter status filter 
         */    
        void slotSetFilter(const Akregator::Filters::ArticleMatcher& textFilter, const Akregator::Filters::ArticleMatcher& statusFilter);
    
        /** Update view if combined view mode is set. Has to be called when
         * the displayed node gets modified.
         */ 
        void slotUpdateCombinedView();
    
        /** 
         * Clears the canvas and disconnects from the currently observed node
         * (if in combined view mode). 
         */
        void slotClear();

        void slotShowSummary(Akregator::TreeNode *node);

        void slotPaletteOrFontChanged();
    
    signals:
    
        /** This gets emitted when url gets clicked */
        void signalOpenUrlRequest(Akregator::OpenUrlRequest&); 

        void started(KIO::Job*);
        void selectionChanged();
        void completed();
        
    protected: // methods
        int pointsToPixel(int points) const;

        bool openUrl(const KUrl &url);
        
    protected slots:

        void slotOpenUrlRequestDelayed(const KUrl&, const KParts::URLArgs&);
        
        void slotCreateNewWindow(const KUrl& url, const KParts::URLArgs& args);
        
        void slotCreateNewWindow(const KUrl& url, 
                                    const KParts::URLArgs& args,
                                    const KParts::WindowArgs& windowArgs, 
                                    KParts::ReadOnlyPart*& part);
        
        void slotPopupMenu(KXMLGUIClient*, const QPoint&, const KUrl&, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t);

        /** Copies current link to clipboard. */
        void slotCopyLinkAddress();

        /** Copies currently selected text to clipboard */
        void slotCopy();

        /** Opens @c m_url inside this viewer */
        void slotOpenLinkInternal();

        /** Opens @c m_url in external viewer, eg. Konqueror */
        void slotOpenLinkInBrowser();

        /** Opens @c m_url in foreground tab */
        void slotOpenLinkInForegroundTab();

        /** Opens @c m_url in background tab */
        void slotOpenLinkInBackgroundTab();

        void slotSaveLinkAs();

        /** This changes cursor to wait cursor */
        void slotStarted(KIO::Job *);

        /** This reverts cursor back to normal one */
        void slotCompleted();

        void slotSelectionChanged();

        void slotArticlesUpdated(Akregator::TreeNode* node, const QList<Akregator::Article>& list);
        void slotArticlesAdded(Akregator::TreeNode* node, const QList<Akregator::Article>& list);
        void slotArticlesRemoved(Akregator::TreeNode* node, const QList<Akregator::Article>& list);
    
    protected: // attributes
        KUrl m_url;
    
    // from ArticleViewer  
    protected:
    
        virtual void keyPressEvent(QKeyEvent* e);
    
        /** renders @c body. Use this method whereever possible.
         *  @param body html to render, without header and footer */
        void renderContent(const QString& body);
    
        /** Resets the canvas and adds writes the HTML header to it.
            */
        void beginWriting();

        /** Finishes writing to the canvas and completes the HTML (by adding closing tags) */
        void endWriting();
    
        void updateCss();
        
        void connectToNode(TreeNode* node);
        void disconnectFromNode(TreeNode* node);
                            
        QString m_normalModeCSS;
        QString m_combinedModeCSS;
        QString m_htmlFooter;
        QString m_currentText;
        KUrl m_imageDir;
        TreeNode* m_node;
        SharedPtr<ArticleFormatter> m_normalViewFormatter;
        SharedPtr<ArticleFormatter> m_combinedViewFormatter;
        Article m_article;
        KUrl m_link;
        Akregator::Filters::ArticleMatcher m_textFilter; 
        Akregator::Filters::ArticleMatcher m_statusFilter;
        enum ViewMode { NormalView, CombinedView, SummaryView };
        ViewMode m_viewMode;
        
        ArticleViewerPart* m_part;
};

class ArticleViewerPart : public KHTMLPart
{
    Q_OBJECT

    public:
        explicit ArticleViewerPart(QWidget* parent);
        
        bool closeUrl();
        
        int button() const;
        
    protected:
        
        /** reimplemented to get the mouse button */
        void urlSelected(const QString &url, int button, int state, const QString &_target, KParts::URLArgs args);
        
    private:
        
        int m_button;
};
    
} // namespace Akregator

#endif // AKREGATOR_ARTICLEVIEWER_H

