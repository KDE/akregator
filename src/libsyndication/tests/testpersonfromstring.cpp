#include "mapper/personimpl.h"
#include "person.h"

#include <QList>
#include <QString>
#include <QStringList>

#include <iostream>

using LibSyndication::PersonPtr;
using LibSyndication::Person;
using LibSyndication::PersonImpl;

int main()
{
    QStringList s;
    QList<PersonPtr> p;

    s.append(QString());
    p.append(PersonPtr(new PersonImpl(QString(), QString(), QString())));
    s.append("");
    p.append(PersonPtr(new PersonImpl(QString(), QString(), QString())));
    s.append("Foo");
    p.append(PersonPtr(new PersonImpl("Foo", QString(), QString())));
    s.append("Foo M. Bar");
    p.append(PersonPtr(new PersonImpl("Foo M. Bar", QString(), QString())));
    s.append("foo@bar.com");
    p.append(PersonPtr(new PersonImpl(QString(), QString(), "foo@bar.com")));
    s.append("<foo@bar.com>");
    p.append(PersonPtr(new PersonImpl(QString(), QString(), "foo@bar.com")));
    s.append("Foo <foo@bar.com>");
    p.append(PersonPtr(new PersonImpl("Foo", QString(), "foo@bar.com")));
    s.append("Foo Bar <foo@bar.com>");
    p.append(PersonPtr(new PersonImpl("Foo Bar", QString(), "foo@bar.com")));
    
    QStringList::ConstIterator sit = s.begin();
    QStringList::ConstIterator send = s.end();
    QList<PersonPtr>::ConstIterator pit = p.begin();
    
    int failCount = 0;
    
    while (sit != s.end())
    {
        PersonPtr person(PersonImpl::fromString(*sit));
        
        if (!(*person == *(*pit)))
        {
            ++failCount;
            std::cout << "\"" << (*sit).toLocal8Bit().data() << "\" parsed incorrectly:\n" << std::endl;
            std::cout << "parsed:\n" << std::endl;
            std::cout << person->debugInfo().toLocal8Bit().data() << std::endl;
            std::cout << "\nshould have been:\n" << std::endl;
            std::cout << (*pit)->debugInfo().toLocal8Bit().data() << std::endl;
        }
        ++sit;
        ++pit;
    }
    
    std::cout << failCount << " tests failed (" << s.count() << " tests in total)" << std::endl;
    return failCount;
}

