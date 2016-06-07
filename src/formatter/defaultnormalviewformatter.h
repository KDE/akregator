/*
   Copyright (C) 2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef DEFAULTNORMALVIEWFORMATTER_H
#define DEFAULTNORMALVIEWFORMATTER_H

#include "articleformatter.h"
#include "akregator_export.h"
class QPaintDevice;
namespace Akregator
{
class GrantleeViewFormatter;
class AKREGATOR_EXPORT DefaultNormalViewFormatter : public ArticleFormatter
{
public:

    explicit DefaultNormalViewFormatter(const QString &grantleeDirectory, const QUrl &imageDir, QPaintDevice *device = Q_NULLPTR);
    ~DefaultNormalViewFormatter();

    QString formatArticles(const QVector<Article> &article, IconOption option) const Q_DECL_OVERRIDE;

    QString formatSummary(TreeNode *node) const Q_DECL_OVERRIDE;

private:
    DefaultNormalViewFormatter();

    QUrl m_imageDir;
    QString m_DefaultThemePath;
    class SummaryVisitor;
    SummaryVisitor *m_summaryVisitor;
    GrantleeViewFormatter *mGrantleeViewFormatter;
};
}
#endif // DEFAULTNORMALVIEWFORMATTER_H
