/***************************************************************************
 *   Copyright (C) 2004 by Stephan Binner                                  *
 *   binner@kde.org                                                        *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "akregator_extension.h"

using namespace Akregator;

aKregatorExtension::aKregatorExtension( KParts::ReadOnlyPart* parent, const char *name)
  : KParts::BrowserExtension( parent, name )
{
}

void aKregatorExtension::emitSaveSettings()
{
  emit saveSettings();
}

#include "akregator_extension.moc"
