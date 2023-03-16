/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "akregator_config_appearance.h"
#include "akregatorconfig.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QAbstractSlider>

#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(KCMAkregatorAppearanceConfig, "akregator_config_appearance.json")
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
KCMAkregatorAppearanceConfig::KCMAkregatorAppearanceConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_widget(new QWidget(this))
#else
KCMAkregatorAppearanceConfig::KCMAkregatorAppearanceConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCModule(parent, data, args)
    , m_widget(new QWidget(widget()))
#endif
{
    m_ui.setupUi(m_widget);
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    auto layout = new QVBoxLayout(this);
#else
    auto layout = new QVBoxLayout(widget());
#endif
    layout->addWidget(m_widget);

    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.kcfg_ColorUnreadArticles, &QWidget::setEnabled);
    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.kcfg_ColorNewArticles, &QWidget::setEnabled);
    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.lbl_newArticles, &QWidget::setEnabled);
    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.lbl_unreadArticles, &QWidget::setEnabled);

    connect(m_ui.slider_minimumFontSize, &QAbstractSlider::valueChanged, m_ui.kcfg_MinimumFontSize, &QSpinBox::setValue);
    connect(m_ui.slider_mediumFontSize, &QAbstractSlider::valueChanged, m_ui.kcfg_MediumFontSize, &QSpinBox::setValue);

    connect(m_ui.slider_minimumFontSize, &QAbstractSlider::sliderMoved, m_ui.kcfg_MinimumFontSize, &QSpinBox::setValue);
    connect(m_ui.slider_mediumFontSize, &QAbstractSlider::sliderMoved, m_ui.kcfg_MediumFontSize, &QSpinBox::setValue);

    connect(m_ui.kcfg_MinimumFontSize, &KPluralHandlingSpinBox::valueChanged, m_ui.slider_minimumFontSize, &QSlider::setValue);
    connect(m_ui.kcfg_MediumFontSize, &KPluralHandlingSpinBox::valueChanged, m_ui.slider_mediumFontSize, &QSlider::setValue);

    m_ui.slider_minimumFontSize->setDisabled(Settings::self()->isImmutable(QStringLiteral("MinimumFontSize")));
    m_ui.slider_mediumFontSize->setDisabled(Settings::self()->isImmutable(QStringLiteral("MediumFontSize")));
    m_ui.lbl_MinimumFontSize->setDisabled(Settings::self()->isImmutable(QStringLiteral("MinimumFontSize")));
    m_ui.lbl_MediumFontSize->setDisabled(Settings::self()->isImmutable(QStringLiteral("MediumFontSize")));

    addConfig(Settings::self(), m_widget);
}

#include "akregator_config_appearance.moc"
