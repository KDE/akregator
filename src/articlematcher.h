/*
 * articlematcher.h
 *
 * Copyright (c) 2004, 2005 Frerich Raabe <raabe@kde.org>
 *               2005 Frank Osterfeld <osterfeld@kde.org>
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
#ifndef AKREGATOR_ARTICLEMATCHER_H
#define AKREGATOR_ARTICLEMATCHER_H

#include "akregatorpart_export.h"
#include <QVector>
#include <QString>
#include <QVariant>

class KConfigGroup;

namespace Akregator
{

class Article;

namespace Filters
{

class AbstractMatcher;
class Criterion;

/** Abstract base class for matchers, a matcher just takes an article and checks whether the article matches some criterion or not.
 *  @author Frank Osterfeld
 */
class AKREGATORPART_EXPORT AbstractMatcher
{
public:
    virtual ~AbstractMatcher();

    virtual bool matches(const Article &article) const = 0;

    virtual void writeConfig(KConfigGroup *config) const = 0;
    virtual void readConfig(KConfigGroup *config) = 0;

    virtual bool operator==(const AbstractMatcher &) const = 0;
    virtual bool operator!=(const AbstractMatcher &other) const = 0;

protected:
    AbstractMatcher();

private:
    Q_DISABLE_COPY(AbstractMatcher)

};

/** a powerful matcher supporting multiple criterions, which can be combined      via logical OR or AND
 *  @author Frerich Raabe
 */
class AKREGATORPART_EXPORT ArticleMatcher : public AbstractMatcher
{
public:

    enum Association {
        None, LogicalAnd, LogicalOr
    };

    ArticleMatcher();
    ArticleMatcher(const QVector<Criterion> &criteria, Association assoc);

    ~ArticleMatcher();

    bool matches(const Article &article) const Q_DECL_OVERRIDE;
    bool operator==(const AbstractMatcher &other) const Q_DECL_OVERRIDE;
    bool operator!=(const AbstractMatcher &other) const Q_DECL_OVERRIDE;

    void writeConfig(KConfigGroup *config) const Q_DECL_OVERRIDE;
    void readConfig(KConfigGroup *config) Q_DECL_OVERRIDE;

private:

    static Association stringToAssociation(const QString &assocStr);
    static QString associationToString(Association association);

    bool anyCriterionMatches(const Article &a) const;
    bool allCriteriaMatch(const Article &a) const;

    QVector<Criterion> m_criteria;
    Association m_association;
};

/** Criterion for ArticleMatcher
 *  @author Frerich Raabe
 */
class AKREGATORPART_EXPORT Criterion
{
public:

    enum Subject {
        Title, Description, Link, Status, KeepFlag, Author
    };

    static QString subjectToString(Subject subj);
    static Subject stringToSubject(const QString &subjStr);

    enum Predicate {
        Contains = 0x01,
        Equals = 0x02,
        Matches = 0x03,
        Negation = 0x80
    };

    static QString predicateToString(Predicate pred);
    static Predicate stringToPredicate(const QString &predStr);

    Criterion();
    Criterion(Subject subject, Predicate predicate, const QVariant &object);
    virtual ~Criterion() {}
    bool satisfiedBy(const Article &article) const;

    virtual void writeConfig(KConfigGroup *config) const;
    virtual void readConfig(KConfigGroup *config);

    Subject subject() const;
    Predicate predicate() const;
    QVariant object() const;
    bool operator==(const Criterion &other) const
    {
        return m_subject == other.m_subject && m_predicate == other.m_predicate && m_object == other.m_object;
    }

private:
    Subject m_subject;
    Predicate m_predicate;
    QVariant m_object;
};

} // namespace Filters
} // namespace Akregator

#endif // AKREGATOR_ARTICLEMATCHER_H
