#ifndef AKREGATOR_OPTIONS_H
#define AKREGATOR_OPTIONS_H

#include <kcmdlineargs.h>
#include <klocale.h>

static KCmdLineOptions akregator_options[] =
{
	{ "a", 0, 0 },
	{ "addfeed <url>", I18N_NOOP( "Add a feed with the given URL" ), 0},
	{ "g", 0, 0 },
	{ "group <groupname>", I18N_NOOP( "When adding feeds, place them in this group" ), I18N_NOOP("Imported") },
	KCmdLineLastOption
};

#endif
