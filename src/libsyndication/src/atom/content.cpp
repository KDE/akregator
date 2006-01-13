#include "content.h"

#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace Atom {

Content::Content() : ElementWrapper()
{
}

Content::Content(const QDomElement& element) : ElementWrapper(element)
{
}

QString Content::type() const
{
    return "TODO";
}

QString Content::src() const
{
    return "TODO";
}

QString Content::debugInfo() const
{
    return "TODO";
}

} // namespace Atom
} //namespace LibSyndication
