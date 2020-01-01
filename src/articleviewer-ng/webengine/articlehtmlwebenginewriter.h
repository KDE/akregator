/*
   Copyright (c) 2015-2020 Laurent Montel <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
