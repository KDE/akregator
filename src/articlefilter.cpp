/*
 * Copyright (c) 2004 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */

#include "articlefilter.h"
#include "myarticle.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kurl.h>

#include <qregexp.h>

using namespace Akregator;

Criterion::Criterion()
{
}

Criterion::Criterion( Subject subject, Predicate predicate, const QVariant &object )
    : m_subject( subject )
    , m_predicate( predicate )
    , m_object( object )
{
}

bool Criterion::satisfiedBy( const MyArticle &article ) const
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

ArticleFilter::ArticleFilter()
{
}

ArticleFilter::ArticleFilter( const QValueList<Criterion> &criteria, Association assoc, Action action )
    : m_criteria( criteria )
    , m_association( assoc )
    , m_action( action )
{
}

bool ArticleFilter::matches( const MyArticle &a ) const
{
    switch ( m_association ) {
        case LogicalOr:
            return anyCriterionMatches( a );
        case LogicalAnd:
            return allCriteriaMatch( a );
    }
    return true;
}

ArticleFilter::Action ArticleFilter::action() const
{
    return m_action;
}

bool ArticleFilter::operator==(const ArticleFilter &other) const
{
    return m_action == other.m_action && m_association == other.m_association && m_criteria == other.m_criteria; 
}
bool ArticleFilter::operator!=(const ArticleFilter &other) const
{ return !(*this == other); }

bool ArticleFilter::anyCriterionMatches( const MyArticle &a ) const
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

bool ArticleFilter::allCriteriaMatch( const MyArticle &a ) const
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

