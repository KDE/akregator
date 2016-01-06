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

#ifndef WEBVIEWFRAME_H
#define WEBVIEWFRAME_H

#include "articleviewer-ng/articleviewerwidgetng.h"
#include "frame.h"
#include "akregatorpart_export.h"
#include "feediconmanager.h"
#include <QObject>

namespace Akregator
{
class AKREGATORPART_EXPORT WebViewFrame : public Frame, public virtual FaviconListener
{
    Q_OBJECT
public:
    explicit WebViewFrame(KActionCollection *ac, QWidget *parent = Q_NULLPTR);
    ~WebViewFrame();

    KParts::ReadOnlyPart *part() const Q_DECL_OVERRIDE;
    QUrl url() const Q_DECL_OVERRIDE;
    bool openUrl(const OpenUrlRequest &request) Q_DECL_OVERRIDE;
    void loadConfig(const KConfigGroup &, const QString &) Q_DECL_OVERRIDE;
    void saveConfig(KConfigGroup &, const QString &) Q_DECL_OVERRIDE;
    void setFavicon(const QIcon &icon) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotTitleChanged(const QString &title);

private:
    Akregator::ArticleViewerWidgetNg *mArticleViewerWidgetNg;

};
}

#endif // WEBVIEWFRAME_H
