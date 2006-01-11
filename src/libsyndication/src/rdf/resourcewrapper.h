#ifndef LIBSYNDICATION_RDF_RESOURCEWRAPPER_H
#define LIBSYNDICATION_RDF_RESOURCEWRAPPER_H

#include <ksharedptr.h>

namespace LibSyndication {
namespace RDF {

class Resource;
typedef KSharedPtr<Resource> ResourcePtr;

class ResourceWrapper
{
    public:
        ResourceWrapper();
        ResourceWrapper(const ResourceWrapper& other);
        ResourceWrapper(ResourcePtr resource);
        virtual ~ResourceWrapper();

        ResourceWrapper& operator=(const ResourceWrapper& other);
        bool operator==(const ResourceWrapper& other) const;
        
        ResourcePtr resource() const;

        bool isNull() const;

    private:

        class ResourceWrapperPrivate;
        KSharedPtr<ResourceWrapperPrivate> d;
};

} // namespace RDF
} // namespace LibSyndication

#endif // LIBSYNDICATION_RDF_RESOURCEWRAPPER_H
