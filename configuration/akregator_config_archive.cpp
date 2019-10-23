/*
    This file is part of Akregator.
    Copyright (c) 2008 Frank Osterfeld <osterfeld@kde.org>

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

#include "akregator_config_archive.h"
#include "akregatorconfig.h"
#include "ui_settings_archive.h"
#include <kconfigwidgets_version.h>
#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>

#include <QButtonGroup>
#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_FACTORY(KCMAkregatorArchiveConfigFactory, registerPlugin<KCMAkregatorArchiveConfig>();
                 )

KCMAkregatorArchiveConfig::KCMAkregatorArchiveConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_widget(new QWidget)
{
    Ui::SettingsArchive m_ui;
    m_ui.setupUi(m_widget);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_widget);

    connect(m_ui.rb_LimitArticleNumber, &QAbstractButton::toggled,
            m_ui.kcfg_MaxArticleNumber, &QWidget::setEnabled);
    connect(m_ui.rb_LimitArticleAge, &QAbstractButton::toggled,
            m_ui.kcfg_MaxArticleAge, &QWidget::setEnabled);

    m_ui.kcfg_MaxArticleNumber->setSuffix(ki18ncp("Limit feed archive size to:", " article", " articles"));
    m_ui.kcfg_MaxArticleAge->setSuffix(ki18ncp("Delete articles older than:", " day", " days"));
    m_archiveModeGroup = new QButtonGroup(this);
    m_archiveModeGroup->addButton(m_ui.rb_KeepAllArticles, Settings::EnumArchiveMode::keepAllArticles);
    m_archiveModeGroup->addButton(m_ui.rb_LimitArticleNumber, Settings::EnumArchiveMode::limitArticleNumber);
    m_archiveModeGroup->addButton(m_ui.rb_LimitArticleAge, Settings::EnumArchiveMode::limitArticleAge);
    m_archiveModeGroup->addButton(m_ui.rb_DisableArchiving, Settings::EnumArchiveMode::disableArchiving);
#if KCONFIGWIDGETS_VERSION < QT_VERSION_CHECK(5, 64, 0)
    connect(m_archiveModeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, qOverload<>(&KCMAkregatorArchiveConfig::changed));
#else
    connect(m_archiveModeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &KCMAkregatorArchiveConfig::markAsChanged);
#endif

    KAboutData *about = new KAboutData(QStringLiteral("kcmakrarchiveconfig"),
                                       i18n("Configure Feed Reader Archive"),
                                       QString(), QString(), KAboutLicense::GPL,
                                       i18n("(c), 2004 - 2008 Frank Osterfeld"));

    about->addAuthor(i18n("Frank Osterfeld"), QString(), QStringLiteral("osterfeld@kde.org"));
    setAboutData(about);

    addConfig(Settings::self(), m_widget);
}

void KCMAkregatorArchiveConfig::load()
{
    setArchiveMode(Settings::archiveMode());
    KCModule::load();
}

void KCMAkregatorArchiveConfig::save()
{
    Settings::setArchiveMode(archiveMode());
    KCModule::save();
}

void KCMAkregatorArchiveConfig::setArchiveMode(int mode)
{
    QAbstractButton *const b = m_archiveModeGroup->button(mode);
    if (b) {
        b->setChecked(true);
    } else {
        qWarning() << QStringLiteral("No button for %1 registered, ignoring call").arg(mode);
    }
}

int KCMAkregatorArchiveConfig::archiveMode() const
{
    const int id = m_archiveModeGroup->checkedId();
    if (id < 0 || id >= Settings::EnumArchiveMode::COUNT) {
        return Settings::EnumArchiveMode::keepAllArticles;
    }
    return id;
}

#include "akregator_config_archive.moc"
