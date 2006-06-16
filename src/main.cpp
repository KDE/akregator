/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "aboutdata.h"
#include "mainwindow.h"
#include "akregator_options.h"

#include <kcmdlineargs.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kuniqueapplication.h>
#include <dbus/qdbus.h>
#include <QStringList>

namespace Akregator {

class Application : public KUniqueApplication {
  public:
    Application() : mMainWindow( ) {}
    ~Application() {}

    int newInstance();

  private:
    Akregator::MainWindow *mMainWindow;
};

int Application::newInstance()
{
  if (!isSessionRestored())
  {
    QDBusInterfacePtr akr("org.kde.akregator", "/Akregator", "org.kde.akregator.part");

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if ( !mMainWindow ) {
      mMainWindow = new Akregator::MainWindow();
      setMainWidget( mMainWindow );
      mMainWindow->loadPart();
      mMainWindow->setupProgressWidgets();
      if (!args->isSet("hide-mainwindow"))
        mMainWindow->show();
      akr->call( "openStandardFeedList");
    }

    QString addFeedGroup = !args->getOption("group").isEmpty() ? args->getOption("group") : i18n("Imported Folder");

    QByteArrayList feeds = args->getOptionList("addfeed");
    QStringList feedsToAdd;
    QByteArrayList::ConstIterator end( feeds.end() );
    for (QByteArrayList::ConstIterator it = feeds.begin(); it != end; ++it)
        feedsToAdd.append(*it);

    if (!feedsToAdd.isEmpty())
        akr->call("addFeedsToGroup", feedsToAdd, addFeedGroup );

    args->clear();
  }
  return KUniqueApplication::newInstance();
}

} // namespace Akregator

int main(int argc, char **argv)
{
    Akregator::AboutData about;
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( Akregator::akregator_options );
    KUniqueApplication::addCmdLineOptions();

    Akregator::Application app;

    // start knotifyclient if not already started. makes it work for people who doesn't use full kde, according to kmail devels
    KNotifyClient::startDaemon();

    // see if we are starting with session management
    if (app.isSessionRestored())
    {
#undef RESTORE
#define RESTORE(type) { int n = 1;\
    while (KMainWindow::canBeRestored(n)){\
        (new type)->restore(n, false);\
            n++;}}

        RESTORE(Akregator::MainWindow);
    }

    return app.exec();
}


