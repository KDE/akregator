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
#include <QVariantHash>
class KActionCollection;
namespace MessageViewer
{
class WebViewAccessKey;
}
namespace Akregator
{
class ArticleViewerNg : public KWebView
{
    Q_OBJECT
public:
    enum OpenUrlType
    {
        CurrentTab,         ///< open url in current tab
        NewTab,             ///< open url according to users settings
        NewFocusedTab,      ///< open url in new tab and focus it
        NewBackGroundTab,   ///< open url in new background tab
        NewWindow,          ///< open url in new window
        WebApp              ///< open url in a web app window
    };


    explicit ArticleViewerNg(KActionCollection *ac, QWidget *parent = Q_NULLPTR);
    ~ArticleViewerNg();
    void paintAboutScreen(const QString &templateName, const QVariantHash &data);

protected:
    void keyReleaseEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
private:
    KActionCollection *mActionCollection;
    MessageViewer::WebViewAccessKey *mWebViewAccessKey;
};
}

#endif // ARTICLEVIEWERNG_H
