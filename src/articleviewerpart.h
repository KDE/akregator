/*
    This file is part of Akregator.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef ARTICLEVIEWERPART_H
#define ARTICLEVIEWERPART_H

#include <khtml_part.h>

namespace Akregator
{
class ArticleViewerPart : public KHTMLPart
{
    Q_OBJECT

public:
    explicit ArticleViewerPart(QWidget *parent);

    bool closeUrl() Q_DECL_OVERRIDE;

    int button() const;

protected:

    /** reimplemented to get the mouse button */
    bool urlSelected(const QString &url, int button, int state, const QString &_target,
                     const KParts::OpenUrlArguments &args = KParts::OpenUrlArguments(),
                     const KParts::BrowserArguments &browserArgs = KParts::BrowserArguments()) Q_DECL_OVERRIDE;

private:

    int m_button;
};
}
#endif // ARTICLEVIEWERPART_H
