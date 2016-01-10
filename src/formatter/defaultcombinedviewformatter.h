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

#ifndef DEFAULTCOMBINEDVIEWFORMATTER_H
#define DEFAULTCOMBINEDVIEWFORMATTER_H

#include "articleformatter.h"
#include "akregator_export.h"
namespace Akregator
{
class AKREGATOR_EXPORT DefaultCombinedViewFormatter : public ArticleFormatter
{
public:
    explicit DefaultCombinedViewFormatter(const QUrl &imageDir, QPaintDevice *device = Q_NULLPTR);

    QString formatArticle(const Article &article, IconOption option) const Q_DECL_OVERRIDE;

    QString formatSummary(TreeNode *node) const Q_DECL_OVERRIDE;

    QString getCss() const Q_DECL_OVERRIDE;

private:
    DefaultCombinedViewFormatter();

    QUrl m_imageDir;
};
}
#endif // DEFAULTCOMBINEDVIEWFORMATTER_H
