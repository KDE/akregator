/*
 * Copyright (c) 2004 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */

#include "articlefilter.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kurl.h>

#include <qregexp.h>

using namespace Akregator;

Criterion::Criterion()
{
}

Criterion::Criterion( Subject subject, Predicate predicate, const QString &object )
    : m_subject( subject ),
    m_predicate( predicate ),
    m_object( object )
{
    m_type=String;
}

Criterion::Criterion( Subject subject, Predicate predicate, int object )
    : m_subject( subject ),
    m_predicate( predicate ),
    m_intObject( object )
{
    m_type=Int;
}

bool Criterion::satisfiedBy( const MyArticle &article ) const
{
    QString concreteSubject;
    int intSubj=0;

    switch ( m_subject ) {
        case Title:
            concreteSubject = article.title();
            break;
        case Description:
            concreteSubject = article.description();
            break;
        case Link:
            // ### Maybe use prettyURL here?
            concreteSubject = article.link().url();
            break;
        case Status:
            intSubj= article.status();
            break;
        default:
            break;
    }

    bool satisfied = false;

    const Predicate predicateType = static_cast<Predicate>( m_predicate & ~Negation );
    switch ( predicateType ) {
        case Contains:
            satisfied = concreteSubject.find( m_object, 0, false ) != -1;
            break;
        case Equals:
	    /// FIXME: use qvariant instead of this hack - smt
            if (m_type==Int)
                satisfied = intSubj == m_intObject;
            else
                satisfied = concreteSubject == m_object;
            break;
        case Matches:
            satisfied = QRegExp( m_object ).search( concreteSubject ) != -1;
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

QString Criterion::object() const
{
    return m_object;
}

ArticleFilter::ArticleFilter()
{
}

ArticleFilter::ArticleFilter( const QValueList<Criterion> &criteria, Association assoc, Action action )
    : m_criteria( criteria ),
    m_association( assoc ),
    m_action( action )
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

void ArticleFilter::writeConfig( KConfig *cfg, const QString &name ) const
{
    KConfigGroupSaver group( cfg, name );

    cfg->writeEntry( "Action", m_action );
    cfg->writeEntry( "Association", m_association );
    cfg->writeEntry( "Criteria", m_criteria.count() );

    QValueList<Criterion>::ConstIterator it;
    QValueList<Criterion>::ConstIterator en( m_criteria.end() );
    unsigned int count = 0;
    for (it = m_criteria.begin(); it != en; ++it, ++count ) {
        QStringList components;
        components << QString::number( ( *it ).subject() )
                   << QString::number( ( *it ).predicate() )
                   << ( *it ).object();
        cfg->writeEntry( QString( "Criterion_%1" ).arg( count ), components );
    }

    cfg->sync();
}

void ArticleFilter::readConfig( KConfig *cfg, const QString &name )
{
    KConfigGroupSaver group( cfg, name );

    m_action = static_cast<Action>( cfg->readUnsignedNumEntry( "Action" ) );
    m_association = static_cast<Association>( cfg->readUnsignedNumEntry( "Association" ) );

    const unsigned int criteria = cfg->readUnsignedNumEntry( "Criteria" );
    for ( unsigned int i = 0; i < criteria; ++i ) {
        const QStringList components =  cfg->readListEntry( QString( "Criterion_%1" ).arg( i ) );
        Criterion::Subject subject = static_cast<Criterion::Subject>( components[ 0 ].toUInt() );
        Criterion::Predicate predicate = static_cast<Criterion::Predicate>( components[ 1 ].toUInt() );
        m_criteria += Criterion( subject, predicate, components[ 2 ] );
    }
}

