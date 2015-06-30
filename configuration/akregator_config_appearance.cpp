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

#include "akregator_config_appearance.h"
#include "akregatorconfig.h"

#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>

#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_FACTORY_WITH_JSON(KCMAkregatorAppearanceConfigFactory, "akregator_config_appearance.json", registerPlugin<KCMAkregatorAppearanceConfig>();)

KCMAkregatorAppearanceConfig::KCMAkregatorAppearanceConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args), m_widget(new QWidget)
{
    m_ui.setupUi(m_widget);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_widget);

    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.kcfg_ColorUnreadArticles, &QWidget::setEnabled);
    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.kcfg_ColorNewArticles, &QWidget::setEnabled);
    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.lbl_newArticles, &QWidget::setEnabled);
    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.lbl_unreadArticles, &QWidget::setEnabled);

    connect(m_ui.slider_minimumFontSize, &QAbstractSlider::valueChanged, m_ui.kcfg_MinimumFontSize , &QSpinBox::setValue);
    connect(m_ui.slider_mediumFontSize, &QAbstractSlider::valueChanged, m_ui.kcfg_MediumFontSize , &QSpinBox::setValue);

    connect(m_ui.slider_minimumFontSize, &QAbstractSlider::sliderMoved, m_ui.kcfg_MinimumFontSize , &QSpinBox::setValue);
    connect(m_ui.slider_mediumFontSize, &QAbstractSlider::sliderMoved, m_ui.kcfg_MediumFontSize , &QSpinBox::setValue);

    connect(m_ui.kcfg_MinimumFontSize, SIGNAL(valueChanged(int)), m_ui.slider_minimumFontSize, SLOT(setValue(int)));
    connect(m_ui.kcfg_MediumFontSize, SIGNAL(valueChanged(int)), m_ui.slider_mediumFontSize, SLOT(setValue(int)));
    KAboutData *about = new KAboutData(QStringLiteral("kcmakrappearanceconfig"),
                                       i18n("Configure Feed Reader Appearance"),
                                       QString(), QString(), KAboutLicense::GPL,
                                       i18n("(c), 2004 - 2008 Frank Osterfeld"));

    about->addAuthor(i18n("Frank Osterfeld"), QString(), QStringLiteral("osterfeld@kde.org"));

    setAboutData(about);

    m_ui.slider_minimumFontSize->setDisabled(Settings::self()->isImmutable(QStringLiteral("MinimumFontSize")));
    m_ui.slider_mediumFontSize->setDisabled(Settings::self()->isImmutable(QStringLiteral("MediumFontSize")));
    m_ui.lbl_MinimumFontSize->setDisabled(Settings::self()->isImmutable(QStringLiteral("MinimumFontSize")));
    m_ui.lbl_MediumFontSize->setDisabled(Settings::self()->isImmutable(QStringLiteral("MediumFontSize")));

    addConfig(Settings::self(), m_widget);
}

#include "akregator_config_appearance.moc"

