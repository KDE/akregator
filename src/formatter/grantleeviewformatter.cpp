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

#include "grantleeviewformatter.h"

#include <grantlee/engine.h>

using namespace Akregator;

GrantleeViewFormatter::GrantleeViewFormatter(const QString &themePath, QObject *parent)
    : QObject(parent)
{
    mEngine = new Grantlee::Engine;
    mTemplateLoader = QSharedPointer<Grantlee::FileSystemTemplateLoader>(new Grantlee::FileSystemTemplateLoader);

    changeGrantleePath(themePath);
}

GrantleeViewFormatter::~GrantleeViewFormatter()
{

}

QString GrantleeViewFormatter::formatArticle(const Article &article, ArticleFormatter::IconOption icon) const
{
    if (!mErrorMessage.isEmpty()) {
        return mErrorMessage;
    }
    QVariantHash mapping;
    Grantlee::Context context(mapping);
    const QString content = mSelfcontainedTemplate->render(&context);
    return content;
}

void GrantleeViewFormatter::refreshTemplate()
{
    mSelfcontainedTemplate = mEngine->loadByName(QStringLiteral("theme.html"));
    if (mSelfcontainedTemplate->error()) {
        mErrorMessage += mSelfcontainedTemplate->errorString() + QLatin1String("<br>");
    }
}

void GrantleeViewFormatter::changeGrantleePath(const QString &path)
{
    if (!mTemplateLoader) {
        mTemplateLoader = QSharedPointer<Grantlee::FileSystemTemplateLoader>(new Grantlee::FileSystemTemplateLoader);
    }
    mTemplateLoader->setTemplateDirs(QStringList() << path);
    mEngine->addTemplateLoader(mTemplateLoader);

    refreshTemplate();
}
