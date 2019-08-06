#ifndef __LCSMGR_SERVICE_H__
#define __LCSMGR_SERVICE_H__

#include "lcsmgr-generated.h"
gboolean on_handle_get_feedback(LcsMgrBase* skeleton,
                                                    GDBusMethodInvocation *invocation,
                                                    gpointer user_data);
gboolean on_handle_insert_license(LcsMgrBase* skeleton,
                                                    GDBusMethodInvocation *invocation,
                                                    gchar *path,
                                                    gpointer user_data);
gboolean on_handle_check_license(LcsMgrBase* skeleton,
                                                    GDBusMethodInvocation *invocation,
                                                    gpointer user_data);
#endif