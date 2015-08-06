Here lie the XML sources for message protocol definitions, and the
C++ sources and headers generated from them with ZPROTO and GSL,
and possibly post-processed manually. Currently this process is
done outside of automated make routine.

The *_msg.* files found here are an internal implementation detail
du-jour and may be removed later (possibly, only a few XMLs will
remain here).

There are also a few build-products of this procedure contained in
the src/api/ directory for the APP and YMSG classes that form our
public API. Corresponding headers are in the project root /include.

