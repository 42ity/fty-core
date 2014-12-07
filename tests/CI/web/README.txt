How to write web tests
======================

NOTE: Paths below begin with the project source root; the REST API / web
test suite resides under '$srcdir/tests/CI/web/' in this source code tree.

To configure a new automated test, do the following:

 * create a new file under the '$srcdir/tests/CI/web/commands/' directory
 * first line should be the `test_it` command which prints out test name
 * anything you want to save as results should be sent via `>&5`
 * while writing tests, you can use following helper functions:
 ** 'api_get' -- returns HTTP GET of specified URL, argument is the URL part
after version
 ** 'api_get_json' -- returns HTTP GET of specified URL without any metadata
and stripped of blank characters, argument is the URL part after version
 * save expected output in '$srcdir/tests/CI/web/results/testname.sh.res'
file for later comparison during automated tests

Testing a test
--------------
Individual tests can be executed by calling `test_web.sh testname.sh`.

