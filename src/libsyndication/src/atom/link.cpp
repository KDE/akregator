#include "link.h"

#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace Atom {

Link::Link() : ElementWrapper()
{
}

Link::Link(const QDomElement& element) : ElementWrapper(element)
{
}

QString Link::href() const
{
    return "TODO";
}

QString Link::rel() const
{
    return "TODO";
}

QString Link::type() const
{
    return "TODO";
}

QString Link::hrefLanguage() const
{
    return "TODO";
}

QString Link::title() const
{
    return "TODO";
}

uint Link::length() const
{
    return 0; // TODO
}

QString Link::debugInfo() const
{
    return "TODO";
}

} // namespace Atom
} //namespace LibSyndication
