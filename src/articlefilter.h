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

#include <qstring.h>
#include <qvaluelist.h>
#include "myarticle.h"

class KConfig;

namespace Akregator
{


           
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

        enum Type
        {
            String = 1,
            Int
        };


        Criterion();
        Criterion( Subject subject, Predicate predicate, const QString &object );
        Criterion( Subject subject, Predicate predicate, int object );
        
        bool satisfiedBy( const MyArticle &article ) const;

        Subject subject() const;
        Predicate predicate() const;
        QString object() const;

    private:
        Subject m_subject;
        Predicate m_predicate;
        QString m_object;
        int m_intObject;
        Type m_type;
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

        void readConfig( KConfig *cfg, const QString &name );
        void writeConfig( KConfig *cfg, const QString &name ) const;

        Action action() const;

    private:
        bool anyCriterionMatches( const MyArticle &a ) const;
        bool allCriteriaMatch( const MyArticle &a ) const;

        QValueList<Criterion> m_criteria;
        Association m_association;
        Action m_action;
};

}

#endif
