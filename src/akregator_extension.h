/***************************************************************************
 *   Copyright (C) 2004 by Stephan Binner                                  *
 *   binner@kde.org                                                        *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef _AKREGATOREXTENSION_H_
#define _AKREGATOREXTENSION_H_

#include <kparts/browserextension.h>

namespace Akregator
{
  class aKregatorPart;
  class aKregatorView;
}

namespace Akregator
{
  class aKregatorExtension : public KParts::BrowserExtension
  {
    Q_OBJECT
    public:
      aKregatorExtension( KParts::ReadOnlyPart* parent, const char *name = 0L );
      void emitSaveSettings();

    signals:
      void saveSettings();
  };
}

#endif // _AKREGATOREXTENSION_H_
