The sources in this directory implement the $BIOS project public API.
Corresponding headers are in the project root /include.

Some of the C++ sources and headers are generated with ZPROTO and GSL,
and possibly post-processed manually. Currently this process is done
outside of automated make routine. Relevant XML sources for message
protocol definitions are in src/msg/:

* `app.c` is generated from `src/msg/application.xml`
* `ymsg.c` is generated from `src/msg/rozp.xml`
* `bios_agent.c` is written and maintained manually

