/*
 * filtermgr.h
 *
 * Copyright (c) 2004 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */

#ifndef ARTICLEFILTER_H
#define ARTICLEFILTER_H

#include <qvaluelist.h>
#include <qvariant.h>

class KConfig;

namespace Akregator
{
class MyArticle;
           
class Criterion
{
    public:

        enum Subject {
            Title, Description, Link, Status
        };

        enum Predicate {
            Contains = 0x01,
            Equals = 0x02,
            Matches = 0x03,
            Negation = 0x80
        };


        Criterion();
        Criterion( Subject subject, Predicate predicate, const QVariant &object );
        
        bool satisfiedBy( const MyArticle &article ) const;

        Subject subject() const;
        Predicate predicate() const;
        QVariant object() const;
        bool operator==(const Criterion& other) const
        { return m_subject == other.m_subject && m_predicate == other.m_predicate && m_object == other.m_object; }
        
    private:
        Subject m_subject;
        Predicate m_predicate;
        QVariant m_object;
};

class ArticleFilter
{
    public:
        enum Action {
            Hide, Show, Notify
        };

        enum Association {
            LogicalAnd, LogicalOr
        };

        ArticleFilter();
        ArticleFilter( const QValueList<Criterion> &criteria, Association assoc, Action action );

        bool matches( const MyArticle &article ) const;
        Action action() const;
        bool operator==(const ArticleFilter &other) const;
        bool operator!=(const ArticleFilter &other) const;
    private:
        bool anyCriterionMatches( const MyArticle &a ) const;
        bool allCriteriaMatch( const MyArticle &a ) const;

        QValueList<Criterion> m_criteria;
        Association m_association;
        Action m_action;
};

}

#endif
