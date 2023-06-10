/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "akregator_config_archive.h"
#include "ui_settings_archive.h"
#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QButtonGroup>
#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(KCMAkregatorArchiveConfig, "akregator_config_archive.json")

KCMAkregatorArchiveConfig::KCMAkregatorArchiveConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , m_widget(new QWidget(widget()))
{
    Ui::SettingsArchive m_ui;
    m_ui.setupUi(m_widget);
    auto layout = new QVBoxLayout(widget());
    layout->addWidget(m_widget);

    connect(m_ui.rb_LimitArticleNumber, &QAbstractButton::toggled, m_ui.kcfg_MaxArticleNumber, &QWidget::setEnabled);
    connect(m_ui.rb_LimitArticleAge, &QAbstractButton::toggled, m_ui.kcfg_MaxArticleAge, &QWidget::setEnabled);

    m_ui.kcfg_MaxArticleNumber->setSuffix(ki18ncp("Limit feed archive size to:", " article", " articles"));
    m_ui.kcfg_MaxArticleAge->setSuffix(ki18ncp("Delete articles older than:", " day", " days"));
    m_archiveModeGroup = new QButtonGroup(this);
    m_archiveModeGroup->addButton(m_ui.rb_KeepAllArticles, Settings::EnumArchiveMode::keepAllArticles);
    m_archiveModeGroup->addButton(m_ui.rb_LimitArticleNumber, Settings::EnumArchiveMode::limitArticleNumber);
    m_archiveModeGroup->addButton(m_ui.rb_LimitArticleAge, Settings::EnumArchiveMode::limitArticleAge);
    m_archiveModeGroup->addButton(m_ui.rb_DisableArchiving, Settings::EnumArchiveMode::disableArchiving);
    connect(m_archiveModeGroup, &QButtonGroup::buttonClicked, this, &KCMAkregatorArchiveConfig::markAsChanged);

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
