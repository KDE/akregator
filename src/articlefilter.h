/*
 * articlefilter.h
 *
 * Copyright (c) 2004, 2005 Frerich Raabe <raabe@kde.org>
 *               2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
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
#ifndef AKREGATOR_FILTERS_ARTICLEFILTER_H
#define AKREGATOR_FILTERS_ARTICLEFILTER_H

#include "akregator_export.h"
#include <QList>
#include <QString>
#include <QVariant>

class KConfig;

namespace Akregator {

class Article;

namespace Filters {

class AbstractAction;
class AbstractMatcher;
class Criterion;

/** an article filter, basically a matcher and an action. 
 *  @author Frank Osterfeld
 */
class AKREGATOR_EXPORT ArticleFilter
{
    public:

        ArticleFilter();
        ArticleFilter(const AbstractMatcher& matcher, const AbstractAction& action);
        ArticleFilter(const ArticleFilter& other);

        virtual ~ArticleFilter();

        /** checks whether an article matches the matcher, and executes the action if so */
        void applyTo(Article& article) const;

        
        
        /** name of the filter, for display in filter list */
        const QString& name() const;
        void setName(const QString& name);

        int id() const;

        AbstractMatcher* matcher() const;
        void setMatcher(const AbstractMatcher& matcher);

        AbstractAction* action() const;
        void setAction(const AbstractAction& action);

        ArticleFilter& operator=(const ArticleFilter& other);
        bool operator==(const ArticleFilter& other) const;

        void writeConfig(KConfig* config) const;
        void readConfig(KConfig* config);

    private:
        class ArticleFilterPrivate;
        ArticleFilterPrivate* d;
    
};

class AKREGATOR_EXPORT ArticleFilterList : public QList<ArticleFilter>
{
public:
    
    void writeConfig(KConfig* config) const;
    void readConfig(KConfig* config);
};

/** Abstract base class for matchers, a matcher just takes an article and checks whether the article matches some criterion or not. 
 *  @author Frank Osterfeld
 */
class AKREGATOR_EXPORT AbstractMatcher
{
    public:

        virtual ~AbstractMatcher() {}
        /** returns a copy of the matcher */
        virtual AbstractMatcher* clone() const = 0;

        virtual bool matches(const Article& article) const = 0;

        virtual void writeConfig(KConfig* config) const = 0;
        virtual void readConfig(KConfig* config) = 0;

        virtual bool operator==(const AbstractMatcher&) const = 0;
        virtual bool operator!=(const AbstractMatcher &other) const = 0;
};

class AKREGATOR_EXPORT AbstractAction
{
    public:
        virtual void exec(Article& article) = 0;
		virtual ~AbstractAction(){}
        virtual void writeConfig(KConfig* config) const = 0;
        virtual void readConfig(KConfig* config) = 0;

        virtual AbstractAction* clone() const = 0;
        virtual bool operator==(const AbstractAction& other) = 0;
};

class AKREGATOR_EXPORT DeleteAction : public AbstractAction
{
    public:
        virtual void exec(Article& article);
       	virtual ~DeleteAction(){} 
        virtual void writeConfig(KConfig* config) const;
        virtual void readConfig(KConfig* config);

        virtual DeleteAction* clone() const { return new DeleteAction; }
        virtual bool operator==(const AbstractAction& other);
};

class AKREGATOR_EXPORT SetStatusAction : public AbstractAction
{
    public:
        SetStatusAction(int status=0);
		virtual ~SetStatusAction(){}
        
        virtual void exec(Article& article);
        
        int status() const;
        void setStatus(int status);

        virtual void writeConfig(KConfig* config) const;
        virtual void readConfig(KConfig* config);

        virtual SetStatusAction* clone() const { return new SetStatusAction(*this); }
        virtual bool operator==(const AbstractAction& other);

    private:
        int m_status;
};

/** a powerful matcher supporting multiple criterions, which can be combined      via logical OR or AND
 *  @author Frerich Raabe
 */
class AKREGATOR_EXPORT ArticleMatcher : public AbstractMatcher
{
    public:

        enum Association {
            None, LogicalAnd, LogicalOr
        };

        ArticleMatcher();
        ArticleMatcher( const QList<Criterion> &criteria, Association assoc);
        
        ArticleMatcher(const ArticleMatcher& other);
        virtual ~ArticleMatcher();

        ArticleMatcher& operator=(const ArticleMatcher& other);
        virtual ArticleMatcher* clone() const;
        virtual bool matches(const Article &article) const;
        virtual bool operator==(const AbstractMatcher &other) const;
        virtual bool operator!=(const AbstractMatcher &other) const;
        
        
        virtual void writeConfig(KConfig* config) const;
        virtual void readConfig(KConfig* config);

    private:

        static Association stringToAssociation(const QString& assocStr);
        static QString associationToString(Association association);

        bool anyCriterionMatches( const Article &a ) const;
        bool allCriteriaMatch( const Article &a ) const;

        QList<Criterion> m_criteria;
        Association m_association;
};

/** Criterion for ArticleMatcher
 *  @author Frerich Raabe
 */
class AKREGATOR_EXPORT Criterion
{
    public:

        enum Subject {
            Title, Description, Link, Status, KeepFlag
        };

        static QString subjectToString(Subject subj);
        static Subject stringToSubject(const QString& subjStr);

        enum Predicate {
            Contains = 0x01,
            Equals = 0x02,
            Matches = 0x03,
            Negation = 0x80
        };

        static QString predicateToString(Predicate pred);
        static Predicate stringToPredicate(const QString& predStr);
    
        Criterion();
        Criterion( Subject subject, Predicate predicate, const QVariant &object );
        virtual ~Criterion(){} 
        bool satisfiedBy( const Article &article ) const;

        virtual void writeConfig(KConfig* config) const;
        virtual void readConfig(KConfig* config);

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

} // namespace Filters
} // namespace Akregator

#endif // AKREGATOR_FILTERS_ARTICLEFILTER_H
