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

  Works, but considered it experimental until it’s polished. It has been only
  tested on GNU/Linux with the prefork MPM.

INSTALLATION
============

  1. You need the Apache2 HTTP Server development files. On Debian that means
     installing the package apache2-prefork-dev.

  2. Then using apxs2 to compile & install:

     $: apxs2 -c -i mod_log_performance.c

  3. Load the module by adding this to your configuration:

     LoadModule log_performance_module /path/to/mod_log_performance.so
  
USAGE
=====

   Once installed, you can now use the following new mod_log_config formatter:

     %j | The resident non-shared stack size (kB) for the request process

   If you are not sure how to use that, check mod_log_config documentation:

     http://httpd.apache.org/docs/2.2/mod/mod_log_config.html
