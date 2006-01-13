#ifndef LIBSYNDICATION_ATOM_GENERATOR_H
#define LIBSYNDICATION_ATOM_GENERATOR_H

#include "../elementwrapper.h"

class QDomDocument;
class QDomElement;
class QString;

namespace LibSyndication {
namespace Atom {

class Generator : public ElementWrapper
{
    public:

        Generator();
        Generator(const QDomElement& element);

        QString uri() const;

        QString version() const;

        QString debugInfo() const;

};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_GENERATOR_H
