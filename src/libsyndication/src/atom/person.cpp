#include "person.h"

#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace Atom {

Person::Person() : ElementWrapper()
{
}

Person::Person(const QDomElement& element) : ElementWrapper(element)
{
}

QString Person::name() const
{
    return "TODO";
}

QString Person::uri() const
{
    return "TODO";
}

QString Person::email() const
{
    return "TODO";
}

QString Person::debugInfo() const
{
    return "TODO";
}

} // namespace Atom
} //namespace LibSyndication
