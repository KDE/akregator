#ifndef AKREGATOR_BACKEND_MK4PLUGIN_H
#define AKREGATOR_BACKEND_MK4PLUGIN_H

#include "../plugin.h"

class QString;

namespace Akregator {
namespace Backend {

class StorageFactory;

class MK4Plugin : public Akregator::Plugin 
{
   public: 
      virtual ~MK4Plugin();	
      virtual bool init();

   private:
      StorageFactory* m_factory;
};

}
}
#endif // AKREGATOR_BACKEND_MK4PLUGIN_H
