/*
   SPDX-FileCopyrightText: 2016-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grantleeviewformatter.h"
#include "akregator_debug.h"
#include "akregatorconfig.h"
#include "articlegrantleeobject.h"
#include "grantleeutil.h"
#include "utils.h"
#include <KLocalizedString>

#include "feed.h"
#include "folder.h"
#include <GrantleeTheme/GrantleeKi18nLocalizer>
#include <KTextTemplate/FileSystemTemplateLoader>
#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QGuiApplication>
#include <QPalette>
#include <QVariantHash>
#include <QVariantList>
#include <utility>

using namespace Qt::Literals::StringLiterals;

using namespace Qt::Literals;
using namespace Akregator;

GrantleeViewFormatter::GrantleeViewFormatter(QString htmlFileName, int deviceDpiY)
    : mHtmlArticleFileName(std::move(htmlFileName))
    , mDirectionString(QApplication::isRightToLeft() ? u"rtl"_s : u"ltr"_s)
    , mDeviceDpiY(deviceDpiY)
{
    mEngine.localizer()->setApplicationDomain("akregator");
    auto loader = QSharedPointer<KTextTemplate::FileSystemTemplateLoader>::create();
    loader->setTemplateDirs({u":/"_s});
    mEngine.addTemplateLoader(loader);
}

GrantleeViewFormatter::~GrantleeViewFormatter() = default;

int GrantleeViewFormatter::pointsToPixel(int pointSize) const
{
    return (pointSize * mDeviceDpiY + 36) / 72;
}

void GrantleeViewFormatter::addStandardObject(QVariantHash &grantleeObject) const
{
    // get color scheme and window background color
    const Colors appColor = getAppColor();

    // Ideally we should use <link href=""> in the html but this doesn't
    // work because the html is loaded via data:/ and can't access qrc.
    QFile cssFile(u":/formatter/html/style.css"_s);
    if (!cssFile.open(QIODeviceBase::ReadOnly)) {
        qCWarning(AKREGATOR_LOG) << "Impossible to read:" << cssFile.fileName();
        Q_ASSERT(false);
    }

    grantleeObject.insert(u"applicationDir"_s, mDirectionString);
    grantleeObject.insert(u"standardFamilyFont"_s, Settings::standardFont());
    grantleeObject.insert(u"sansSerifFont"_s, Settings::sansSerifFont());
    grantleeObject.insert(u"serifFont"_s, Settings::serifFont());
    grantleeObject.insert(u"mediumFontSize"_s, Settings::mediumFontSize());
    grantleeObject.insert(u"smallFontSize"_s, Settings::minimumFontSize());
    grantleeObject.insert(u"sidebarCss"_s, sidebarCss(appColor));
    grantleeObject.insert(u"css"_s, cssFile.readAll());
    grantleeObject.insert(u"colorScheme"_s, appColor.colorScheme);
    grantleeObject.insert(u"backgroundColor"_s, appColor.backgroundColor);
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
        values.colorScheme = u"dark"_s;
    } else {
        values.colorScheme = u"light"_s;
    }

    return values;
}

QString lighterColor(const QString &inputColor, int factor)
{
    QColor color(inputColor);
    color = color.lighter(factor);
    return color.name();
}

QString GrantleeViewFormatter::sidebarCss(const Colors &colors) const
{
    const bool isDark = colors.colorScheme == "dark"_L1;

    QString backgroundColor = isDark ? colors.backgroundColor : u"#fff"_s;
    QString borderColor = isDark ? u"#55595C"_s : u"#BBBDBE"_s;

    // use background color for scrollbar on dark themes to adapt better to color schemes
    QString lighterBackgroundColor = lighterColor(colors.backgroundColor, 200);
    QString thumbColor = isDark ? lighterBackgroundColor : u"#CACCCC"_s;
    QString thumbHoverColor = isDark ? u"#346D8E"_s : u"#90C7E4"_s;

    return u"html::-webkit-scrollbar {\n"
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
           "}\n\n"_s.arg(backgroundColor)
               .arg(borderColor)
               .arg(thumbColor)
               .arg(thumbHoverColor);
}

QString GrantleeViewFormatter::formatFeed(Akregator::Feed *feed)
{
    mTemplate = mEngine.loadByName(u"formatter/html/defaultnormalvisitfeed.html"_s);
    if (mTemplate->error()) {
        return mTemplate->errorString();
    }
    QVariantHash feedObject;
    addStandardObject(feedObject);
    feedObject.insert(u"strippedTitle"_s, Utils::stripTags(feed->title()));
    QString numberOfArticle;
    if (feed->unread() == 0) {
        numberOfArticle = i18n(" (no unread articles)");
    } else {
        numberOfArticle = i18np(" (1 unread article)", " (%1 unread articles)", feed->unread());
    }

    feedObject.insert(u"feedCount"_s, numberOfArticle);

    QString feedImage;
    if (!feed->logoInfo().imageUrl.isEmpty()) { // image
        feedImage = GrantleeUtil::imageFeed(feed);
    } else {
        feedImage = u"<div class=\"body\">"_s;
    }
    feedObject.insert(u"feedImage"_s, feedImage);

    if (!feed->description().isEmpty()) {
        QString feedDescription = u"<div dir=\"%1\">"_s.arg(mDirectionString);
        feedDescription += i18n("<b>Description:</b> %1<br />", feed->description());
        feedDescription += u"</div>"_s; // /description
        feedObject.insert(u"feedDescription"_s, feedDescription);
    }

    if (!feed->htmlUrl().isEmpty()) {
        QString feedHomePage = u"<div dir=\"%1\">"_s.arg(mDirectionString);
        feedHomePage += i18n("<b>Homepage:</b> <a href=\"%1\">%1</a>", feed->htmlUrl());
        feedHomePage += u"</div>"_s; // / link
        feedObject.insert(u"feedHomePage"_s, feedHomePage);
    }

    if (!feed->copyright().isEmpty()) {
        QString feedCopyright = u"<div dir=\"%1\">"_s.arg(mDirectionString);
        feedCopyright += i18n("<b>Copyright:</b> %1</a>", feed->copyright());
        feedCopyright += u"</div>"_s;
        feedObject.insert(u"feedCopyright"_s, feedCopyright);
    }

    KTextTemplate::Context context(feedObject);
    context.setLocalizer(mEngine.localizer());

    const QString contentHtml = mTemplate->render(&context);
    return contentHtml;
}

QString GrantleeViewFormatter::formatFolder(Akregator::Folder *node)
{
    mTemplate = mEngine.loadByName(u"formatter/html/defaultnormalvisitfolder.html"_s);
    if (mTemplate->error()) {
        return mTemplate->errorString();
    }
    QVariantHash folderObject;
    addStandardObject(folderObject);

    folderObject.insert(u"nodeTitle"_s, node->title());
    QString numberOfArticle;
    if (node->unread() == 0) {
        numberOfArticle = i18n(" (no unread articles)");
    } else {
        numberOfArticle = i18np(" (1 unread article)", " (%1 unread articles)", node->unread());
    }

    folderObject.insert(u"nodeCount"_s, numberOfArticle);
    KTextTemplate::Context context(folderObject);
    context.setLocalizer(mEngine.localizer());

    const QString contentHtml = mTemplate->render(&context);
    return contentHtml;
}

QString GrantleeViewFormatter::formatArticles(const QList<Article> &article, ArticleFormatter::IconOption icon)
{
    mTemplate = mEngine.loadByName(u"formatter/html/normalview.html"_s);
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
    articleObject.insert(u"articles"_s, articlesList);

    addStandardObject(articleObject);
    articleObject.insert(u"loadExternalReference"_s, Settings::loadExternalReferences());
    articleObject.insert(u"dateI18n"_s, i18n("Date"));
    articleObject.insert(u"commentI18n"_s, i18n("Comment"));
    articleObject.insert(u"completeStoryI18n"_s, i18n("Complete Story"));
    articleObject.insert(u"authorI18n"_s, i18n("Author"));
    articleObject.insert(u"enclosureI18n"_s, i18n("Enclosure"));

    KTextTemplate::Context context(articleObject);
    context.setLocalizer(mEngine.localizer());

    const QString contentHtml = mTemplate->render(&context);
    qDeleteAll(lstObj);
    return contentHtml;
}
