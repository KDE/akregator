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
#include "articlegrantleeobject.h"
#include "utils.h"
#include <KLocalizedString>

#include <grantlee/engine.h>

#include <QVariantHash>
#include <QApplication>
#include <QDateTime>
#include <QVariantList>

using namespace Akregator;

GrantleeViewFormatter::GrantleeViewFormatter(const QString &themePath, QObject *parent)
    : PimCommon::GenericGrantleeFormatter(QStringLiteral("main.html"), themePath, parent)
{
}

GrantleeViewFormatter::~GrantleeViewFormatter()
{

}

QString GrantleeViewFormatter::formatArticle(const QVector<Article> &article, ArticleFormatter::IconOption icon) const
{
    if (!errorMessage().isEmpty()) {
        return errorMessage();
    }

    QVariantHash articleObject;

    QVariantList articlesList;
    const int nbArticles(article.count());
    articlesList.reserve(nbArticles);
    for (int i = 0; i < nbArticles; ++i) {
        ArticleGrantleeObject *articleObj = new ArticleGrantleeObject(article.at(i), icon);
        articlesList << QVariant::fromValue(static_cast<QObject *>(articleObj));

    }
    articleObject.insert(QStringLiteral("article"), articlesList);

    const QString directionString = QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr");
    articleObject.insert(QStringLiteral("applicationDir"), directionString);

    articleObject.insert(QStringLiteral("dateI18n"), i18n("Date"));
    articleObject.insert(QStringLiteral("commentI18n"), i18n("Comment"));
    articleObject.insert(QStringLiteral("completeStoryI18n"), i18n("Complete Story"));
    articleObject.insert(QStringLiteral("authorI18n"), i18n("Author"));
    articleObject.insert(QStringLiteral("enclosureI18n"), i18n("Enclosure"));
#if 0
    const QString strippedTitle = Akregator::Utils::stripTags(article.title());
    articleObject.insert(QStringLiteral("strippedTitle"), strippedTitle);
    const QString author = article.authorAsHtml();
    if (!author.isEmpty()) {
        articleObject.insert(QStringLiteral("author"), author);
    }
    const QString enc = formatEnclosure(*article.enclosure());
    if (!enc.isEmpty()) {
        articleObject.insert(QStringLiteral("enclosure"), enc);
    }
    if (article.pubDate().isValid()) {

    }

    const QString content = article.content(Article::DescriptionAsFallback);
    if (!content.isEmpty()) {
        articleObject.insert(QStringLiteral("content"), content);
    }

    QString text;
    const QString directionString = QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr");
    text = QStringLiteral("<div class=\"headerbox\" dir=\"%1\">\n").arg(directionString);
    const QString enc = formatEnclosure(*article.enclosure());

    const QString strippedTitle = Utils::stripTags(article.title());
    if (!strippedTitle.isEmpty()) {
        text += QStringLiteral("<div class=\"headertitle\" dir=\"%1\">\n").arg(directionString);
        if (article.link().isValid()) {
            text += QLatin1String("<a href=\"") + article.link().url() + QLatin1String("\">");
        }
        text += strippedTitle;
        if (article.link().isValid()) {
            text += QLatin1String("</a>");
        }
        text += QStringLiteral("</div>\n");
    }
    if (article.pubDate().isValid()) {
        text += QStringLiteral("<span class=\"header\" dir=\"%1\">").arg(directionString);
        text += QStringLiteral("%1:").arg(i18n("Date"));
        text += QLatin1String("</span><span class=\"headertext\">");
        text += KLocale::global()->formatDateTime(article.pubDate(), KLocale::FancyLongDate) + QLatin1String("</span>\n"); // TODO: might need RTL?
    }
    const QString author = article.authorAsHtml();
    if (!author.isEmpty()) {
        text += QStringLiteral("<br/><span class=\"header\" dir=\"%1\">").arg(directionString);
        text += QStringLiteral("%1:").arg(i18n("Author"));
        text += QLatin1String("</span><span class=\"headertext\">");
        text += author + QLatin1String("</span>\n"); // TODO: might need RTL?
    }

    if (!enc.isEmpty()) {
        text += QStringLiteral("<br/><span class=\"header\" dir=\"%1\">").arg(directionString);
        text += QStringLiteral("%1:").arg(i18n("Enclosure"));
        text += QLatin1String("</span><span class=\"headertext\">");
        text += enc + QLatin1String("</span>\n"); // TODO: might need RTL?
    }

    text += QStringLiteral("</div>\n"); // end headerbox

    if (icon == ShowIcon && article.feed() && !article.feed()->image().isNull()) {
        const Feed *feed = article.feed();
        QString file = Utils::fileNameForUrl(feed->xmlUrl());
        QUrl u(m_imageDir);
        u = u.adjusted(QUrl::RemoveFilename);
        u.setPath(u.path() + file);
        text += QStringLiteral("<a href=\"%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(feed->htmlUrl(), u.url());
    }

    const QString content = article.content(Article::DescriptionAsFallback);
    if (!content.isEmpty()) {
        text += QStringLiteral("<div dir=\"%1\">").arg(Utils::directionOf(Utils::stripTags(content)));
        text += QLatin1String("<span class=\"content\">") + content + QLatin1String("</span>");
        text += QLatin1String("</div>");
    }

    text += QLatin1String("<div class=\"body\">");

    if (article.commentsLink().isValid()) {
        text += QLatin1String("<a class=\"contentlink\" href=\"");
        text += article.commentsLink().url();
        text += QLatin1String("\">") + i18n("Comments");
        if (article.comments()) {
            text += QLatin1String(" (") + QString::number(article.comments()) + QLatin1Char(')');
        }
        text += QLatin1String("</a>");
    }

    if (!enc.isEmpty()) {
        text += QStringLiteral("<p><em>%1</em> %2</p>").arg(i18n("Enclosure:")).arg(enc);
    }

    if (article.link().isValid() || (article.guidIsPermaLink() && QUrl(article.guid()).isValid())) {
        text += QLatin1String("<p><a class=\"contentlink\" href=\"");
        // in case link isn't valid, fall back to the guid permaLink.
        if (article.link().isValid()) {
            text += article.link().url();
        } else {
            text += article.guid();
        }
        text += QLatin1String("\">") + i18n("Complete Story") + QLatin1String("</a></p>");
    }

    text += QLatin1String("</div>");

    return text;

#endif


    QVariantHash mapping;
    mapping.insert(QStringLiteral("article"), articleObject);
    return render(mapping);
}

