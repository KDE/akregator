/*
    This file is part of Akregator.

    Copyright (C) 2006 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "articleformatter.h"
#include "akregatorconfig.h"
#include "article.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "treenodevisitor.h"
#include "utils.h"

#include <KLocalizedString>

#include <QApplication>
#include <QPaintDevice>
#include <QPalette>
#include <QString>
#include <KFormat>

using namespace Syndication;
using namespace Akregator;

class ArticleFormatter::Private
{
public:
    explicit Private(QPaintDevice *device_);
    QPaintDevice *device;
    class SummaryVisitor;
};

ArticleFormatter::Private::Private(QPaintDevice *device_)
    : device(device_)
{
}

ArticleFormatter::ArticleFormatter(QPaintDevice *device)
    : d(new Private(device))
{
}

ArticleFormatter::~ArticleFormatter()
{
    delete d;
}

void ArticleFormatter::setPaintDevice(QPaintDevice *device)
{
    d->device = device;
}

int ArticleFormatter::pointsToPixel(int pointSize) const
{
    return (pointSize * d->device->logicalDpiY() + 36) / 72;
}

QString ArticleFormatter::formatEnclosure(const Enclosure &enclosure)
{
    if (enclosure.isNull()) {
        return QString();
    }

    const QString title = !enclosure.title().isEmpty() ? enclosure.title() : enclosure.url();
    const uint length = enclosure.length();
    const QString type = enclosure.type();
    QString inf;
    if (!type.isEmpty() && length > 0) {
        inf = i18n("(%1, %2)", type, KFormat().formatByteSize(length));
    } else if (!type.isNull()) {
        inf = type;
    } else if (length > 0) {
        inf = KFormat().formatByteSize(length);
    }
    const QString str = i18n("<a href=\"%1\">%2</a> %3", enclosure.url(), title, inf);
    return str;
}

