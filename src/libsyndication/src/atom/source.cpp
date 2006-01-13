#include "category.h"
#include "generator.h"
#include "link.h"
#include "person.h"
#include "source.h"

#include <QDomElement>
#include <QList>
#include <QString>

namespace LibSyndication {
namespace Atom {

Source::Source() : ElementWrapper()
{
}

Source::Source(const QDomElement& element) : ElementWrapper(element)
{
}

QList<Person> Source::authors() const
{
    return QList<Person>(); // TODO
}

QList<Person> Source::contributors() const
{
    return QList<Person>(); // TODO
}

QList<Category> Source::categories() const
{
    return QList<Category>(); // TODO
}

Generator Source::generator() const
{
    return Generator(); // TODO
}

QString Source::icon() const
{
    return "TODO";
}

QString Source::id() const
{
    return "TODO";
}

QList<Link> Source::links() const
{
    return QList<Link>(); // TODO
}

QString Source::logo() const
{
    return "TODO";
}

QString Source::rights() const
{
    return "TODO";
}

QString Source::subtitle() const
{
    return "TODO";
}

QString Source::title() const
{
    return "TODO";
}

time_t Source::updated() const
{
    return 0; // TODO
}

QString Source::debugInfo() const
{
    return "TODO";
}

} // namespace Atom
} //namespace LibSyndication
