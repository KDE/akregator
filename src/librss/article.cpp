/*
 * article.cpp
 *
 * Copyright (c) 2001, 2002, 2003, 2004 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "article.h"
#include "tools_p.h"

#include <krfcdate.h>
#include <kurl.h>
#include <kurllabel.h>

#include <qdatetime.h>
#include <qdom.h>

using namespace RSS;

struct Article::Private : public Shared
{
	QString title;
	KURL link;
	QString description;
	QDateTime pubDate;
	QString guid;
	bool guidIsPermaLink;
};

Article::Article() : d(new Private)
{
}

Article::Article(const Article &other) : d(0)
{
	*this = other;
}

Article::Article(const QDomNode &node) : d(new Private)
{
	QString elemText;
	
	if (!(elemText = extractNode(node, QString::fromLatin1("title"))).isNull())
		d->title = elemText;
	if (!(elemText = extractNode(node, QString::fromLatin1("link"))).isNull())
		d->link = elemText;
	if (!(elemText = extractNode(node, QString::fromLatin1("description"))).isNull())
		d->description = elemText;
	if (!(elemText = extractNode(node, QString::fromLatin1("pubDate"))).isNull()) {
		time_t _time = KRFCDate::parseDate(elemText);
		/* \bug This isn't really the right way since it will set the date to
		 * Jan 1 1970, 1:00:00 if the passed date was invalid; this means that
		 * we cannot distinguish between that date, and invalid values. :-/
		 */
		d->pubDate.setTime_t(_time);
	}
	if (!(elemText = extractNode(node, QString::fromLatin1("dc:date"))).isNull()) {
        time_t _time = KRFCDate::parseDateISO8601(elemText);
        /* \bug This isn't really the right way since it will set the date to
         * Jan 1 1970, 1:00:00 if the passed date was invalid; this means that
         * we cannot distinguish between that date, and invalid values. :-/
         */
        d->pubDate.setTime_t(_time);
    }

	QDomNode n = node.namedItem(QString::fromLatin1("guid"));
	if (!n.isNull()) {
		d->guidIsPermaLink = true;
		if (n.toElement().attribute(QString::fromLatin1("isPermaLink"), "true") == "false") d->guidIsPermaLink = false;

		if (!(elemText = extractNode(node, QString::fromLatin1("guid"))).isNull())
			d->guid = elemText;
	}
}

Article::~Article()
{
	if (d->deref())
		delete d;
}

QString Article::title() const
{
	return d->title;
}

const KURL &Article::link() const
{
	return d->link;
}

QString Article::description() const
{
	return d->description;
}

QString Article::guid() const
{
	return d->guid;
}

bool Article::guidIsPermaLink() const
{
	return d->guidIsPermaLink;
}

const QDateTime &Article::pubDate() const
{
	return d->pubDate;
}

KURLLabel *Article::widget(QWidget *parent, const char *name) const
{
	KURLLabel *label = new KURLLabel(d->link.url(), d->title, parent, name);
	label->setUseTips(true);
	if (!d->description.isNull())
		label->setTipText(d->description);
	
	return label;
}

Article &Article::operator=(const Article &other)
{
	if (this != &other) {
		other.d->ref();
		if (d && d->deref())
			delete d;
		d = other.d;
	}
	return *this;
}

bool Article::operator==(const Article &other) const
{
	return d->title == other.title() &&
		d->link == other.link() &&
		d->description == other.description() &&
		d->pubDate == other.pubDate() &&
		d->guid == other.guid() &&
		d->guidIsPermaLink == other.guidIsPermaLink();
}

// vim:noet:ts=4
