#include "resource.h"
#include "resourcewrapper.h"


namespace LibSyndication {
namespace RDF {

class ResourceWrapper::ResourceWrapperPrivate : public KShared
{
    public:
        
    ResourcePtr resource;
};

ResourceWrapper::ResourceWrapper() : d(new ResourceWrapperPrivate)
{
    d->resource = new Resource();
}

ResourceWrapper::ResourceWrapper(const ResourceWrapper& other)
{
    *this = other;
}

ResourceWrapper::ResourceWrapper(ResourcePtr resource) : d(new ResourceWrapperPrivate)
{
    d->resource = resource;
}

ResourceWrapper::~ResourceWrapper()
{
}

ResourceWrapper& ResourceWrapper::operator=(const ResourceWrapper& other)
{
    d = other.d;
    return *this;
}

bool ResourceWrapper::operator==(const ResourceWrapper& other) const
{
    return *(d->resource) == *(other.d->resource);
}

bool ResourceWrapper::isNull() const
{
    return d->resource->isNull();
}

ResourcePtr ResourceWrapper::resource() const
{
    return d->resource;
}

} // namespace RDF
} // namespace LibSyndication
