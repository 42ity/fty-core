= The tools/init-os-accounts.sh script
:Author:        Evgeny Klimov
:Email:         EvgenyKlimov@eaton.com

The project includes a `tools/init-os-accounts.sh` script whose main
purpose is to automate and standardize the creation of the user and
group accounts for the BIOS project, including (optionally) passwords.

These accounts (and the passwords) are further used in the automated
testing scripts, as well in the system security setup example files
and documentation.

NOTE: At the moment the script supports just one user and group, but
its code is sufficiently modularized to possibly easily generate more
accounts in a loop, if that is later needed...

The script can create the accounts in the running system as well as
in an "alternate root" directory which hosts a filesystem image being
prepared. Since the system utilities use `chroot` to manipulate the
"alternate root", the script tries to bootstrap a sufficient structure
if pointed at a location which does not have the needed files and
libraries.

Like many other scripts in the project, this one can be tuned by the
environment variables, and falls back to defaults expected by other
components.



== Environment variables
The script's behavior can be tuned by environment variables, which
allows for repetitive tuning during the development cycle (especially
in CI-driven rebuilds in a clean environment), while the command-line
to execute the build remains short.

Some such variables are intended as "flags" set by the user so as to
modify nuances in behavior, others provide specific values for some
working variables so the script does not have to guess them or fall
back onto hardcoded defaults.

All of these are optional.


=== True variables that can be overridden by the caller

==== 'DEBUG'
A non-empty value causes more informative (and maybe intimate) output
that can be logged by the caller, such as the (hardcoded default)
password or the password hash value ultimately used by the script.

==== 'GROUP_NAME'
Name of the group that would be created for assignment of access
rights (in SASL/PAM and SUDO configurations in particular), and
is the default group for the user account to execute the $BIOS
project's daemons (see '$USER_NAME' below).

==== 'USER_ADD_GROUPS'
Additional groups where the '$USER_NAME' user account belongs,
such as 'sasl' in our default setup.

This can be set to special value of "minus" or "dash" ('-') in
order to skip assignment of secondary groups.

==== 'USER_NAME'
Name of the user account to execute the $BIOS project's daemons.

==== 'USER_GECOS'
Description (Name, Location, Contact under GECOS markup) for the
user account in the system tables.

==== 'USER_SHELL'
The default shell interpreter used by the $BIOS daemon user account.

==== 'USER_HOME'
The home directory path used by the $BIOS daemon user account.

==== 'CREATE_HOME'
If this variable is not set, then the value of 'CREATE_HOME'
definition from '$ALTROOT/etc/login.defs' is used.

If the envvar 'CREATE_HOME' is valid, it overrides that from the
'login.defs' configuration.

Ultimately, if it is 'true', then the creation of a user's home
directory (under '$ALTROOT' generally) is requested during the
'genUser()' processing.

The default is to not create a home directory.

NOTE: (TODO?) There is currently no support for a 'false' value to
forcefully disable creation of the home directory.

==== 'USER_PASS' and 'USER_PASS_HASH'
Plain-text and encrypted password for the $BIOS daemon user account.
See also 'DEF_USER_PASS' and 'DEF_USER_PASS_HASH' below.

The main idea with these is that the caller can provide either the
plain-text password (for simplicity) or its hash prepared by one of
the algorithms supported by the OS (for security and reproducibility).
In the end, it is the hash that is actually used for account creation.
If NEITHER of these is provided, there is a fallback to hard-coded
defaults with values expected by the testing scripts, in particular.

==== 'ALTROOT'
Instead of the running OS, an alternate filesystem image rooted at
'$ALTROOT' may be modified by the script (assumes filesystem-based
databases of user/group account data, such as '/etc/passwd').

If that tree does not contain the files for the FS-based account
databases, then these files are created and will contain the few
lines with data just for the newly created accounts. If the task
is done in a blank directory, then this can be used basically to
create those lines and copy-paste (or concatenate) them into some
other storage, filesystem image, documentation, etc.

The location is required to contain an 'etc/' subdirectory, but if
files related to the filesystem-based are missing there -- that
generates just a warning but not a fatal failure (since other user
credential storage can generally be used by an image, such as LDAP).

==== 'ALTROOT_MAKEFAKE' toggle
If there is no expected content under '$ALTROOT' (and if 'ALTROOT'
is not '/'), then the special value of 'ALTROOT_MAKEFAKE=Y' forces
initialization of that location in order for the "shadow utilities"
('groupadd', 'useradd' and such) to function with filesystem-based
user/group account databases.




=== The internal and unoverridable values (hardcoded defaults)

==== 'MKPASSWD' path
Pathname of the `mkpasswd` program which generates Unix passwords.

==== 'OPENSSL' path
Pathname of the `openssl` program which generates Unix passwords
among many other capabilities.

==== 'LANG' and 'LC_ALL'
Forced to the value of 'C' for default ASCII 7-bit English text I/O.

==== 'DEF_USER_PASS' string
This variable contains the plain-text password which can be set for
the user account, and matches the expectations in the testing scripts
(the password defined there).

It is the default for the 'USER_PASS' variable if that is empty.

==== 'DEF_USER_PASS_HASH' string
This variable contains the cipher-text of the password in 'DEF_USER_PASS'
and is the actual value assigned during the user account creation.

It is the default for the 'USER_PASS_HASH' variable if that is empty.



=== Items of programmatic interest
Items below are implementation detail, needed only for maintenance and
further development of the script.

Most of the code is implemented as routines to facilitate testing or
future expansion such as to loop over several user accounts, or support
for specific numeric user/group IDs.

Beside sanity-checking of inputs (which at the moment includes creation
of an "alternate root" if requested and that location is empty), the
script's main logic currently is just to call 'genGroup', 'genUser' and
'verifyGU' -- and all the rest grows from there.

==== 'CODE' for the 'die()' routine
The 'die()' routine prints a timestamped "FATAL" message from its
arguments to 'stderr', and exits. The optional 'CODE' variable for it
can set the 'exit()' value (defaults to '1' if not provided), i.e.:
----
    CODE=123 die "Failed to open config file" "$FILENAME"
----

==== 'CURID'
Numeric user ID of the current account (who ran this script), or empty
if there is an error detecting it.

==== 'RUNAS'
Variable that contains the name of the routine used to elevate privileges
in the case that such elevation is needed (currently defaults to 'sudo'
for non-'root' users, empty for 'root').

This is optionally prepended to calls which may need elevation, by pattern
'$RUNAS prog args...' and so the command (or a routine in this script's
future versions) should take care of elevating all its arguments, assuming
the first one to be a program name/path.

==== 'hashPasswd()' routine
Creates a UNIX password hash using '$MKPASSWD' or '$OPENSSL' and the value
of global '$USER_PASS' as the plain-text password (no salt used for now).
The routine stores the result in global envvar '$USER_PASS_HASH' and returns
'0' if generation was successful (and this hash value is not empty).
Debian 8 as of today can generate and process at least the hashes below
(with special prefixes in the hash-string), and so the script attempts 
to generate them in this order of preference (from more to less secure):

 * 'sha-512' = '$6$'
 * 'sha-256' = '$5$'
 * 'md5' = '$1$'
 * 'crypt' ('des-56') = no prefix, just the hash about 13 characters long

==== 'genGroup()' routine
Creates the group account '$GROUP_NAME', ignoring errors about "not unique
name or number".

Specifying a fixed numeric group ID is currently not supported -- so one
is assigned by the system according to its settings.

==== 'genUser()' routine
Creates the user account '$USER_NAME' with password from '$USER_PASS_HASH'
(generated from '$USER_PASS' if the hash is empty, or defaulted to hardcoded
value upon failures) with a primary group '$GROUP_NAME'.

Optionally also attempts to create the home directory, if that was requested
via '$CREATE_HOME'.

Then if the list at '$USER_ADD_GROUPS' is not empty, the script attempts to
add those groups are as secondaries (but failures are not fatal).

Errors about "not unique name or number" and "can't create home directory"
are reported but ultimately ignored as far as exit codes are concerned.

Specifying a fixed numeric user or group ID is currently not supported --
so one is assigned by the system according to its settings for newly created
accounting entries.

==== 'verifyGU()' routine
A simple verifier of user/group account in an "alternate root" environment
(`grep` in filesystem-based databases) and a more complete verification
(with `getent` and `finger` if available and `id`) in a running OS context.



