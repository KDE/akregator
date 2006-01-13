#ifndef LIBSYNDICATION_ATOM_CONTENT_H
#define LIBSYNDICATION_ATOM_CONTENT_H

#include "../elementwrapper.h"

class QDomElement;
class QString;

namespace LibSyndication {
namespace Atom {

class Content : public ElementWrapper
{
    public:
        Content();
        Content(const QDomElement& element);

        QString type() const;

        QString src() const;
        
        QString debugInfo() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_CONTENT_H
