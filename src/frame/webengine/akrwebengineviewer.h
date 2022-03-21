/*
   SPDX-FileCopyrightText: 2016-2022 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "articleviewer-ng/webengine/articleviewerwebengine.h"

class KActionCollection;
namespace Akregator
{
class AkrWebEngineViewer : public ArticleViewerWebEngine
{
    Q_OBJECT
public:
    explicit AkrWebEngineViewer(KActionCollection *ac, QWidget *parent = nullptr);
    ~AkrWebEngineViewer() override;
    void updateSecurity() override;

protected:
    void displayContextMenu(const QPoint &pos) override;

    void contextMenuEvent(QContextMenuEvent *e) override;
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;
private Q_SLOTS:
    void slotWebHitFinished(const WebEngineViewer::WebHitTestResult &result);
};
}
