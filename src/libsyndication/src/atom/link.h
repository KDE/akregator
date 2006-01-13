#ifndef LIBSYNDICATION_ATOM_LINK_H
#define LIBSYNDICATION_ATOM_LINK_H

#include "../elementwrapper.h"

class QDomElement;
class QString;

namespace LibSyndication {
namespace Atom {

class Link : public ElementWrapper
{
    public:

        Link();
        Link(const QDomElement& e);
    
        QString href() const;
    
        QString rel() const;
        
        QString type() const;
    
        QString hrefLanguage() const;
    
        QString title() const;
    
        uint length() const;
    
        QString debugInfo() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_LINK_H
