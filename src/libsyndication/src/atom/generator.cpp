#include "generator.h"

#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace Atom {

Generator::Generator() : ElementWrapper()
{
}

Generator::Generator(const QDomElement& element) : ElementWrapper(element)
{
}

QString Generator::uri() const
{
    return "TODO";
}

QString Generator::version() const
{
    return "TODO";
}

QString Generator::debugInfo() const
{
    return "TODO";
}

} // namespace Atom
} //namespace LibSyndication

