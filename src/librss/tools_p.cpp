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

#include <kcharsets.h>
#include <kglobal.h>

#include <qdom.h>

QString RSS::extractNode(const QDomNode &parent, const QString &elemName)
{
	QDomNode node = parent.namedItem(elemName);
	if (node.isNull())
		return QString::null;

	QString result = node.toElement().text().simplifyWhiteSpace();
	if (result.isEmpty())
		return QString::null;

	return result;
}

// vim:noet:ts=4
