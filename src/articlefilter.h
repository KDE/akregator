/*
 * articlefilter.h
 *
 * Copyright (c) 2004, 2005 Frerich Raabe <raabe@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
            Title, Description, Link, Status, KeepFlag
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
            NoAction, Hide, Show, Notify
        };

        enum Association {
            None, LogicalAnd, LogicalOr
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
