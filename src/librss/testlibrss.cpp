#include "testlibrss.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>

using namespace RSS;

void Tester::test()
{
	Loader *loader = Loader::create();
	connect( loader, SIGNAL( loadingComplete( Loader *, Document, Status ) ),
	         this, SLOT( slotLoadingComplete( Loader *, Document, Status ) ) );
	loader->loadFrom( "http://sourceforge.net/export/rss2_projnews.php?group_id=29057&rss_fulltext=1", new FileRetriever );
}

void Tester::slotLoadingComplete( Loader *loader, Document doc, Status status )
{
	if ( status == Success )
		kdDebug() << "Successfully retrieverd '" << doc.title() << "'" << endl;

	if ( status != Success )
		kdDebug() << "ERROR " << loader->errorCode() << endl;

	kapp->quit();
}

int main( int argc, char **argv )
{
	KAboutData aboutData( "testlibrss", "testlibrss", "0.1" );
	KCmdLineArgs::init( argc, argv, &aboutData );
	KApplication app;

	Tester tester;
	tester.test();

	return app.exec();
}

#include "testlibrss.moc"
