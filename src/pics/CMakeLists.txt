



########### install files ###############

install( FILES akregator_flag.png kmmsgflag.png kmmsgnew.png kmmsgunseen.png  DESTINATION ${DATA_INSTALL_DIR}/akregator/pics)

kde4_install_icons( ${ICON_INSTALL_DIR}   )




#original Makefile.am contents follow:

#pics_DATA = akregator_flag.png kmmsgflag.png kmmsgnew.png kmmsgunseen.png
#picsdir = $(kde_datadir)/akregator/pics
#
#KDE_ICON = AUTO
#
##EXTRA_DIST = $(pics_DATA)
##akregatoricondir = $(kde_datadir)/akregator/icons
##akregator_ICON = rss_label
