Initial BIOS REST component
===========================

This component will eventually support all REST API as defined in RFC-11.
Currently highly in progress and supports only time command.

Requirements
------------

Requires +tntnet+, +libsodium+, +cxxtools+ and +libsasl2+. To be able to set
date/time, also needs properly setup sudo.

Testing
-------

Before testing, make sure that you have sasl running and accessible locally at
expected location - currently hardcoded to +/var/run/saslauthd/mux+ and that
you have correct credentials set in +test_web.sh+. Afterwards you can run
+make test+ and in different terminal on the same machine +test_web.sh+.

To test assets, run +mk_test_data.sh+ which will generate static files you can
use to test assets using API calls defined in rfc-11.

TODO
----

Currently hacked together. Needs rewrite to use autotools, get rid of
hardcoded values and maybe some indentation and comments wouldn't hurt either.
Also at some point we need to rotate keys for security reason. Maybe
implementing API call for refreshing token sounds like a good idea to allow
people to prolong their session.
