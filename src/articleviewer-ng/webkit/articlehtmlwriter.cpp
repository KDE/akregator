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

#include "articlehtmlwriter.h"

#include <QWebView>
#include "akregator_debug.h"

using namespace Akregator;

ArticleHtmlWriter::ArticleHtmlWriter(QWebView *view, QObject *parent)
    : QObject(parent),
      mState(Ended),
      mWebView(view)
{

}

ArticleHtmlWriter::~ArticleHtmlWriter()
{

}

void ArticleHtmlWriter::begin()
{
    if (mState != Ended) {
        qCWarning(AKREGATOR_LOG) << "begin() called on non-ended session!";
        reset();
    }
    // clear the widget:
    mWebView->setUpdatesEnabled(false);
    //mWebView->scrollUp(10);
    mWebView->load(QUrl());
    mState = Begun;
}

void ArticleHtmlWriter::end()
{
    if (mState != Begun) {
        qCWarning(AKREGATOR_LOG) << "Called on non-begun or queued session!";
    }
    mWebView->setHtml(mHtml, QUrl(QStringLiteral("file:///")));
    mWebView->show();
    mHtml.clear();

    mWebView->setUpdatesEnabled(true);
    mWebView->update();
    mState = Ended;
    Q_EMIT finished();
}

void ArticleHtmlWriter::reset()
{
    if (mState != Ended) {
        mHtml.clear();
        mState = Begun; // don't run into end()'s warning
        end();
        mState = Ended;
    }
}

void ArticleHtmlWriter::queue(const QString &str)
{
    if (mState != Begun) {
        qCWarning(AKREGATOR_LOG) << "Called in Ended or Queued state!";
    }
    mHtml.append(str);
}

void ArticleHtmlWriter::flush()
{
    mState = Begun; // don't run into end()'s warning
    end();
}
