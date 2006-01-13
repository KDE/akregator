#ifndef LIBSYNDICATION_ATOM_PARSER_H
#define LIBSYNDICATION_ATOM_PARSER_H

#include "../abstractparser.h"

class QString;

namespace LibSyndication {

class AbstractDocument;
class DocumentSource;

namespace Atom {

    
class Parser : public LibSyndication::AbstractParser
{
    public:

        bool accept(const LibSyndication::DocumentSource& source) const;

        LibSyndication::AbstractDocument* parse(const LibSyndication::DocumentSource& source) const;
        
        QString format() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_PARSER_H
