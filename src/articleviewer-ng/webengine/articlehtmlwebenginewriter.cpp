/*
   Copyright (C) 2015-2020 Laurent Montel <montel@kde.org>

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

#include "articlehtmlwebenginewriter.h"
#include "articleviewerwebengine.h"
#include "akregator_debug.h"

using namespace Akregator;

ArticleHtmlWebEngineWriter::ArticleHtmlWebEngineWriter(ArticleViewerWebEngine *view, QObject *parent)
    : QObject(parent)
    , mState(Ended)
    , mBaseUrl(QStringLiteral("file:///"))
    , mWebView(view)
{
}

ArticleHtmlWebEngineWriter::~ArticleHtmlWebEngineWriter()
{
}

void ArticleHtmlWebEngineWriter::begin()
{
    if (mState != Ended) {
        qCWarning(AKREGATOR_LOG) << "begin() called on non-ended session!";
        reset();
    }
    // clear the widget:
    mWebView->setUpdatesEnabled(false);
    mWebView->load(QUrl(QStringLiteral("about:blank")));
    mState = Begun;
}

void ArticleHtmlWebEngineWriter::setBaseUrl(const QUrl &url)
{
    mBaseUrl = url;
}

void ArticleHtmlWebEngineWriter::end()
{
    if (mState != Begun) {
        qCWarning(AKREGATOR_LOG) << "Called on non-begun or queued session!";
    }
    mWebView->setHtml(mHtml, mBaseUrl);
    mWebView->show();
    mHtml.clear();

    mWebView->setUpdatesEnabled(true);
    mWebView->update();
    mState = Ended;
    Q_EMIT finished();
}

void ArticleHtmlWebEngineWriter::reset()
{
    if (mState != Ended) {
        mHtml.clear();
        mState = Begun; // don't run into end()'s warning
        end();
        mState = Ended;
    }
}

void ArticleHtmlWebEngineWriter::queue(const QString &str)
{
    if (mState != Begun) {
        qCWarning(AKREGATOR_LOG) << "Called in Ended or Queued state!";
    }
    mHtml.append(str);
}

void ArticleHtmlWebEngineWriter::flush()
{
    mState = Begun; // don't run into end()'s warning
    end();
}
