/*
 * tools_p.h
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef LIBRSS_TOOLS_P_H
#define LIBRSS_TOOLS_P_H

#include "global.h"
#include <time.h>


class QDomNode;
class QDomElement;
class QString;

namespace RSS
{
	struct Shared
	{
		Shared() : count(1) { }
		void ref() { count++; }
		bool deref() { return !--count; }
		unsigned int count;
	};

	QString extractNode(const QDomNode &parent, const QString &elemName, bool isInlined=true);
	QString extractTitle(const QDomNode &parent);
	QString childNodesAsXML(const QDomNode& parent);
	time_t parseISO8601Date(const QString &s);
        QString parseItemAuthor(const QDomElement& element, Format format, Version version);
}

#endif // LIBRSS_TOOLS_P_H
// vim:noet:ts=4
