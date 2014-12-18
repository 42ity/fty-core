if HAVE_A2X

# Targets manpages for "CLI" program
SRC_CLI_PAGES = cli.txt
MAN8_CLI_PAGES = cli.8
MAN_CLI_PAGES = $(MAN8_CLI_PAGES)

### Aggregate the known manpage files and types by man-section
### http://en.wikipedia.org/wiki/Man_page#Manual_sections

# Section 1 - General commands
man1_MANS =

# Section 3 - Library functions, covering in particular the C standard library
man3_MANS =

# Section 5 - File formats and conventions
man5_MANS =

# Section 8 - System administration commands and daemons
man8_MANS = $(MAN8_CLI_PAGES)

# The ultimate list of man pages to build and redistribute
MAN_MANS = $(MAN1_MANS) $(MAN3_MANS) $(MAN5_MANS) $(MAN8_MANS)

# Generic targets
all-local: $(MAN_MANS)

# Make sure our source texts are available in the redistributable source
EXTRA_DIST = $(SRC_CLI_PAGES) $(MAN_MANS)

# Notes: we don't clean MAN_MANS, to avoid regeneration before dist
CLEANFILES = *.xml *.html

SUFFIXES = .txt .html .1 .3 .5 .8

# Actual build rules
# NOTE: Even though a2x prints the warning below, the option is essential for
# the resulting manpage to appear in the build directory rather than near the
# original source file, so just ignore it:
#   a2x: WARNING: --destination-dir option is only applicable to HTML based outputs
A2X_MANPAGE_OPTS = --doctype manpage --format manpage --destination-dir=$(builddir)

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

# Needed for automake, remains empty
clean-local:

