#ifndef LIBSYNDICATION_MAPPER_RDFMAPPER_H
#define LIBSYNDICATION_MAPPER_RDFMAPPER_H

#include "feedrdfimpl.h"

#include "../rdf/document.h"
#include "../abstractdocument.h"
#include "../feed.h"
#include "../mapper.h"



namespace LibSyndication {

/** @internal */
class RDFMapper : public Mapper<Feed>
{
    KSharedPtr<Feed> map(AbstractDocumentPtr doc) const
    {
        return KSharedPtr<Feed>(new FeedRDFImpl(LibSyndication::RDF::DocumentPtr::staticCast(doc)));
    }
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_MAPPER_RDFMAPPER_H
