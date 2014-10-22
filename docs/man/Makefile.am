if HAVE_A2X
# Targets manpages
SRC_CLI_PAGES = cli.txt
MAN_CLI_PAGES = cli.8

man8_MANS = $(MAN_CLI_PAGES)

MAN_MANS = $(MAN_CLI_PAGES)

# Generic targets
all-local: $(MAN_MANS)

EXTRA_DIST = $(SRC_CLI_PAGES) $(MAN_MANS)

# Notes: we don't clean MAN_MANS, to avoid regeneration before dist
CLEANFILES = *.xml *.html

SUFFIXES = .txt .html .1 .3 .5 .8

# Actual build rules

A2X_MANPAGE_OPTS = --doctype manpage --format manpage

.txt.1:
	$(A2X) $(A2X_MANPAGE_OPTS) $<

.txt.3:
	$(A2X) $(A2X_MANPAGE_OPTS) $<

.txt.5:
	$(A2X) $(A2X_MANPAGE_OPTS) $<

.txt.8:
	$(A2X) $(A2X_MANPAGE_OPTS) $<

else

all-local:
	@echo "SKIP: Missing 'a2x' program, manpage generation skipped!" >&2

endif
