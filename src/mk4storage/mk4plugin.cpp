#include "mk4plugin.h"

#include <klocale.h>

#include "storagefactorymk4impl.h"
#include "storagefactoryregistry.h"

AKREGATOR_EXPORT_PLUGIN( Akregator::Backend::MK4Plugin )

namespace Akregator {
namespace Backend {

bool MK4Plugin::init()
{
   m_factory = new StorageFactoryMK4Impl();
   return StorageFactoryRegistry::self()->registerFactory(m_factory, "metakit");
}

MK4Plugin::~MK4Plugin()
{
    StorageFactoryRegistry::self()->unregisterFactory("metakit");
    delete m_factory;
}

}
}
