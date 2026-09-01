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
#include <KConfig>
#include <KConfigGroup>
#include <QUrl>
#include <TextUtils/ConvertText>

#include <QRegularExpression>

using namespace Qt::Literals::StringLiterals;

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
        return u"Title"_s;
    case Link:
        return u"Link"_s;
    case Description:
        return u"Description"_s;
    case Status:
        return u"Status"_s;
    case KeepFlag:
        return u"KeepFlag"_s;
    case Author:
        return u"Author"_s;
    }
    return {};
}

Criterion::Subject Criterion::stringToSubject(const QString &subjStr)
{
    if (subjStr == "Title"_L1) {
        return Title;
    } else if (subjStr == "Link"_L1) {
        return Link;
    } else if (subjStr == "Description"_L1) {
        return Description;
    } else if (subjStr == "Status"_L1) {
        return Status;
    } else if (subjStr == "KeepFlag"_L1) {
        return KeepFlag;
    } else if (subjStr == "Author"_L1) {
        return Author;
    }

    // hopefully never reached
    return Description;
}

QString Criterion::predicateToString(Predicate pred)
{
    switch (pred) {
    case Contains:
        return u"Contains"_s;
    case Equals:
        return u"Equals"_s;
    case Matches:
        return u"Matches"_s;
    case Negation:
        return u"Negation"_s;
    }
    return {};
}

Criterion::Predicate Criterion::stringToPredicate(const QString &predStr)
{
    if (predStr == "Contains"_L1) {
        return Contains;
    } else if (predStr == "Equals"_L1) {
        return Equals;
    } else if (predStr == "Matches"_L1) {
        return Matches;
    } else if (predStr == "Negation"_L1) {
        return Negation;
    }

    // hopefully never reached
    return Contains;
}

Criterion::Criterion() = default;

Criterion::Criterion(Subject subject, Predicate predicate, QVariant object)
    : m_subject(subject)
    , m_predicate(predicate)
    , m_object(std::move(object))
{
}

void Criterion::writeConfig(KConfigGroup *config) const
{
    config->writeEntry(u"subject"_s, subjectToString(m_subject));

    config->writeEntry(u"predicate"_s, predicateToString(m_predicate));

    config->writeEntry(u"objectType"_s, QString::fromLatin1(m_object.typeName()));

    config->writeEntry(u"objectValue"_s, m_object);
}

void Criterion::readConfig(KConfigGroup *config)
{
    m_subject = stringToSubject(config->readEntry(u"subject"_s, QString()));
    m_predicate = stringToPredicate(config->readEntry(u"predicate"_s, QString()));
    QMetaType type = QMetaType::fromName(config->readEntry(u"objectType"_s, QString()).toLatin1().constData());

    if (QMetaType::Type(type.id()) != QMetaType::UnknownType) {
        m_object = config->readEntry(u"objectValue"_s, QVariant(type));
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

    switch (predicateType) {
    case Contains:
        satisfied = TextUtils::ConvertText::normalize(concreteSubject.toString()).indexOf(m_object.toString(), 0, Qt::CaseInsensitive) != -1;
        break;
    case Equals: {
        const QString subjectType = QLatin1StringView(concreteSubject.typeName());
        if (subjectType == "int"_L1) {
            satisfied = concreteSubject.toInt() == m_object.toInt();
        } else {
            satisfied = TextUtils::ConvertText::normalize(concreteSubject.toString()) == m_object.toString();
        }
    } break;
    case Matches:
        satisfied = TextUtils::ConvertText::normalize(concreteSubject.toString()).contains(QRegularExpression(m_object.toString()));
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

ArticleMatcher::ArticleMatcher(QList<Criterion> criteria, Association assoc)
    : m_criteria(std::move(criteria))
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
    config->writeEntry(u"matcherAssociation"_s, associationToString(m_association));

    config->writeEntry(u"matcherCriteriaCount"_s, m_criteria.count());

    const QString criterionGroupPrefix = config->name() + "_Criterion"_L1;

    const int criteriaSize(m_criteria.size());
    for (int index = 0; index < criteriaSize; ++index) {
        *config = KConfigGroup(config->config(), criterionGroupPrefix + QString::number(index));
        m_criteria.at(index).writeConfig(config);
    }
}

void ArticleMatcher::readConfig(KConfigGroup *config)
{
    m_criteria.clear();
    m_association = stringToAssociation(config->readEntry(u"matcherAssociation"_s, QString()));

    const int count = config->readEntry(u"matcherCriteriaCount"_s, 0);

    const QString criterionGroupPrefix = config->name() + "_Criterion"_L1;

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
    if (assocStr == "LogicalAnd"_L1) {
        return LogicalAnd;
    } else if (assocStr == "LogicalOr"_L1) {
        return LogicalOr;
    } else {
        return None;
    }
}

QString ArticleMatcher::associationToString(Association association)
{
    switch (association) {
    case LogicalAnd:
        return u"LogicalAnd"_s;
    case LogicalOr:
        return u"LogicalOr"_s;
    default:
        return u"None"_s;
    }
}
} // namespace Filters
} // namespace Akregator
