/*
 * tools_p.cpp
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "tools_p.h"

#include <krfcdate.h>
#include <qdom.h>

time_t RSS::parseISO8601Date(const QString &s)
{
	if (s.find('T') != -1)
		return KRFCDate::parseDateISO8601(s);
	QString time=s;
	time += "T12:00:00";
	return KRFCDate::parseDateISO8601(time);
}


QString RSS::extractNode(const QDomNode &parent, const QString &elemName, bool simplifyWhiteSpace)
{
	QDomNode node = parent.namedItem(elemName);
	if (node.isNull())
		return QString::null;

	QString result = node.toElement().text();
	if (simplifyWhiteSpace) {
		// MC : this line assumes that a feed containing no < has no html tags
		// and then have no formattig, so we should convert \n into <br> to make it readable
		// Mon, 23 Aug 2004 15:22:06 +0200
		if(!result.contains("<")) result=result.replace(QChar('\n'),"<br />");

		result=result.simplifyWhiteSpace();
	}
	if (result.isEmpty())
		return QString::null;

	return result;
}

// vim:noet:ts=4
