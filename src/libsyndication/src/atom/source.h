#ifndef LIBSYNDICATION_ATOM_SOURCE_H
#define LIBSYNDICATION_ATOM_SOURCE_H

#include "../elementwrapper.h"

#include <ctime>

class QDomElement;
class QString;

template <class T> class QList;

namespace LibSyndication {
namespace Atom {

class Category;
class Generator;
class Link;
class Person;

class Source : public ElementWrapper
{
    public:

        Source();
        Source(const QDomElement& element);

        QList<Person> authors() const;

        QList<Person> contributors() const;

        QList<Category> categories() const;

        Generator generator() const;

        QString icon() const;

        QString id() const;

        QList<Link> links() const;

        QString logo() const;

        QString rights() const;

        QString subtitle() const;

        QString title() const;

        time_t updated() const;

        QString debugInfo() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_SOURCE_H
