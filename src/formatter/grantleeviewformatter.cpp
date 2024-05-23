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
#include <GrantleeTheme/GrantleeKi18nLocalizer>
#include <GrantleeTheme/QtResourceTemplateLoader>
#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QGuiApplication>
#include <QPalette>
#include <QVariantHash>
#include <QVariantList>

using namespace Akregator;

GrantleeViewFormatter::GrantleeViewFormatter(const QString &htmlFileName, int deviceDpiY)
    : mHtmlArticleFileName(htmlFileName)
    , mDirectionString(QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr"))
    , mDeviceDpiY(deviceDpiY)
{
    mEngine.localizer()->setApplicationDomain("akregator");
    mEngine.addTemplateLoader(QSharedPointer<GrantleeTheme::QtResourceTemplateLoader>::create());
}

GrantleeViewFormatter::~GrantleeViewFormatter() = default;

int GrantleeViewFormatter::pointsToPixel(int pointSize) const
{
    return (pointSize * mDeviceDpiY + 36) / 72;
}

void GrantleeViewFormatter::addStandardObject(QVariantHash &grantleeObject) const
{
    // get color scheme and window background color
    Colors appColor = getAppColor();

    // Ideally we should use <link href=""> in the html but this doesn't
    // work because the html is loaded via data:/ and can't access qrc.
    QFile cssFile(QStringLiteral(":/formatter/html/style.css"));
    if (!cssFile.open(QIODeviceBase::ReadOnly)) {
        Q_ASSERT(false);
    }

    grantleeObject.insert(QStringLiteral("applicationDir"), mDirectionString);
    grantleeObject.insert(QStringLiteral("standardFamilyFont"), Settings::standardFont());
    grantleeObject.insert(QStringLiteral("sansSerifFont"), Settings::sansSerifFont());
    grantleeObject.insert(QStringLiteral("serifFont"), Settings::serifFont());
    grantleeObject.insert(QStringLiteral("mediumFontSize"), Settings::mediumFontSize());
    grantleeObject.insert(QStringLiteral("smallFontSize"), Settings::minimumFontSize());
    grantleeObject.insert(QStringLiteral("sidebarCss"), sidebarCss(appColor));
    grantleeObject.insert(QStringLiteral("css"), cssFile.readAll());
    grantleeObject.insert(QStringLiteral("colorScheme"), appColor.colorScheme);
    grantleeObject.insert(QStringLiteral("backgroundColor"), appColor.backgroundColor);
}

Colors GrantleeViewFormatter::getAppColor() const
{
    QGuiApplication *guiApp = qobject_cast<QGuiApplication *>(QCoreApplication::instance());

    const QPalette palette = guiApp->palette();
    const QColor windowColor = palette.window().color();
    const QColor windowTextColor = palette.windowText().color();

    Colors values;
    values.backgroundColor = windowColor.name();

    // dark or light mode
    if (windowColor.value() < windowTextColor.value()) {
        values.colorScheme = QStringLiteral("dark");
    } else {
        values.colorScheme = QStringLiteral("light");
    }

    return values;
}

QString lighterColor(QString inputColor, int factor)
{
    QColor color(inputColor);
    color = color.lighter(factor);
    return color.name();
}

QString GrantleeViewFormatter::sidebarCss(const Colors &colors) const
{
    bool isDark = colors.colorScheme == QStringLiteral("dark");

    QString backgroundColor = isDark ? colors.backgroundColor : QStringLiteral("#fff");
    QString borderColor = isDark ? QStringLiteral("#55595C") : QStringLiteral("#BBBDBE");

    // use background color for scrollbar on dark themes to adapt better to color schemes
    QString lighterBackgroundColor = lighterColor(colors.backgroundColor, 200);
    QString thumbColor = isDark ? lighterBackgroundColor : QStringLiteral("#CACCCC");
    QString thumbHoverColor = isDark ? QStringLiteral("#346D8E") : QStringLiteral("#90C7E4");

    return QStringLiteral(
               "html::-webkit-scrollbar {\n"
               "  /* we'll add padding as \"border\" in the thumb*/\n"
               "    height: 20px;\n"
               "    width: 20px;\n"
               "    background: %1;\n"
               "    border-left: 1px solid %2;\n"
               "    padding-left: 1px;\n"
               "}\n\n"

               "html::-webkit-scrollbar-track {\n"
               "    border-radius: 20px;\n"
               "    width: 6px !important;\n"
               "    box-sizing: content-box;\n"
               "}\n\n"

               "html::-webkit-scrollbar-thumb {\n"
               "    background-color: %3;\n"
               "    border: 6px solid transparent;\n"
               "    border-radius: 20px;\n"
               "    background-clip: content-box;\n"
               "    width: 8px !important; /* 20px scrollbar - 2 * 6px border */\n"
               "    box-sizing: content-box;\n"
               "    min-height: 30px;\n"
               "}\n\n"

               "html::-webkit-scrollbar-thumb:window-inactive {\n"
               "   background-color: %3; /* when window is inactive it's gray */\n"
               "}\n\n"

               "html::-webkit-scrollbar-thumb:hover {\n"
               "    background-color: %4; /* hovered is a lighter blue */\n"
               "}\n\n"

               "html::-webkit-scrollbar-corner {\n"
               "    background-color: white;\n"
               "}\n\n")
        .arg(backgroundColor)
        .arg(borderColor)
        .arg(thumbColor)
        .arg(thumbHoverColor);
}

QString GrantleeViewFormatter::formatFeed(Akregator::Feed *feed)
{
    mTemplate = mEngine.loadByName(QStringLiteral(":/formatter/html/defaultnormalvisitfeed.html"));
    if (mTemplate->error()) {
        return mTemplate->errorString();
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

    KTextTemplate::Context context(feedObject);
    context.setLocalizer(mEngine.localizer());

    const QString contentHtml = mTemplate->render(&context);
    return contentHtml;
}

QString GrantleeViewFormatter::formatFolder(Akregator::Folder *node)
{
    mTemplate = mEngine.loadByName(QStringLiteral(":/formatter/html/defaultnormalvisitfolder.html"));
    if (mTemplate->error()) {
        return mTemplate->errorString();
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
    KTextTemplate::Context context(folderObject);
    context.setLocalizer(mEngine.localizer());

    const QString contentHtml = mTemplate->render(&context);
    return contentHtml;
}

QString GrantleeViewFormatter::formatArticles(const QList<Article> &article, ArticleFormatter::IconOption icon)
{
    mTemplate = mEngine.loadByName(QStringLiteral(":/formatter/html/normalview.html"));
    if (mTemplate->error()) {
        return mTemplate->errorString();
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

    KTextTemplate::Context context(articleObject);
    context.setLocalizer(mEngine.localizer());

    const QString contentHtml = mTemplate->render(&context);
    qDeleteAll(lstObj);
    return contentHtml;
}
