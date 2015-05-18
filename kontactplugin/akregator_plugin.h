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

  You should have received a copy of the GNU General Public Licensea along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_PLUGIN_H
#define AKREGATOR_PLUGIN_H

#include <KontactInterface/UniqueAppHandler>

namespace KontactInterface
{
class Plugin;
}

class OrgKdeAkregatorPartInterface;

class AkregatorUniqueAppHandler : public KontactInterface::UniqueAppHandler
{
public:
    explicit AkregatorUniqueAppHandler(KontactInterface::Plugin *plugin)
        : KontactInterface::UniqueAppHandler(plugin) {}
    void loadCommandLineOptions(QCommandLineParser *parser) Q_DECL_OVERRIDE;
    int activate(const QStringList &args) Q_DECL_OVERRIDE;
};

class AkregatorPlugin : public KontactInterface::Plugin
{
    Q_OBJECT

public:
    AkregatorPlugin(KontactInterface::Core *core, const QVariantList &);
    ~AkregatorPlugin();

    QString tipFile() const Q_DECL_OVERRIDE;
    int weight() const Q_DECL_OVERRIDE
    {
        return 475;
    }

    OrgKdeAkregatorPartInterface *interface();

    virtual QStringList configModules() const;
    QStringList invisibleToolbarActions() const Q_DECL_OVERRIDE;
    bool isRunningStandalone() const Q_DECL_OVERRIDE;
    void readProperties(const KConfigGroup &config) Q_DECL_OVERRIDE;
    void saveProperties(KConfigGroup &config) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void addFeed();

protected:
    KParts::ReadOnlyPart *createPart() Q_DECL_OVERRIDE;
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher;
    OrgKdeAkregatorPartInterface *m_interface;

private:
    void setHelpText(QAction *action, const QString &text);
};

#endif
