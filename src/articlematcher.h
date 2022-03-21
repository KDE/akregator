/*
 * articlematcher.h
 *
 * SPDX-FileCopyrightText: 2004, 2005 Frerich Raabe <raabe@kde.org>
 * SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#pragma once

#include "akregatorpart_export.h"
#include <QString>
#include <QVariant>
#include <QVector>

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
    enum Association { None, LogicalAnd, LogicalOr };

    ArticleMatcher();
    ArticleMatcher(const QVector<Criterion> &criteria, Association assoc);

    ~ArticleMatcher() override;

    bool matches(const Article &article) const override;
    bool operator==(const AbstractMatcher &other) const override;
    bool operator!=(const AbstractMatcher &other) const override;

    void writeConfig(KConfigGroup *config) const override;
    void readConfig(KConfigGroup *config) override;

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
    enum Subject { Title, Description, Link, Status, KeepFlag, Author };

    static QString subjectToString(Subject subj);
    static Subject stringToSubject(const QString &subjStr);

    enum Predicate { Contains = 0x01, Equals = 0x02, Matches = 0x03, Negation = 0x80 };

    static QString predicateToString(Predicate pred);
    static Predicate stringToPredicate(const QString &predStr);

    Criterion();
    Criterion(Subject subject, Predicate predicate, const QVariant &object);
    virtual ~Criterion() = default;

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
