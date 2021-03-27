/*
   SPDX-FileCopyrightText: 2015-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akregator_export.h"
#include <QObject>
#include <QUrl>
namespace Akregator
{
class ArticleViewerWebEngine;
class AKREGATOR_EXPORT ArticleHtmlWebEngineWriter : public QObject
{
    Q_OBJECT
public:
    explicit ArticleHtmlWebEngineWriter(ArticleViewerWebEngine *view, QObject *parent = nullptr);
    ~ArticleHtmlWebEngineWriter() override;

    void begin();
    void end();
    void reset();
    void queue(const QString &str);
    void flush();
    void setBaseUrl(const QUrl &url);

Q_SIGNALS:
    void finished();

private:
    enum State { Begun, Queued, Ended } mState = Ended;

    QString mHtml;
    QUrl mBaseUrl;
    ArticleViewerWebEngine *const mWebView;
};
}

