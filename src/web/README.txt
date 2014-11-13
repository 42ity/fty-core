Initial BIOS REST component
===========================

This component will eventually support all REST API as defined in RFC-11.
Currently highly in progress and supports only time command.

Requirements
------------

Requires 'tntnet', 'libsodium', 'cxxtools', 'libsasl2' and 'saslauthd'. 
To be able to set date/time, also needs properly setup 'sudo' (example
configuration provided in the project documentation).

Testing
-------

Before testing, make sure that you have `saslauthd` running and accessible
locally at expected location. The `configure` script accepts the option
'--with-saslauthd-mux=/path/to/mux' or tests one of '/var/run/saslauthd/mux'
or '/var/run/sasl2/mux' to set it up accordinally.

On Debian 8 the SASL and PAM are pre-integrated, you only need to make
sure that the 'bios' user (with the currently hardcoded password 'nosoup4u')
has been created in the OS and added to the 'sasl' group. The project 
delivers a script `init-os-accounts.sh` made to set this all up for you.

On other OSes some manual configuration may be needed...

Ensure `saslauthd` is configured to use PAM, so if `testsaslauthd` does
not work in the `test_web.sh`, check your configuration.

SASL2 config in custom '/etc/sasl2/bios.conf':
----
    pwcheck_method: saslauthd
    mech_list: plain login
----

PAM config in custom '/etc/pam.d/bios' for openSUSE:
----
    #%PAM-1.0
    auth     include        common-auth
    account  include        common-account
    password include        common-password
    session  include        common-session
----

PAM config in custom '/etc/pam.d/bios' (or appended to '/etc/pam.conf')
for common PAM:
----
    auth    required        pam_env.so
    auth    required        pam_unix.so     try_first_pass
    account required        pam_unix.so     try_first_pass
    password        requisite       pam_cracklib.so
    password        required        pam_unix.so     use_authtok shadow try_first_pass
    session required        pam_limits.so
    session required        pam_unix.so     try_first_pass
    session optional        pam_umask.so
    session optional        pam_env.so
----

Verify SASL connectivity with:
----
:; /usr/sbin/testsaslauthd -u bios -p nosoup4u -s bios
----

Then you know you have correct credentials set in `test_web.sh`.

For tests of REST API, don't forget to preconfigure the database
(MySQL or MariaDB) with `ci-fill-db.sh`.

Afterwards you can run `make test` and in different terminal on
the same machine `test_web.sh` or wrapper `ci-test-restapi.sh`.

To test assets, run `mk_test_data.sh` which will generate static
files you can use to test assets using API calls defined in rfc-11.

TODO
----

Currently hacked together. Needs rewrite to use autotools, get rid of
hardcoded values and maybe some indentation and comments wouldn't hurt either.
Also at some point we need to rotate keys for security reason. Maybe
implementing API call for refreshing token sounds like a good idea to allow
people to prolong their session.
