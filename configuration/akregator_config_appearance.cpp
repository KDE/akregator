/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "akregator_config_appearance.h"
#include "akregatorconfig.h"

#include <KPluginFactory>
#include <QAbstractSlider>

#include <QVBoxLayout>

using namespace Qt::Literals::StringLiterals;

using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(KCMAkregatorAppearanceConfig, "akregator_config_appearance.json")
KCMAkregatorAppearanceConfig::KCMAkregatorAppearanceConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , m_widget(new QWidget(widget()))
{
    m_ui.setupUi(m_widget);
    auto layout = new QVBoxLayout(widget());
    layout->addWidget(m_widget);

    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.kcfg_ColorUnreadArticles, &QWidget::setEnabled);
    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.kcfg_ColorNewArticles, &QWidget::setEnabled);
    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.lbl_newArticles, &QWidget::setEnabled);
    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.lbl_unreadArticles, &QWidget::setEnabled);

    connect(m_ui.slider_minimumFontSize, &QAbstractSlider::valueChanged, m_ui.kcfg_MinimumFontSize, &QSpinBox::setValue);
    connect(m_ui.slider_mediumFontSize, &QAbstractSlider::valueChanged, m_ui.kcfg_MediumFontSize, &QSpinBox::setValue);

    connect(m_ui.kcfg_MinimumFontSize, &QSpinBox::valueChanged, m_ui.slider_minimumFontSize, &QSlider::setValue);
    connect(m_ui.kcfg_MediumFontSize, &QSpinBox::valueChanged, m_ui.slider_mediumFontSize, &QSlider::setValue);

    const bool minimumFontSizeImmutable = Settings::self()->isImmutable(u"MinimumFontSize"_s);
    const bool mediumFontSizeImmutable = Settings::self()->isImmutable(u"MediumFontSize"_s);
    m_ui.slider_minimumFontSize->setDisabled(minimumFontSizeImmutable);
    m_ui.slider_mediumFontSize->setDisabled(mediumFontSizeImmutable);
    m_ui.lbl_MinimumFontSize->setDisabled(minimumFontSizeImmutable);
    m_ui.lbl_MediumFontSize->setDisabled(mediumFontSizeImmutable);

    addConfig(Settings::self(), m_widget);
}

#include "akregator_config_appearance.moc"

#include "moc_akregator_config_appearance.cpp"
