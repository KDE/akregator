/*
  This file is part of Akregator.

  SPDX-FileCopyrightText: 2004 Sashmit Bhaduri <smt@vfemail.net>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0

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

#pragma once

#include <KontactInterface/UniqueAppHandler>

namespace KontactInterface
{
class Plugin;
}

class OrgKdeAkregatorPartInterface;

class AkregatorUniqueAppHandler : public KontactInterface::UniqueAppHandler
{
    Q_OBJECT
public:
    explicit AkregatorUniqueAppHandler(KontactInterface::Plugin *plugin)
        : KontactInterface::UniqueAppHandler(plugin)
    {
    }

    void loadCommandLineOptions(QCommandLineParser *parser) override;
    int activate(const QStringList &args, const QString &workingDir) override;
};

class AkregatorPlugin : public KontactInterface::Plugin
{
    Q_OBJECT

public:
    AkregatorPlugin(KontactInterface::Core *core, const QVariantList &);
    ~AkregatorPlugin() override;

    int weight() const override
    {
        return 475;
    }

    OrgKdeAkregatorPartInterface *interface();

    bool isRunningStandalone() const override;
    void readProperties(const KConfigGroup &config) override;
    void saveProperties(KConfigGroup &config) override;

protected:
    KParts::Part *createPart() override;
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher = nullptr;
    OrgKdeAkregatorPartInterface *m_interface = nullptr;

private:
    void addFeed();
    void setHelpText(QAction *action, const QString &text);
};

