kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/akregator/about FILES top-right-akregator.png  	main.html  	akregator.css )

kde4_footer()



#original Makefile.am contents follow:

#about_DATA = \
#	top-right-akregator.png \
#	main.html \
#	akregator.css
#
#aboutdir = $(kde_datadir)/akregator/about
