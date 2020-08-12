/*
   SPDX-FileCopyrightText: 2015-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ARTICLEHTMLWEBENGINEWRITER_H
#define ARTICLEHTMLWEBENGINEWRITER_H

#include <QObject>
#include <QUrl>
#include "akregator_export.h"
namespace Akregator {
class ArticleViewerWebEngine;
class AKREGATOR_EXPORT ArticleHtmlWebEngineWriter : public QObject
{
    Q_OBJECT
public:
    explicit ArticleHtmlWebEngineWriter(ArticleViewerWebEngine *view, QObject *parent = nullptr);
    ~ArticleHtmlWebEngineWriter();

    void begin();
    void end();
    void reset();
    void queue(const QString &str);
    void flush();
    void setBaseUrl(const QUrl &url);

Q_SIGNALS:
    void finished();

private:
    enum State {
        Begun,
        Queued,
        Ended
    } mState;

    QString mHtml;
    QUrl mBaseUrl;
    ArticleViewerWebEngine *mWebView = nullptr;
};
}

#endif // ARTICLHTMLWEBENGINEWRITER_H
