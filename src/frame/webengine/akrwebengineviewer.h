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

#ifndef AKRWEBENGINEVIEWER_H
#define AKRWEBENGINEVIEWER_H

#include "articleviewer-ng/webengine/articleviewerwebengine.h"

class KActionCollection;
namespace Akregator
{
class AkrWebEngineViewer : public ArticleViewerWebEngine
{
    Q_OBJECT
public:
    explicit AkrWebEngineViewer(KActionCollection *ac, QWidget *parent = Q_NULLPTR);
    ~AkrWebEngineViewer();

protected:
    void displayContextMenu(const QPoint &pos) Q_DECL_OVERRIDE;

    void contextMenuEvent(QContextMenuEvent *e) Q_DECL_OVERRIDE;
private Q_SLOTS:
    void slotWebHitFinished(const WebEngineViewer::WebHitTestResult &result);
};
}

#endif // WEBENGINEVIEWER_H
