/*
 * Main file implementing mod_log_performance (MLP) for Apache HTTP Server 2.x
 *
 * Written by: Amr Mostafa <amr.mostafa@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "httpd.h"
#include "http_config.h"
#include "apr_optional.h"
#include "apr_strings.h"
#include "mod_log_config.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

static int pagesize = 1;

static const int mlp_get_rss(request_rec *r)
{
    char *filename;
    filename = apr_psprintf(r->pool, "/proc/%s/stat", ap_append_pid(r->pool, "", ""));

    apr_file_t *procfile;
    if (APR_SUCCESS != apr_file_open(&procfile, filename, APR_READ, APR_OS_DEFAULT, r->pool)) {
        return -1;
    }

    static char stat[1024];
    char *sp, *rss;
    // Read RSS from /proc/<pid>/stat, which is the 24th field
    while (apr_file_gets(stat, 1024, procfile) == APR_SUCCESS) {
        // Skip until the process name, which might be parser-unfriendly
        // This advances us 2 fields
        sp = strchr(stat, ')');
        sp += 2;

        // Skip 21 fields
        int i;
        for (i = 0; i < 21; i++) {
            sp = strchr(sp, ' ');
            if (sp == NULL) {
                return -1;
            }
            sp++;
        }

        // Read RSS
        rss = sp;
        while (*sp != ' ') {
            sp++;
        }
        *sp = '\0';

        // Convert to kB and return
        return (atoi(rss) * pagesize) / 1024;
    }

    apr_file_close(procfile);
    return -1;
}

static const char *log_cpu_utime(request_rec *r, char *a)
{
    struct rusage rs;
    getrusage(RUSAGE_SELF, &rs);

    return (const char *) apr_psprintf(r->pool, "%ld.%06ld", rs.ru_utime.tv_sec, rs.ru_utime.tv_usec);
}

static const char *log_rss(request_rec *r, char *a)
{
    const int rss = mlp_get_rss(r);
    if (rss <= 0) {
        return "-";
    }

    return (const char *) apr_itoa(r->pool, rss);
}

static const char *log_rss_delta(request_rec *r, char *a)
{
    static int previous_rss = -1;
    int delta, rss;

    rss = mlp_get_rss(r);

    if (rss >= 0 && previous_rss >= 0) {
        delta = rss - previous_rss;
        previous_rss = rss;
        return (const char *) apr_itoa(r->pool, delta);
    }

    previous_rss = rss;
    return "-";
}

static int mlp_pre_config(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp)
{
    static APR_OPTIONAL_FN_TYPE(ap_register_log_handler) *log_pfn_register;

    log_pfn_register = APR_RETRIEVE_OPTIONAL_FN(ap_register_log_handler);

    if (log_pfn_register) {
        log_pfn_register(p, "j", log_rss, 0);
        log_pfn_register(p, "J", log_rss_delta, 0);
        log_pfn_register(p, "c", log_cpu_utime, 0);
    }

    return OK;
}

static void mlp_init_child(apr_pool_t *p, server_rec *s)
{
    pagesize = getpagesize();
}

static void mlp_register_hooks(apr_pool_t *p) {
    ap_hook_pre_config(mlp_pre_config, NULL, NULL, APR_HOOK_REALLY_FIRST);
    ap_hook_child_init(mlp_init_child, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA log_performance_module = {
    STANDARD20_MODULE_STUFF,
    NULL, NULL, NULL, NULL, NULL,
    mlp_register_hooks,
};
