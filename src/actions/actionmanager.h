/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <QObject>

#include "akregator_export.h"

class QAction;

class QWidget;

namespace WebEngineViewer
{
class ZoomActionMenu;
}

namespace Akregator
{
/**
 * interface for accessing actions, popup menus etc. from
 * widgets.
 * (Extracted from the implementation to avoid dependencies
 *  between widgets and  Akregator::Part).
 */
class AKREGATOR_EXPORT ActionManager : public QObject
{
    Q_OBJECT

public:
    static ActionManager *getInstance();
    static void setInstance(ActionManager *manager);

    explicit ActionManager(QObject *parent = nullptr);
    ~ActionManager() override;

    virtual QAction *action(const QString &name) = 0;
    virtual QWidget *container(const QString &name) = 0;

    virtual WebEngineViewer::ZoomActionMenu *zoomActionMenu() const = 0;

    virtual void setArticleActionsEnabled(bool enabled) = 0;

private:
    static ActionManager *m_self;
};
} // namespace Akregator

