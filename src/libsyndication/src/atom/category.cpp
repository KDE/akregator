#include "category.h"

#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace Atom {

Category::Category() : ElementWrapper()
{
}

Category::Category(const QDomElement& element) : ElementWrapper(element)
{
}

QString Category::term() const
{
    return "TODO";
}

QString Category::scheme() const
{
    return "TODO";
}

QString Category::label() const
{
    return "TODO";
}

QString Category::debugInfo() const
{
    return "TODO";
}

} // namespace Atom
} //namespace LibSyndication
