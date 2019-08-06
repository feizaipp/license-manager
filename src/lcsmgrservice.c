#include "lcsmgrservice.h"
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "common.h"
#include <syslog.h>

gboolean on_handle_get_feedback(LcsMgrBase* skeleton,
                                                    GDBusMethodInvocation *invocation,
                                                    gpointer user_data)
{
    char *md5;

    md5 = get_feedback();
    lcs_mgr_base_complete_get_feedback(skeleton, invocation, md5);
    g_free(md5);

    return TRUE;
}

gboolean on_handle_insert_license(LcsMgrBase* skeleton,
                                                    GDBusMethodInvocation *invocation,
                                                    gchar *path,
                                                    gpointer user_data)
{
    int ret;

    ret = insert_license(path);

    lcs_mgr_base_complete_insert_license(skeleton, invocation, ret);

    return TRUE;
}

gboolean on_handle_check_license(LcsMgrBase* skeleton,
                                                    GDBusMethodInvocation *invocation,
                                                    gpointer user_data)
{
    int ret;

    ret = check_license();

    lcs_mgr_base_complete_check_license(skeleton, invocation, ret);

    return TRUE;
}
