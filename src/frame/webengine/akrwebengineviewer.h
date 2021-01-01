/*
   SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AKRWEBENGINEVIEWER_H
#define AKRWEBENGINEVIEWER_H

#include "articleviewer-ng/webengine/articleviewerwebengine.h"

class KActionCollection;
namespace Akregator {
class AkrWebEngineViewer : public ArticleViewerWebEngine
{
    Q_OBJECT
public:
    explicit AkrWebEngineViewer(KActionCollection *ac, QWidget *parent = nullptr);
    ~AkrWebEngineViewer() override;

protected:
    void displayContextMenu(const QPoint &pos) override;

    void contextMenuEvent(QContextMenuEvent *e) override;
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;
private Q_SLOTS:
    void slotWebHitFinished(const WebEngineViewer::WebHitTestResult &result);
};
}

#endif // WEBENGINEVIEWER_H
