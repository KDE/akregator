#ifndef LIBSYNDICATION_ATOM_PERSON_H
#define LIBSYNDICATION_ATOM_PERSON_H

#include "../elementwrapper.h"

class QDomElement;
class QString;

namespace LibSyndication {
namespace Atom {

class Person : public ElementWrapper
{
    public:

       Person();
       Person(const QDomElement& element);

       /**
        * returns a human-readable name for the person. The name is a required 
        * attribute of person constructs.
        *
        * @return a human-readable name of the person
        */
        QString name() const;

        QString uri() const;

        /**
         * returns an e-mail address associated with the person. The e-mail address is optional.
         *
         * @return an e-mail address, or QString::null if none is set
         */
        QString email() const;

        QString debugInfo() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_PERSON_H
