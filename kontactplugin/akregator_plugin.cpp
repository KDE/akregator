/*
  This file is part of Akregator.

  SPDX-FileCopyrightText: 2004 Sashmit Bhaduri <smt@vfemail.net>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
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

EXPORT_KONTACT_PLUGIN_WITH_JSON(AkregatorPlugin, "akregatorplugin.json")

AkregatorPlugin::AkregatorPlugin(KontactInterface::Core *core, const QVariantList &)
    : KontactInterface::Plugin(core, core, "akregator")
{
    setComponentName(QStringLiteral("akregator"), i18n("Akregator"));

    QAction *action
        = new QAction(QIcon::fromTheme(QStringLiteral("bookmark-new")),
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
    m_interface = nullptr;
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

OrgKdeAkregatorPartInterface *AkregatorPlugin::interface()
{
    if (!m_interface) {
        part();
    }
    Q_ASSERT(m_interface);
    return m_interface;
}

KParts::Part *AkregatorPlugin::createPart()
{
    KParts::Part *part = loadPart();
    if (!part) {
        return nullptr;
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

void AkregatorUniqueAppHandler::loadCommandLineOptions(QCommandLineParser *parser)
{
    Akregator::akregator_options(parser);
}

int AkregatorUniqueAppHandler::activate(const QStringList &args, const QString &workingDir)
{
    // Ensure part is loaded
    (void)plugin()->part();

    org::kde::akregator::part akregator(
        QStringLiteral("org.kde.akregator"), QStringLiteral("/Akregator"), QDBusConnection::sessionBus());
    akregator.openStandardFeedList();
    akregator.handleCommandLine(args);

    return KontactInterface::UniqueAppHandler::activate(args, workingDir);
}

#include "akregator_plugin.moc"
