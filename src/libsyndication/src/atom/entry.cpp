#include "category.h"
#include "content.h"
#include "entry.h"
#include "link.h"
#include "person.h"
#include "source.h"

#include <QDomElement>
#include <QList>
#include <QString>

namespace LibSyndication {
namespace Atom {

Entry::Entry() : ElementWrapper()
{
}

Entry::Entry(const QDomElement& element) : ElementWrapper(element)
{
}

QList<Person> Entry::authors() const
{
    return QList<Person>(); // TODO
}

QList<Category> Entry::categories() const
{
    return QList<Category>(); // TODO
}

QList<Person> Entry::contributors() const
{
    return QList<Person>(); // TODO
}

QString Entry::id() const
{
    return "TODO";
}

QList<Link> Entry::links() const
{
    return QList<Link>();  // TODO
}

QString Entry::rights() const
{
    return "TODO";
}

Source Entry::source() const
{
    return Source(); // TODO
}

time_t Entry::published() const
{
    return 0; // TODO
}

time_t Entry::updated() const
{
    return 0; // TODO
}

QString Entry::summary() const
{
    return "TODO";
}

QString Entry::title() const
{
    return "TODO";
}

Content Entry::content() const
{
    return Content(); // TODO
}

QString Entry::debugInfo() const
{
    return "TODO";
}

} // namespace Atom
} //namespace LibSyndication

