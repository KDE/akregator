#ifndef LIBSYNDICATION_ATOM_CATEGORY_H
#define LIBSYNDICATION_ATOM_CATEGORY_H

#include "../elementwrapper.h"

class QDomElement;
class QString;

namespace LibSyndication {
namespace Atom {

class Category : public ElementWrapper
{
    public:
        Category();
        Category(const QDomElement& element);
        
        /* 1 */
        QString term() const;
    
        /* 0..1 */
        QString scheme() const;
    
        /* 0..1 */
        /* TODO: language sensitive */
        QString label() const;
    
        QString debugInfo() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_CATEGORY_H
