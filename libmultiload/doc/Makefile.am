
all: html/index.html

html/index.html: ../src/include/multiload.h doc-pages.h doxygen.config DoxygenLayout.xml style.css custom_header.html custom_footer.html
	@-rm -rf html/
	@mkdir -p html/
	$(DOXYGEN) doxygen.config
	@-rm -f \
		html/bc_s.png \
		html/doc.png \
		html/doxygen.png \
		html/dynsections.js \
		html/nav_f.png \
		html/nav_g.png \
		html/nav_h.png \
		html/splitbar.png \
		html/tab_a.png \
		html/tab_b.png \
		html/tab_h.png \
		html/tab_s.png \
		html/tabs.css

install-data-local: html/index.html
	@$(NORMAL_INSTALL)
	$(MKDIR_P) $(htmldir)/html
	$(INSTALL) html/* $(htmldir)/html

uninstall-local:
	@$(NORMAL_UNINSTALL)
	rm -rf $(htmldir)/html

clean-local:
	rm -rf html/
