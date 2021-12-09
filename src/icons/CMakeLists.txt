# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
file(GLOB icons "*-apps-akregator.*")
ecm_install_icons(ICONS
    ${icons}
    16-apps-akregator_empty.png
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)

