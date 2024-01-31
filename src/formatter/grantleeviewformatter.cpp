/*
   SPDX-FileCopyrightText: 2016-2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grantleeviewformatter.h"
#include "akregatorconfig.h"
#include "articlegrantleeobject.h"
#include "grantleeutil.h"
#include "utils.h"
#include <KLocalizedString>

#include "feed.h"
#include "folder.h"
#include <QApplication>
#include <QVariantHash>
#include <QVariantList>

using namespace Akregator;

GrantleeViewFormatter::GrantleeViewFormatter(const QString &htmlFileName, const QString &themePath, int deviceDpiY)
    : GrantleeTheme::GenericFormatter(htmlFileName, themePath)
    , mHtmlArticleFileName(htmlFileName)
    , mDirectionString(QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr"))
    , mGrantleeThemePath(QStringLiteral("file://") + themePath + QLatin1Char('/'))
    , mDeviceDpiY(deviceDpiY)
{
    setApplicationDomain("akregator");
}

GrantleeViewFormatter::~GrantleeViewFormatter() = default;

int GrantleeViewFormatter::pointsToPixel(int pointSize) const
{
    return (pointSize * mDeviceDpiY + 36) / 72;
}

void GrantleeViewFormatter::addStandardObject(QVariantHash &grantleeObject) const
{
    grantleeObject.insert(QStringLiteral("absoluteThemePath"), mGrantleeThemePath);
    grantleeObject.insert(QStringLiteral("applicationDir"), mDirectionString);
    grantleeObject.insert(QStringLiteral("standardFamilyFont"), Settings::standardFont());
    grantleeObject.insert(QStringLiteral("mediumFontSize"), Settings::mediumFontSize());
    grantleeObject.insert(QStringLiteral("sidebarCss"), sidebarCss());
}

QString GrantleeViewFormatter::sidebarCss() const
{
    return QStringLiteral(
        "html::-webkit-scrollbar {\n"
        "  /* we'll add padding as \"border\" in the thumb*/\n"
        "    height: 20px;\n"
        "    width: 20px;\n"
        "    background: white;\n"
        "    border-left: 1px solid #e5e5e5;\n"
        "    padding-left: 1px;\n"
        "}\n\n"

        "html::-webkit-scrollbar-track {\n"
        "    border-radius: 20px;\n"
        "    width: 6px !important;\n"
        "    box-sizing: content-box;\n"
        "}\n\n"

        "html::-webkit-scrollbar-thumb {\n"
        "    background-color: #CBCDCD;\n"
        "    border: 6px solid transparent;\n"
        "    border-radius: 20px;\n"
        "    background-clip: content-box;\n"
        "    width: 8px !important; /* 20px scrollbar - 2 * 6px border */\n"
        "    box-sizing: content-box;\n"
        "    min-height: 30px;\n"
        "}\n\n"

        "html::-webkit-scrollbar-thumb:window-inactive {\n"
        "   background-color: #949699; /* when window is inactive it's gray */\n"
        "}\n\n"

        "html::-webkit-scrollbar-thumb:hover {\n"
        "    background-color: #93CEE9; /* hovered is a lighter blue */\n"
        "}\n\n"

        "html::-webkit-scrollbar-corner {\n"
        "    background-color: white;\n"
        "}\n\n");
}

QString GrantleeViewFormatter::formatFeed(Akregator::Feed *feed)
{
    setDefaultHtmlMainFile(QStringLiteral("defaultnormalvisitfeed.html"));
    if (!errorMessage().isEmpty()) {
        return errorMessage();
    }
    QVariantHash feedObject;
    addStandardObject(feedObject);
    feedObject.insert(QStringLiteral("strippedTitle"), Utils::stripTags(feed->title()));
    QString numberOfArticle;
    if (feed->unread() == 0) {
        numberOfArticle = i18n(" (no unread articles)");
    } else {
        numberOfArticle = i18np(" (1 unread article)", " (%1 unread articles)", feed->unread());
    }

    feedObject.insert(QStringLiteral("feedCount"), numberOfArticle);

    QString feedImage;
    if (!feed->logoInfo().imageUrl.isEmpty()) { // image
        feedImage = GrantleeUtil::imageFeed(feed);
    } else {
        feedImage = QStringLiteral("<div class=\"body\">");
    }
    feedObject.insert(QStringLiteral("feedImage"), feedImage);

    if (!feed->description().isEmpty()) {
        QString feedDescription = QStringLiteral("<div dir=\"%1\">").arg(mDirectionString);
        feedDescription += i18n("<b>Description:</b> %1<br />", feed->description());
        feedDescription += QStringLiteral("</div>"); // /description
        feedObject.insert(QStringLiteral("feedDescription"), feedDescription);
    }

    if (!feed->htmlUrl().isEmpty()) {
        QString feedHomePage = QStringLiteral("<div dir=\"%1\">").arg(mDirectionString);
        feedHomePage += i18n("<b>Homepage:</b> <a href=\"%1\">%1</a>", feed->htmlUrl());
        feedHomePage += QStringLiteral("</div>"); // / link
        feedObject.insert(QStringLiteral("feedHomePage"), feedHomePage);
    }

    if (!feed->copyright().isEmpty()) {
        QString feedCopyright = QStringLiteral("<div dir=\"%1\">").arg(mDirectionString);
        feedCopyright += i18n("<b>Copyright:</b> %1</a>", feed->copyright());
        feedCopyright += QStringLiteral("</div>");
        feedObject.insert(QStringLiteral("feedCopyright"), feedCopyright);
    }

    return render(feedObject);
}

QString GrantleeViewFormatter::formatFolder(Akregator::Folder *node)
{
    setDefaultHtmlMainFile(QStringLiteral("defaultnormalvisitfolder.html"));
    if (!errorMessage().isEmpty()) {
        return errorMessage();
    }
    QVariantHash folderObject;
    addStandardObject(folderObject);

    folderObject.insert(QStringLiteral("nodeTitle"), node->title());
    QString numberOfArticle;
    if (node->unread() == 0) {
        numberOfArticle = i18n(" (no unread articles)");
    } else {
        numberOfArticle = i18np(" (1 unread article)", " (%1 unread articles)", node->unread());
    }

    folderObject.insert(QStringLiteral("nodeCount"), numberOfArticle);
    return render(folderObject);
}

QString GrantleeViewFormatter::formatArticles(const QList<Article> &article, ArticleFormatter::IconOption icon)
{
    setDefaultHtmlMainFile(mHtmlArticleFileName);
    if (!errorMessage().isEmpty()) {
        return errorMessage();
    }

    QVariantHash articleObject;

    QVariantList articlesList;
    const int nbArticles(article.count());
    articlesList.reserve(nbArticles);
    QList<ArticleGrantleeObject *> lstObj;
    lstObj.reserve(nbArticles);
    for (int i = 0; i < nbArticles; ++i) {
        auto articleObj = new ArticleGrantleeObject(article.at(i), icon);
        articlesList << QVariant::fromValue(static_cast<QObject *>(articleObj));
        lstObj.append(articleObj);
    }
    articleObject.insert(QStringLiteral("articles"), articlesList);

    addStandardObject(articleObject);
    articleObject.insert(QStringLiteral("loadExternalReference"), Settings::loadExternalReferences());
    articleObject.insert(QStringLiteral("dateI18n"), i18n("Date"));
    articleObject.insert(QStringLiteral("commentI18n"), i18n("Comment"));
    articleObject.insert(QStringLiteral("completeStoryI18n"), i18n("Complete Story"));
    articleObject.insert(QStringLiteral("authorI18n"), i18n("Author"));
    articleObject.insert(QStringLiteral("enclosureI18n"), i18n("Enclosure"));

    const QString str = render(articleObject);
    qDeleteAll(lstObj);
    return str;
}
