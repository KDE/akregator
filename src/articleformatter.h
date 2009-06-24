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

#ifndef AKREGATOR_ARTICLEFORMATTER_H
#define AKREGATOR_ARTICLEFORMATTER_H

#include <KUrl>

#include <boost/shared_ptr.hpp>

class QPaintDevice;
class QString;

namespace KRss {
    class FeedList;
    class Item;
    class TreeNode;
}

namespace Akregator {

class ArticleFormatter
{
    public:

        enum IconOption {
            NoIcon,
            ShowIcon
        };

        explicit ArticleFormatter( QPaintDevice* device = 0 );

        virtual ~ArticleFormatter();

        void setPaintDevice(QPaintDevice* device);

        virtual QString formatItem( const boost::shared_ptr<const KRss::FeedList>& fl, const KRss::Item& item, IconOption icon ) const = 0;

        virtual QString formatSummary( const boost::shared_ptr<const KRss::FeedList>& fl, const boost::shared_ptr<KRss::TreeNode>& node ) const = 0;

        virtual QString getCss() const = 0;

    protected:

        int pointsToPixel(int pointSize) const;

    private:
        class Private;
        Private* const d;
        Q_DISABLE_COPY( ArticleFormatter )
};

class DefaultNormalViewFormatter : public ArticleFormatter
{
    public:

        explicit DefaultNormalViewFormatter( const KUrl& imageDir, QPaintDevice* device = 0 );
        ~DefaultNormalViewFormatter();

        QString formatItem( const boost::shared_ptr<const KRss::FeedList>& fl, const KRss::Item& item, IconOption option ) const;

        /* reimp */ QString formatSummary( const boost::shared_ptr<const KRss::FeedList>& fl, const boost::shared_ptr<KRss::TreeNode>& node ) const;;

        QString getCss() const;

    private:
        DefaultNormalViewFormatter();

        KUrl m_imageDir;
        class SummaryVisitor;
        class SummaryFeedVisitor;
};

class DefaultCombinedViewFormatter : public ArticleFormatter
{

    public:

        explicit DefaultCombinedViewFormatter( const KUrl& imageDir, QPaintDevice* device = 0 );

        QString formatItem( const boost::shared_ptr<const KRss::FeedList>& fl, const KRss::Item& item, IconOption option ) const;

        /* reimp */ QString formatSummary( const boost::shared_ptr<const KRss::FeedList>& fl, const boost::shared_ptr<KRss::TreeNode>& node ) const;;

        QString getCss() const;

    private:
        DefaultCombinedViewFormatter();

        KUrl m_imageDir;
};

} // namespace Akregator

#endif // AKREGATOR_ARTICLEFORMATTER_H
