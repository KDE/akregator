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
#include "enclosure.h"
#include "category.h"

#include <kdebug.h>
#include <krfcdate.h>
#include <kurl.h>
#include <kurllabel.h>
#include <kcodecs.h> 

#include <qdatetime.h>
#include <qdom.h>
//Added by qt3to4:
#include <Q3ValueList>

using namespace RSS;
namespace RSS
{
    KMD5 md5Machine;
}

struct Article::Private : public Shared
{
	QString title;
	KURL link;
	QString description;
	QDateTime pubDate;
	QString guid;
	bool guidIsPermaLink;
    MetaInfoMap meta;
	KURL commentsLink;
        int numComments;
        Enclosure enclosure;
	Q3ValueList<Category> categories;
};

Article::Article() : d(new Private)
{
}

Article::Article(const Article &other) : d(0)
{
	*this = other;
}

Enclosure Article::enclosure() const
{
    return d->enclosure;
}

Q3ValueList<Category> Article::categories() const
{
    return d->categories;
}

Article::Article(const QDomNode &node, Format format, Version version) : d(new Private)
{
	QString elemText;

	d->numComments=0;

	if (!(elemText = extractTitle(node)).isNull())
		d->title = elemText;
   
	if (format==AtomFeed)
	{
		QDomNode n;
		for (n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
			const QDomElement e = n.toElement();
			if ( (e.tagName()==QString::fromLatin1("link")) &&
				(e.attribute(QString::fromLatin1("rel"))==QString::fromLatin1("alternate")))
				{   
					d->link=n.toElement().attribute(QString::fromLatin1("href"));
					break;
				}
		}
	}
	else
	{
		if (!(elemText = extractNode(node, QString::fromLatin1("link"))).isNull())
			d->link = elemText;
	}


    // prefer content/content:encoded over summary/description for feeds that provide it
    QString tagName=(format==AtomFeed)? QString::fromLatin1("content"): QString::fromLatin1("content:encoded");
    
    if (!(elemText = extractNode(node, tagName, false)).isNull())
        d->description = elemText;
    
    if (d->description.isEmpty())
    {
		if (!(elemText = extractNode(node, QString::fromLatin1("body"), false)).isNull())
			d->description = elemText;
    
		if (d->description.isEmpty())  // 3rd try: see http://www.intertwingly.net/blog/1299.html
		{
			if (!(elemText = extractNode(node, QString::fromLatin1((format==AtomFeed)? "summary" : "description"), false)).isNull())
				d->description = elemText;
		}
	}
    
	time_t time = 0;

	if (format == AtomFeed)
	{
        if (version == vAtom_1_0)
            elemText = extractNode(node, QString::fromLatin1("updated"));
        else
           elemText = extractNode(node, QString::fromLatin1("issued"));

		if (!elemText.isNull())
			time = parseISO8601Date(elemText); 	
	}
	else 
	{
		elemText = extractNode(node, QString::fromLatin1("pubDate"));
		if (!elemText.isNull())
			time = KRFCDate::parseDate(elemText);
	}
	
	if (!(elemText = extractNode(node, QString::fromLatin1("dc:date"))).isNull())
	{
		time = parseISO8601Date(elemText);
	}

	// 0 means invalid, not epoch (parsers return epoch+1 when parsing epoch, see the KRFCDate::parseDate() docs)
        if (time != 0)
		d->pubDate.setTime_t(time);

	if (!(elemText = extractNode(node, QString::fromLatin1("wfw:comment"))).isNull()) {
		d->commentsLink = elemText;
	}

    if (!(elemText = extractNode(node, QString::fromLatin1("slash:comments"))).isNull()) {
        d->numComments = elemText.toInt();
    }

    QDomElement element = QDomNode(node).toElement();

    // in RSS 1.0, we use <item about> attribute as ID
    // FIXME: pass format version instead of checking for attribute

    if (!element.isNull() && element.hasAttribute(QString::fromLatin1("rdf:about")))
    {
        d->guid = element.attribute(QString::fromLatin1("rdf:about")); // HACK: using ns properly did not work
        d->guidIsPermaLink = false;
    }
    else
    {
        tagName=(format==AtomFeed)? QString::fromLatin1("id"): QString::fromLatin1("guid");
        QDomNode n = node.namedItem(tagName);
	    if (!n.isNull())
        {
            d->guidIsPermaLink = (format==AtomFeed)? false : true;
            if (n.toElement().attribute(QString::fromLatin1("isPermaLink"), "true") == "false") d->guidIsPermaLink = false;
            if (!(elemText = extractNode(node, tagName)).isNull())
                d->guid = elemText;
        }
    }    

	if(d->guid.isEmpty()) {
		d->guidIsPermaLink = false;
        
		md5Machine.reset();
		QDomNode n(node);
		md5Machine.update(d->title.utf8());
		md5Machine.update(d->description.utf8());
		d->guid = QString(md5Machine.hexDigest().data());
        d->meta[QString::fromLatin1("guidIsHash")] = QString::fromLatin1("true");
	}

    QDomNode enclosure = element.namedItem(QString::fromLatin1("enclosure"));
    if (enclosure.isElement())
        d->enclosure = Enclosure::fromXML(enclosure.toElement());

    for (QDomNode i = node.firstChild(); !i.isNull(); i = i.nextSibling())
    {
        if (i.isElement())
        {
            if (i.toElement().tagName() == QString::fromLatin1("metaInfo:meta"))
            {
                QString type = i.toElement().attribute(QString::fromLatin1("type"));
                d->meta[type] = i.toElement().text();
            }
            else if (i.toElement().tagName() == QString::fromLatin1("category"))
            {
                d->categories.append(Category::fromXML(i.toElement()));
            }
        }
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

const KURL &Article::commentsLink() const
{
	return d->commentsLink;
}

int Article::comments() const
{
	return d->numComments;
}


QString Article::meta(const QString &key) const
{
    return d->meta[key];
}

KURLLabel *Article::widget(QWidget *parent, const char *name) const
{
	KURLLabel *label = new KURLLabel(d->link.url(), d->title, parent);
	label->setObjectName(name);
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
	return d->guid == other.guid();
}

// vim:noet:ts=4
