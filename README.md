INTRODUCTION
============

  mod_log_performance is a module for Apache HTTP Server 2.x.

  For each request, it allows you to log the amount of memory used by the
  process that served it as seen from the system’s point of view (e.g. like
  what’s usually reported by tools like ps or top), but it’s handy this way
  because it gets logged per request allowing you to later process the logs
  (e.g. grep, awk, sort and friends) to see which requests caused the process 
  memory to increase.

STATUS
======

  Works, but is pretty experimental at this stage. It has been only tested on
  GNU/Linux with the prefork MPM.

INSTALLATION
============

  1. You need the Apache2 HTTP Server development files. That means installing
     the package apache2-prefork-dev on Debian or httpd-devel on Redhat.

  2. Then using apxs to compile & install:

     $: apxs -c -i mod_log_performance.c

     Note: On Debian it's `apxs2`

  3. Load the module by adding this to your configuration:

     LoadModule log_performance_module /path/to/mod_log_performance.so
  
USAGE
=====

  Once installed, you can now use the following new mod_log_config specifiers:

    %j | The resident set size in kB for the request process
    %J | Like %j, but logs the change (delta) from previous request
    %c | The absolute CPU user time in ms for the request process

  If you are not sure how to use that, check mod_log_config documentation:

    http://httpd.apache.org/docs/2.2/mod/mod_log_config.html

TODOs
=====

  * Add more features (i.e. formatting specifiers)
    * CPU user time delta
  * Support the Worker (multi-threaded) MPM
  * Build scripts
  * Testing on other platforms supported by Apache HTTP server, which include
    but not limited to: OpenBSD, FreeBSD, NetBSD, Solaris, AIX, OS X, HPUX
  * Support Win32 (since I rely on /proc filesystem, I know for sure that Win32
    is currently not supported)
