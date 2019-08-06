#include <stdio.h> 
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include "lcsmgr-generated.h"
#include <errno.h>
#include <getopt.h>
#include <string.h>

#define SIGNLEN (1024/8)

char pub_key[]="-----BEGIN PUBLIC KEY-----\n"\
                        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDFvBuyboKwa+BqFLzOqW70Fj91\n"\
                        "QA/cPLp82cMZgVOWfXFzT/V22BFIvH3xDS/0Jbniphf7VVLCsxi8bd1EUTN2oaZM\n"\
                        "9S7oxG06PgASKKF0rsAfFX0Rehjq/dGlmtJLmmyxWIgZIHV8lmum4ZLhWWd8xvov\n"\
                        "tDwEwVDnq+bAKSR//wIDAQAB\n"\
                        "-----END PUBLIC KEY-----\n";

static struct option const long_options[] =
{
    {"help", no_argument, 0, 'h'},
    {"insert", required_argument, 0, 'i'},
    {"gencode", required_argument, 0, 'g'},
    {NULL, 0, NULL, 0}
};

static void usage(char *name)
{
    printf("\
Usage: %s [OPTION]... [FILE]\n\
\t-h, --help      help info\n\
\t-i, --insert [FILE]       insert license\n\
\t-g, --gencode       gen feedback code\n", name);
    exit(1);
}

static long get_file_len(char *path)
{
    FILE *pFile;
    long size = 0;

    pFile = fopen(path, "rb");
    if (pFile==NULL) {
        printf("open %s file failed.\n", path);
    } else {
        fseek(pFile, 0, SEEK_END);
        size = ftell(pFile);
        fclose(pFile);
    }
    return size;
}

static char *read_data(char *path)
{
    char *data = NULL;
    long len = 0;
    int fd = -1;

    len = get_file_len(path);
    data = malloc(len);
    if (data == NULL) {
        printf("malloc failed\n");
        return NULL;
    }
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("open %s failed.\n", path);
        return NULL;
    }
    read(fd, data, len);
    close(fd);
    return data;
}

static void store_feedback(char *md5)
{
    int fd = -1;
    char path[1024];
    uid_t uid;
    uid_t NO_UID = -1;
    struct passwd *pw;

    uid = geteuid();
    pw = (uid == NO_UID && errno ? NULL : getpwuid(uid));
    if ((strcmp(pw->pw_name, "root") == 0) || \
        (strcmp(pw->pw_name, "auditadm") == 0) || \
        (strcmp(pw->pw_name, "secadm") == 0)) {
        snprintf(path, sizeof(path), "/%s/feedback.txt", pw->pw_name);
    } else {
        snprintf(path, sizeof(path), "/home/%s/feedback.txt", pw->pw_name);
    }
    fd = open(path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 0) {
        printf("open %s failed strerr:%s.\n", path, strerror(errno));
        return;
    }
    write(fd, md5, strlen(md5));
    close(fd);
    printf("gencode succ, save to %s\n", path);
}

static void get_feedback(void)
{
    LcsMgrBase *proxy = NULL;
    GError *error = NULL;
    char *ret_str;

    proxy = lcs_mgr_base_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                            G_DBUS_PROXY_FLAGS_NONE,
                                                            "org.freedesktop.LcsMgrService",
                                                            "/org/freedesktop/LcsMgrService/Base",
                                                            NULL,
                                                            &error);
    if (proxy == NULL) {
        g_printerr ("Failed to create proxy: %s\n", error->message);
    }
    lcs_mgr_base_call_get_feedback_sync(proxy, &ret_str, NULL, &error);
    if (ret_str != NULL) {
        store_feedback(ret_str);
        g_free(ret_str);
    }
}

static void insert_license(char *path)
{
    LcsMgrBase *proxy = NULL;
    GError *error = NULL;
    int ret;

    proxy = lcs_mgr_base_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                            G_DBUS_PROXY_FLAGS_NONE,
                                                            "org.freedesktop.LcsMgrService",
                                                            "/org/freedesktop/LcsMgrService/Base",
                                                            NULL,
                                                            &error);
    if (proxy == NULL) {
        g_printerr ("Failed to create proxy: %s\n", error->message);
    }
    lcs_mgr_base_call_insert_license_sync(proxy, path, &ret, NULL, &error);
    if (ret == 1) {
        printf("insert ok.\n");
    } else {
        printf("insert failed.\n");
    }
}

static int check_license(void)
{
    LcsMgrBase *proxy = NULL;
    GError *error = NULL;
    int ret;

    proxy = lcs_mgr_base_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                            G_DBUS_PROXY_FLAGS_NONE,
                                                            "org.freedesktop.LcsMgrService",
                                                            "/org/freedesktop/LcsMgrService/Base",
                                                            NULL,
                                                            &error);
    if (proxy == NULL) {
        g_printerr ("Failed to create proxy: %s\n", error->message);
    }
    lcs_mgr_base_call_check_license_sync(proxy, &ret, NULL, &error);
    if (ret == 1) {
        printf("check ok.\n");
    } else {
        printf("check failed, gen feedback.\n");
    }
    return ret;
}

int main(int argc, char **argv)
{
    unsigned char signret[SIGNLEN];
    unsigned int siglen;
    int optc;
    char *license_file;
    char resolved_path[1024];
    char *data = NULL;
    long data_len;
    char g_option = 0;
    char i_option = 0;

    while ((optc = getopt_long (argc, argv, "hgi:", long_options, NULL)) != -1) {
        switch (optc) {
        case 'h':
            usage(argv[0]);
            break;
            
        case 'g':
            g_option = 1;
            break;
        
        case 'i':
            i_option = 1;
            license_file = optarg;
            break;
        }
    }
    if ((!g_option && !i_option) || (g_option && i_option)) {
        usage(argv[0]);
    }

    if (g_option) {
        if (!check_license())
            get_feedback();
    }

    if (i_option) {
        realpath(license_file, resolved_path);
        insert_license(resolved_path);
    }

    return 0;
}