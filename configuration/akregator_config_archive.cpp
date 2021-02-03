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

K_PLUGIN_FACTORY(KCMAkregatorArchiveConfigFactory, registerPlugin<KCMAkregatorArchiveConfig>();)

KCMAkregatorArchiveConfig::KCMAkregatorArchiveConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_widget(new QWidget(this))
{
    Ui::SettingsArchive m_ui;
    m_ui.setupUi(m_widget);
    auto layout = new QVBoxLayout(this);
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
    connect(m_archiveModeGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, &KCMAkregatorArchiveConfig::markAsChanged);

    KAboutData *about = new KAboutData(QStringLiteral("kcmakrarchiveconfig"),
                                       i18n("Configure Feed Reader Archive"),
                                       QString(),
                                       QString(),
                                       KAboutLicense::GPL,
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
