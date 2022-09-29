/*
 * articlematcher.cpp
 *
 * SPDX-FileCopyrightText: 2004, 2005 Frerich Raabe <raabe@kde.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include "articlematcher.h"
#include "akregator_debug.h"
#include "article.h"
#include "utils/textutil.h"
#include <KConfig>
#include <KConfigGroup>
#include <QUrl>

#include <QRegularExpression>

namespace Akregator
{
namespace Filters
{
AbstractMatcher::AbstractMatcher() = default;

AbstractMatcher::~AbstractMatcher() = default;

QString Criterion::subjectToString(Subject subj)
{
    switch (subj) {
    case Title:
        return QStringLiteral("Title");
    case Link:
        return QStringLiteral("Link");
    case Description:
        return QStringLiteral("Description");
    case Status:
        return QStringLiteral("Status");
    case KeepFlag:
        return QStringLiteral("KeepFlag");
    case Author:
        return QStringLiteral("Author");
    }
    return {};
}

Criterion::Subject Criterion::stringToSubject(const QString &subjStr)
{
    if (subjStr == QLatin1String("Title")) {
        return Title;
    } else if (subjStr == QLatin1String("Link")) {
        return Link;
    } else if (subjStr == QLatin1String("Description")) {
        return Description;
    } else if (subjStr == QLatin1String("Status")) {
        return Status;
    } else if (subjStr == QLatin1String("KeepFlag")) {
        return KeepFlag;
    } else if (subjStr == QLatin1String("Author")) {
        return Author;
    }

    // hopefully never reached
    return Description;
}

QString Criterion::predicateToString(Predicate pred)
{
    switch (pred) {
    case Contains:
        return QStringLiteral("Contains");
    case Equals:
        return QStringLiteral("Equals");
    case Matches:
        return QStringLiteral("Matches");
    case Negation:
        return QStringLiteral("Negation");
    }
    return {};
}

Criterion::Predicate Criterion::stringToPredicate(const QString &predStr)
{
    if (predStr == QLatin1String("Contains")) {
        return Contains;
    } else if (predStr == QLatin1String("Equals")) {
        return Equals;
    } else if (predStr == QLatin1String("Matches")) {
        return Matches;
    } else if (predStr == QLatin1String("Negation")) {
        return Negation;
    }

    // hopefully never reached
    return Contains;
}

Criterion::Criterion() = default;

Criterion::Criterion(Subject subject, Predicate predicate, const QVariant &object)
    : m_subject(subject)
    , m_predicate(predicate)
    , m_object(object)
{
}

void Criterion::writeConfig(KConfigGroup *config) const
{
    config->writeEntry(QStringLiteral("subject"), subjectToString(m_subject));

    config->writeEntry(QStringLiteral("predicate"), predicateToString(m_predicate));

    config->writeEntry(QStringLiteral("objectType"), QString::fromLatin1(m_object.typeName()));

    config->writeEntry(QStringLiteral("objectValue"), m_object);
}

void Criterion::readConfig(KConfigGroup *config)
{
    m_subject = stringToSubject(config->readEntry(QStringLiteral("subject"), QString()));
    m_predicate = stringToPredicate(config->readEntry(QStringLiteral("predicate"), QString()));
    QVariant::Type type = QVariant::nameToType(config->readEntry(QStringLiteral("objType"), QString()).toLatin1().constData());

    if (type != QVariant::Invalid) {
        m_object = config->readEntry(QStringLiteral("objectValue"), QVariant(type));
    }
}

bool Criterion::satisfiedBy(const Article &article) const
{
    if (article.isNull()) {
        return false;
    }

    QVariant concreteSubject;

    switch (m_subject) {
    case Title:
        concreteSubject = QVariant(article.title());
        break;
    case Description:
        concreteSubject = QVariant(article.description());
        break;
    case Link:
        // ### Maybe use prettyUrl here?
        concreteSubject = QVariant(article.link().url());
        break;
    case Status:
        concreteSubject = QVariant(article.status());
        break;
    case KeepFlag:
        concreteSubject = QVariant(article.keep());
        break;
    case Author:
        concreteSubject = QVariant(article.authorName());
    }

    bool satisfied = false;

    const auto predicateType = static_cast<Predicate>(m_predicate & ~Negation);
    QString subjectType = QLatin1String(concreteSubject.typeName());

    switch (predicateType) {
    case Contains:
        satisfied = TextUtil::normalize(concreteSubject.toString()).indexOf(m_object.toString(), 0, Qt::CaseInsensitive) != -1;
        break;
    case Equals:
        if (subjectType == QLatin1String("int")) {
            satisfied = concreteSubject.toInt() == m_object.toInt();
        } else {
            satisfied = TextUtil::normalize(concreteSubject.toString()) == m_object.toString();
        }
        break;
    case Matches:
        satisfied = TextUtil::normalize(concreteSubject.toString()).contains(QRegularExpression(m_object.toString()));
        break;
    default:
        qCDebug(AKREGATOR_LOG) << "Internal inconsistency; predicateType should never be Negation";
        break;
    }

    if (m_predicate & Negation) {
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
    : m_association(None)
{
}

ArticleMatcher::~ArticleMatcher() = default;

ArticleMatcher::ArticleMatcher(const QVector<Criterion> &criteria, Association assoc)
    : m_criteria(criteria)
    , m_association(assoc)
{
}

bool ArticleMatcher::matches(const Article &a) const
{
    switch (m_association) {
    case LogicalOr:
        return anyCriterionMatches(a);
    case LogicalAnd:
        return allCriteriaMatch(a);
    default:
        break;
    }
    return true;
}

void ArticleMatcher::writeConfig(KConfigGroup *config) const
{
    config->writeEntry(QStringLiteral("matcherAssociation"), associationToString(m_association));

    config->writeEntry(QStringLiteral("matcherCriteriaCount"), m_criteria.count());

    QString criterionGroupPrefix = config->name() + QLatin1String("_Criterion");

    const int criteriaSize(m_criteria.size());
    for (int index = 0; index < criteriaSize; ++index) {
        *config = KConfigGroup(config->config(), criterionGroupPrefix + QString::number(index));
        m_criteria.at(index).writeConfig(config);
    }
}

void ArticleMatcher::readConfig(KConfigGroup *config)
{
    m_criteria.clear();
    m_association = stringToAssociation(config->readEntry(QStringLiteral("matcherAssociation"), QString()));

    const int count = config->readEntry(QStringLiteral("matcherCriteriaCount"), 0);

    const QString criterionGroupPrefix = config->name() + QLatin1String("_Criterion");

    for (int i = 0; i < count; ++i) {
        Criterion c;
        *config = KConfigGroup(config->config(), criterionGroupPrefix + QString::number(i));
        c.readConfig(config);
        m_criteria.append(c);
    }
}

bool ArticleMatcher::operator==(const AbstractMatcher &other) const
{
    auto ptr = const_cast<AbstractMatcher *>(&other);
    auto o = dynamic_cast<ArticleMatcher *>(ptr);
    if (!o) {
        return false;
    } else {
        return m_association == o->m_association && m_criteria == o->m_criteria;
    }
}

bool ArticleMatcher::operator!=(const AbstractMatcher &other) const
{
    return !(*this == other);
}

bool ArticleMatcher::anyCriterionMatches(const Article &a) const
{
    if (m_criteria.isEmpty()) {
        return true;
    }
    const int criteriaSize(m_criteria.size());
    for (int index = 0; index < criteriaSize; ++index) {
        if (m_criteria.at(index).satisfiedBy(a)) {
            return true;
        }
    }
    return false;
}

bool ArticleMatcher::allCriteriaMatch(const Article &a) const
{
    if (m_criteria.isEmpty()) {
        return true;
    }
    const int criteriaSize(m_criteria.size());
    for (int index = 0; index < criteriaSize; ++index) {
        if (!m_criteria.at(index).satisfiedBy(a)) {
            return false;
        }
    }
    return true;
}

ArticleMatcher::Association ArticleMatcher::stringToAssociation(const QString &assocStr)
{
    if (assocStr == QLatin1String("LogicalAnd")) {
        return LogicalAnd;
    } else if (assocStr == QLatin1String("LogicalOr")) {
        return LogicalOr;
    } else {
        return None;
    }
}

QString ArticleMatcher::associationToString(Association association)
{
    switch (association) {
    case LogicalAnd:
        return QStringLiteral("LogicalAnd");
    case LogicalOr:
        return QStringLiteral("LogicalOr");
    default:
        return QStringLiteral("None");
    }
}
} // namespace Filters
} // namespace Akregator
