kde4_header()




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
