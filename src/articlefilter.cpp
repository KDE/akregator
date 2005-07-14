/*
 * articlefilter.cpp
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
#include "articlefilter.h"
#include "article.h"
#include "tag.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kurl.h>

#include <qregexp.h>

namespace Akregator {

Criterion::Criterion()
{
}

Criterion::Criterion( Subject subject, Predicate predicate, const QVariant &object )
    : m_subject( subject )
    , m_predicate( predicate )
    , m_object( object )
{
}

bool Criterion::satisfiedBy( const Article &article ) const
{
    QVariant concreteSubject;

    switch ( m_subject ) {
        case Title:
            concreteSubject = QVariant(article.title());
            break;
        case Description:
            concreteSubject = QVariant(article.description());
            break;
        case Link:
            // ### Maybe use prettyURL here?
            concreteSubject = QVariant(article.link().url());
            break;
        case Status:
            concreteSubject = QVariant(article.status());
            break;
        case KeepFlag:
            concreteSubject = QVariant(article.keep());   
        default:
            break;
    }

    bool satisfied = false;

    const Predicate predicateType = static_cast<Predicate>( m_predicate & ~Negation );
	QString subjectType=concreteSubject.typeName();

    switch ( predicateType ) {
        case Contains:
            satisfied = concreteSubject.toString().find( m_object.toString(), 0, false ) != -1;
            break;
        case Equals:
            if (subjectType=="int")
                satisfied = concreteSubject.toInt() == m_object.toInt();
            else
                satisfied = concreteSubject.toString() == m_object.toString();
            break;
        case Matches:
            satisfied = QRegExp( m_object.toString() ).search( concreteSubject.toString() ) != -1;
            break;
        default:
            kdDebug() << "Internal inconsistency; predicateType should never be Negation" << endl;
            break;
    }

    if ( m_predicate & Negation ) {
        satisfied = !satisfied;
    }

    return satisfied;
}

Criterion::Subject Criterion::subject() const
{
    return m_subject;
}

Criterion::Predicate Criterion::predicate() const
{
    return m_predicate;
}

QVariant Criterion::object() const
{
    return m_object;
}

ArticleMatcher::ArticleMatcher()
    : m_association( None )
{
}

ArticleMatcher* ArticleMatcher::clone() const
{
    return new ArticleMatcher(*this);
}

ArticleMatcher::ArticleMatcher( const QValueList<Criterion> &criteria, Association assoc)
    : m_criteria( criteria )
    , m_association( assoc )
{
}

bool ArticleMatcher::matches( const Article &a ) const
{
    switch ( m_association ) {
        case LogicalOr:
            return anyCriterionMatches( a );
        case LogicalAnd:
            return allCriteriaMatch( a );
        default:
            break;
    }
    return true;
}

bool ArticleMatcher::operator==(const AbstractMatcher& other) const
{
    AbstractMatcher* ptr = const_cast<AbstractMatcher*>(&other);
    ArticleMatcher* o = dynamic_cast<ArticleMatcher*>(ptr);
    if (!o)
        return false;
    else
        return m_association == o->m_association && m_criteria == o->m_criteria;
}
bool ArticleMatcher::operator!=(const AbstractMatcher& other) const
{
    return !(*this == other);
}

bool ArticleMatcher::anyCriterionMatches( const Article &a ) const
{
    if (m_criteria.count()==0)
        return true;
    QValueList<Criterion>::ConstIterator it = m_criteria.begin();
    QValueList<Criterion>::ConstIterator end = m_criteria.end();
    for ( ; it != end; ++it ) {
        if ( ( *it ).satisfiedBy( a ) ) {
            return true;
        }
    }
    return false;
}

bool ArticleMatcher::allCriteriaMatch( const Article &a ) const
{
    if (m_criteria.count()==0)
        return true;
    QValueList<Criterion>::ConstIterator it = m_criteria.begin();
    QValueList<Criterion>::ConstIterator end = m_criteria.end();
    for ( ; it != end; ++it ) {
        if ( !( *it ).satisfiedBy( a ) ) {
            return false;
        }
    }
    return true;
}

class TagMatcher::TagMatcherPrivate
{
    public:
    Tag tag;
    bool operator==(const TagMatcherPrivate& other) const
    {
        return tag == other.tag;
    }
};

TagMatcher::TagMatcher(const Tag& tag) : d(new TagMatcherPrivate)
{
    d->tag = tag;
}

TagMatcher::TagMatcher() : d(new TagMatcherPrivate)
{
}

TagMatcher::~TagMatcher()
{
    delete d;
    d = 0;
}

bool TagMatcher::matches(const Article& article) const
{
    return article.hasTag(d->tag.id());
}

TagMatcher* TagMatcher::clone() const
{
    return new TagMatcher(*this);
}


TagMatcher::TagMatcher(const TagMatcher& other) : AbstractMatcher(other), d(0)
{
    *this = other;
}

bool TagMatcher::operator==(const AbstractMatcher& other) const
{
    AbstractMatcher* ptr = const_cast<AbstractMatcher*>(&other);
    TagMatcher* tagFilter = dynamic_cast<TagMatcher*>(ptr);
    return tagFilter ? *d == *(tagFilter->d) : false;
}

bool TagMatcher::operator!=(const AbstractMatcher &other) const
{
    return !(*this == other);
}

TagMatcher& TagMatcher::operator=(const TagMatcher& other)
{
    d = new TagMatcherPrivate;
    *d = *(other.d);
    return *this;
}

} //namespace Akregator
