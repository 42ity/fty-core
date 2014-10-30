Initial BIOS REST component
===========================

This component will eventually support all REST API as defined in RFC-11.
Currently highly in progress and supports only time command.

Requirements
------------

Requires +tntnet+, +libsodium+, +cxxtools+, +libsasl2+ and +saslauthd+. To be
able to set date/time, also needs properly setup sudo.

Testing
-------

Before testing, make sure that you have saslauthd running and accessible
locally at expected location. configure.ac test one of +/var/run/saslauthd/mux+
+/var/run/sasl2/mux+ and setup it accordinally. Alternativelly you can pass the
path using --with-saslauthd-mux argument for configure script.

Ensure saslauthd is configured to use PAM, so if testsaslauthd does not work,
check your configuration.

    cat /etc/sasl2/bios.conf
    pwcheck_method: saslauthd
    mech_list: plain login

    # for openSUSE
    cat /etc/pam.d/bios
    #%PAM-1.0
    auth     include        common-auth
    account  include        common-account
    password include        common-password
    session  include        common-session

    # for common PAM
    auth    required        pam_env.so
    auth    required        pam_unix.so     try_first_pass
    account required        pam_unix.so     try_first_pass
    password        requisite       pam_cracklib.so
    password        required        pam_unix.so     use_authtok shadow try_first_pass
    session required        pam_limits.so
    session required        pam_unix.so     try_first_pass
    session optional        pam_umask.so
    session optional        pam_env.so

Then you know you have correct credentials set in +test_web.sh+. Afterwards you
can run +make test+ and in different terminal on the same machine
+test_web.sh+.

To test assets, run +mk_test_data.sh+ which will generate static files you can
use to test assets using API calls defined in rfc-11.

TODO
----

Currently hacked together. Needs rewrite to use autotools, get rid of
hardcoded values and maybe some indentation and comments wouldn't hurt either.
Also at some point we need to rotate keys for security reason. Maybe
implementing API call for refreshing token sounds like a good idea to allow
people to prolong their session.
