= The tools/builder.sh script
:Author:        Evgeny Klimov
:Email:         <EvgenyKlimov@eaton.com>

The project includes a `tools/builder.sh` script whose main purpose
is to automate the repetitive standard GNU automake procedure of
`./autogen.sh && ./configure && make && make install` with further
optimizations to run a parallelized build when possible, into a
single script with several short-named methods to quickly run the
frequently needed building scenarios.

NOTE: Currently the `./configure` script is called without any
command-line parameters. Any environment variables which influence
it (like 'CFLAGS') should pass through from the caller's shell, though.

As part of development and maintenance of the 'Makefile' and related
build files, it occasionally happens that something happens differently
if the build is executed "in-place" from the project's root directory,
or "relocated" from another directory, or kicked around with the evil
`make dist` or `make distcheck` logic. To simplify verification that
everything behaves consistently in these cases, the `builder.sh` also
automates these three build modes.

This script can be executed explicitly or via our `autogen.sh` by
passing any command-line parameters to it -- in that case `autogen.sh`
completes its job of making sure that the `configure` script exists
and is up to date, and then falls through to execute the `builder.sh`.
Conversely, if `builder.sh` is called by itself, it first executes
`autogen.sh` to make sure the `configure` script is up to date, and
then proceeds with its own application logic to automate the build
method requested by the user.

Calling either `./autogen.sh` or `./tools/builder.sh` without any
command-line parameters should do just the default autogen job --
make sure that the `confugure` script exists and is up to date, and
quit with success (or fail otherwise).


== Command-line options
Currently these optional modifier are supported on the command-line:

 * '--warnless-unused' -- this suppresses the compiler warnings about
unused variables (see details below in the 'WARNLESS_UNUSED' envvar
description).
 * '--noparmake' or '--disable-parallel-make' -- this sets 'NOPARMAKE=yes'
(see below) for this invokation of the script i.e. to override the current
environment variable
 * '--parmake' or '--enable-parallel-make' -- this sets 'NOPARMAKE=no'
to enable parallel makes (enabled by default unless envvars forbid)

The first command-line attribute that is not an option defined above
is considered to be the request of a command-line method as described
below.


== Command-line methods
The currently defined following methods (selected by first attribute
on the command-line) include:

 * (no attributes) Recreate the `configure` script if needed, and exit
 * 'help' -- display short usage notice on currently supported methods
 * 'build' or 'build-samedir' -- (re)create `configure` if needed,
run it in the project root directory, execute first a parallel (for
speed) and then a sequential (for correctness) `make` for targets
that are named further on the command line (or default to 'all' as
implicitly defined in the 'Makefile')
 * 'build-subdir' -- same as above, except that a subdirectory is
created and changed into before doing the job
 * 'install' or 'install-samedir' -- do the 'build-samedir' routine
(including the wiping of the workspace from earlier build products)
followed by a `make install`
 * 'install-subdir' -- do the 'build-subdir' routine followed by
a `make install`
 * 'configure' -- (re)create `configure` if needed, clean up the
project root directory with a `make distclean`, and run `./configure`
 * 'distcheck' -- (re)create `configure` if needed, clean up the
project root directory with a `make distclean`, run `./configure`
in the project root directory, and finally run `make distcheck`
 * 'distclean' -- (re)create `configure` if needed, run it in the
project root directory to create the proper 'Makefile', and run
`make distclean` to delete everything as configured in 'Makefile'
 * 'make' or 'make-samedir', and 'make-subdir' -- run just the parallel
and sequential 'make' routine for the optionally specified target(s)
from the relevant (base or "relocated") directory; that is -- do not
cleanup and reconfigure the build area

Any parameters on the command line after the method specification
are processed according to the method. Currently this means the
optional list of 'Makefile' targets for the 'make-samedir',
'make-subdir', 'build-samedir', 'build-subdir', 'install-samedir'
and 'install-subdir' methods, and ignored for others.



== Environment variables
The script's behavior can be tuned by environment variables, which 
allows for repetitive tuning during the development and rebuild
cycle, while the command-line to execute the build remains short.

Some such variables are intended as "flags" set by the user so as to
modify nuances in behavior, others provide specific values for some
working variables so the script does not have to guess them or fall
back onto hardcoded defaults.

All of these are optional.


=== 'CHECKOUTDIR' path
The 'CHECKOUTDIR' is an optionally defined (usually by our CI scripts)
directory name which should contain the project sources.

The `builder.sh` script rebases into the root of the project sources
as specified by 'CHECKOUTDIR' if present, or guessed from the script's
own path name by default. Then during the script's work the variable
is redefined to contain the full filesystem path of the root of project
sources.


=== 'BLDARCH' tag
The 'BLDARCH' is a string tag for the "relocated" build environment
to use uniquely named directories for the compilation and installation.
This way the same checked-out copy of the source code (or the same
developer workspace) can be used by several building processes.
This is generally needed for multiplatform builds, which we do intend
to support due to x86 development and ARM target devices.

As a bonus, the "relocated build" products are stored in a separate
subdirectory and do not pollute the directories with the source code.

The default value depends on the compilation host's OS and CPU:
----
:; BLDARCH="`uname -s`-`uname -m`"
----


=== 'DESTDIR' path
The 'DESTDIR' generally specifies the prefix used during `make install`
and is the path prepended to the root directory assumed by the project.
That is, the built files are copied into '$DESTDIR/usr/some/thing' when
the installation target is being made.

The default value generally is empty (install into the currently running
OS), and the default value in `builder.sh` depends on '$BLDARCH':
----
:; DESTDIR="/var/tmp/bios-core-instroot-${BLDARCH}"
----


=== 'BUILDSUBDIR' path
If the "relocated" build is invoked ('make-subdir', 'build-subdir', or
'install-subdir'), then the specified directory (absolute, or relative
to the '$CHECKOUTDIR') is used to contain the temporary build products.

The default value in `builder.sh` depends on '$BLDARCH':
----
:; BUILDSUBDIR="build-${BLDARCH}"
----



=== 'WARNLESS_UNUSED' toggle
`export WARNLESS_UNUSED=yes` is one of two ways to quickly suppress the
compiler warnings about unused variables (there is a lot of those in
some of our automatically generated code) so that they do not distract
the developer from noticing some more severe errors.

If suppression of these warnings is enabled by the environment variable
'WARNLESS_UNUSED=yes' or by the command-line option '--warnless-unused',
the `builder.sh` script appends some compiler options to 'CFLAGS' and
'CXXFLAGS'. See the definition in the script source, but at the time of
this writing, the added `gcc` flags are:
----
  -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable
----


=== 'NOPARMAKE' toggle
The `builder.sh` script makes a lot of effort to build the project in
parallel first (for speed, with a best-effort approach ignoring any
errors that might pop up) and afterwards sequentially in order of
dependencies to make sure everything is built and linked correctly.

Still, sometimes there may be concerns that some interim failure of
the parallel make uncovered some race conditions or other brokenness
in our 'Makefile' or in the tools used, and this should generally be
fixed.

`export NOPARMAKE=yes` allows to confirm or rule out such problems: it
tells `builder.sh` to skip the parallel building attempts and proceed
to a sequential `make` right after a successful `configure`.

A sequential-only build also allows easier tracing of the build logs,
as each task is done one by one in order and is easily linked with a
reported error, if any.


=== 'NCPUS' count (semi-private)
The 'NCPUS' variable contains the number of processors available for
the build on this system. While this allows a developer to influence
roughly how much of the hardware resources the build run may consume
(via `make` parallelization), this variable is not quite intended for
manual specification -- rather use 'NPARMAKES' for actual control.

The `builder.sh` script tries several methods to detect the number of
CPUs on the system, or defaults to "1" upon errors.


=== 'NPARMAKES' count
The 'NPARMAKES' variable contains the number of `make` processes that
should be run in parallel on the first pass (if not disabled with the
'NOPARMAKE' toggle).

It may be set by the developer as desired, otherwise it defaults to
twice the 'NCPUS' per general recommendation (two processes per CPU,
one is actively compiling and another is waiting for disk I/O), or
to "2" upon errors.



