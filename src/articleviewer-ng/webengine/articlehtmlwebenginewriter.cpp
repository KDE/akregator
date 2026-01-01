/*
   SPDX-FileCopyrightText: 2015-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "articlehtmlwebenginewriter.h"
#include "akregator_debug.h"
#include "articleviewerwebengine.h"

using namespace Akregator;

ArticleHtmlWebEngineWriter::ArticleHtmlWebEngineWriter(ArticleViewerWebEngine *view, QObject *parent)
    : QObject(parent)
    , mBaseUrl(QStringLiteral("file:///"))
    , mWebView(view)
{
}

ArticleHtmlWebEngineWriter::~ArticleHtmlWebEngineWriter() = default;

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

#include "moc_articlehtmlwebenginewriter.cpp"
