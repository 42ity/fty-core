How to write web tests
======================

* create a new file in +web/commands+
* first line should be +test_it+ command which prints out test name
* anything you want to save as results should be sent via +>&5+
* while writing tests, you can use following helper functions:
** +api_get+ returns get of specified URL, argument is the URL part after
    version
** +api_get_json+ returns get of specified URL without any metadata and
    stripped of blank characters, argument is the URL part after version
* save expected output in +results/testname.sh.res+

Testing test
------------

Individual test can be run by calling +test_web.sh testname.sh+.
