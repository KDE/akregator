/*
  This file is part of Akregator.

  Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "akregator_plugin.h"
#include "akregator_part.h"
#include "akregator_options.h"
#include "partinterface.h"

#include <KontactInterface/Core>

#include <QAction>
#include <KActionCollection>
#include <KLocalizedString>
#include <QIcon>
#include <QStandardPaths>

EXPORT_KONTACT_PLUGIN(AkregatorPlugin, akregator)

AkregatorPlugin::AkregatorPlugin(KontactInterface::Core *core, const QVariantList &)
    : KontactInterface::Plugin(core, core, "akregator"), m_interface(Q_NULLPTR)
{
    setComponentName(QStringLiteral("akregator"), QStringLiteral("akregator"));

    QAction *action =
        new QAction(QIcon::fromTheme(QStringLiteral("bookmark-new")),
                    i18nc("@action:inmenu", "New Feed..."), this);
    actionCollection()->addAction(QStringLiteral("feed_new"), action);
    actionCollection()->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F));
    setHelpText(action, i18nc("@info:status", "Create a new feed"));
    action->setWhatsThis(
        i18nc("@info:whatsthis",
              "You will be presented with a dialog where you can add a new feed."));
    connect(action, &QAction::triggered, this, &AkregatorPlugin::addFeed);
    insertNewAction(action);

    mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
        new KontactInterface::UniqueAppHandlerFactory<AkregatorUniqueAppHandler>(), this);
}

AkregatorPlugin::~AkregatorPlugin()
{
    delete m_interface;
    m_interface = Q_NULLPTR;
}

void AkregatorPlugin::setHelpText(QAction *action, const QString &text)
{
    action->setStatusTip(text);
    action->setToolTip(text);
    if (action->whatsThis().isEmpty()) {
        action->setWhatsThis(text);
    }
}

bool AkregatorPlugin::isRunningStandalone() const
{
    return mUniqueAppWatcher->isRunningStandalone();
}

QStringList AkregatorPlugin::invisibleToolbarActions() const
{
    return QStringList() << QStringLiteral("file_new_contact");
}

OrgKdeAkregatorPartInterface *AkregatorPlugin::interface()
{
    if (!m_interface) {
        part();
    }
    Q_ASSERT(m_interface);
    return m_interface;

}

QString AkregatorPlugin::tipFile() const
{
    // TODO: tips file
    //QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "akregator/tips");
    QString file;
    return file;
}

KParts::ReadOnlyPart *AkregatorPlugin::createPart()
{
    KParts::ReadOnlyPart *part = loadPart();
    if (!part) {
        return Q_NULLPTR;
    }

    m_interface = new OrgKdeAkregatorPartInterface(
        QStringLiteral("org.kde.akregator"), QStringLiteral("/Akregator"), QDBusConnection::sessionBus());
    m_interface->openStandardFeedList();

    return part;
}

void AkregatorPlugin::addFeed()
{
    // Ensure part is loaded
    (void)part();

    org::kde::akregator::part akregator(
        QStringLiteral("org.kde.akregator"), QStringLiteral("/Akregator"), QDBusConnection::sessionBus());
    akregator.addFeed();
}

QStringList AkregatorPlugin::configModules() const
{
    QStringList modules;
    modules << QStringLiteral("PIM/akregator.desktop");
    return modules;
}

void AkregatorPlugin::readProperties(const KConfigGroup &config)
{
    if (part()) {
        Akregator::Part *myPart = static_cast<Akregator::Part *>(part());
        myPart->readProperties(config);
    }
}

void AkregatorPlugin::saveProperties(KConfigGroup &config)
{
    if (part()) {
        Akregator::Part *myPart = static_cast<Akregator::Part *>(part());
        myPart->saveProperties(config);
    }
}

void AkregatorUniqueAppHandler::loadCommandLineOptions()
{
    KCmdLineArgs::addCmdLineOptions(Akregator::akregator_options());
}

int AkregatorUniqueAppHandler::newInstance()
{
    // Ensure part is loaded
    (void)plugin()->part();

    org::kde::akregator::part akregator(
        QStringLiteral("org.kde.akregator"), QStringLiteral("/Akregator"), QDBusConnection::sessionBus());
    akregator.openStandardFeedList();
    akregator.handleCommandLine();

    return KontactInterface::UniqueAppHandler::newInstance();
}
#include "akregator_plugin.moc"
