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

#include <httpd.h>
#include <http_config.h>
#include <apr_optional.h>
#include <mod_log_config.h>

static const char *my_rss(request_rec *r)
{
    char *filename;
    filename = apr_psprintf(r->pool, "/proc/%s/status", ap_append_pid(r->pool, "", ""));

    apr_file_t *procfile;
    if (APR_SUCCESS != apr_file_open(&procfile, filename, APR_READ, APR_OS_DEFAULT, r->pool)) {
        return "-";
    }

    char *status = apr_palloc(r->pool, 1024);
    char *rss;
    while (apr_file_gets(status, 1024, procfile) == APR_SUCCESS) {
        rss = strstr(status, "VmRSS:");
        if (rss != NULL) {
            apr_file_close(procfile);

            rss += 6;
            while (*rss == ' ' || *rss == '\t') {
                rss++;
            }
            char *c = rss;
            while (*c != ' ') {
                c++;
            }
            *c = '\0';
            return (const char *) rss;
        }
    }

    apr_file_close(procfile);
    return "-";
}

static const char *log_memory(request_rec *r, char *a)
{
    return my_rss(r);
}

static int mlp_pre_config(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp)
{
    static APR_OPTIONAL_FN_TYPE(ap_register_log_handler) *log_pfn_register;

    log_pfn_register = APR_RETRIEVE_OPTIONAL_FN(ap_register_log_handler);

    if (log_pfn_register) {
        log_pfn_register(p, "j", log_memory, 0);
    }

    return OK;
}

static void mlp_register_hooks(apr_pool_t *p) {
    ap_hook_pre_config(mlp_pre_config, NULL, NULL, APR_HOOK_REALLY_FIRST);
}

module AP_MODULE_DECLARE_DATA log_performance_module = {
    STANDARD20_MODULE_STUFF,
    NULL, NULL, NULL, NULL, NULL,
    mlp_register_hooks,
};
