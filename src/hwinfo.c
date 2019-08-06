#include <stdio.h>
#include <string.h>
#include "hwinfo.h"

static char *parse_hardware_info(const char * file_name, const char * match_words)
{
    char line[1024];
    char *hw_info = NULL;
    char *info = NULL;

    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        g_printerr("fopen file:%s failed\n", file_name);
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        hw_info = strstr(line, match_words);
        if (NULL == hw_info) {
            continue;
        }
        hw_info += strlen(match_words);
        break;
    }
    fclose(file);
    info = g_strdup(hw_info);
    return info;
}

static char *get_mac_address_by_system(void)
{
    char *mac_info = NULL;

    const char * lshw_result = "/var/lib/lcsmgrservice/.lshw_result.txt";
    char command[512] = { 0 };
    snprintf(command, sizeof(command), "lshw -c network | grep serial | head -n 1 > %s", lshw_result);

    if (0 == system(command)) {
        mac_info = parse_hardware_info(lshw_result, "serial: ");
    }

    unlink(lshw_result);

    return mac_info;
}

static char *get_cpu_id_by_asm(void)
{
    char cpu[32] = { 0 };
    unsigned int s1 = 0;
    unsigned int s2 = 0;
    char *cpu_info = NULL;

    asm volatile
    (
        "movl $0x01, %%eax; \n\t"
        "xorl %%edx, %%edx; \n\t"
        "cpuid; \n\t"
        "movl %%edx, %0; \n\t"
        "movl %%eax, %1; \n\t"
        : "=m"(s1), "=m"(s2)
    );
    if (0 == s1 && 0 == s2) {
        return NULL;
    }
    snprintf(cpu, sizeof(cpu), "%08X%08X", htonl(s2), htonl(s1));
    cpu_info = g_strdup(cpu);
    return cpu_info;
}

static char *get_board_serial_by_system(void)
{
    const char * dmidecode_result = "/var/lib/lcsmgrservice/.dmidecode_result.txt";
    char command[512] = { 0 };
    char *board_info = NULL;

    snprintf(command, sizeof(command), "dmidecode -t 2 | grep Serial > %s", dmidecode_result);

    if (0 == system(command)) {
        board_info = parse_hardware_info(dmidecode_result, "Serial Number: ");
    }

    unlink(dmidecode_result);

    return board_info;
}

static char *get_disk_serial_interface_properties (GDBusProxy *proxy)
{
    gchar **cached_properties;
    guint n;
    GVariant *value;
    gboolean removable;
    char *disk_info = NULL;

    value = g_dbus_proxy_get_cached_property(proxy, "Removable");
    if (value) {
        removable = g_variant_get_boolean(value);
        if (removable == FALSE) {
            value = g_dbus_proxy_get_cached_property(proxy, "Serial");
            disk_info = g_variant_dup_string (value, NULL);
            g_variant_unref (value);
        }
        g_variant_unref (value);
    }
    return disk_info;
}

static char *get_disk_serial_interface (UDisksObject *object)
{
    GList *interface_proxies;
    GList *l;
    char *disk_info = NULL;

    interface_proxies = g_dbus_object_get_interfaces (G_DBUS_OBJECT (object));
    for (l = interface_proxies; l != NULL; l = l->next)
    {
        GDBusProxy *iproxy = G_DBUS_PROXY (l->data);
        disk_info = get_disk_serial_interface_properties (iproxy);
        if (disk_info)
            break;
    }
    g_list_foreach (interface_proxies, (GFunc) g_object_unref, NULL);
    g_list_free (interface_proxies);

    return disk_info;
}

char *get_disk_serial_number(void)
{
    GList *objects;
    GList *l;
    GError *error = NULL;
    char *disk_info = NULL;
    UDisksClient *client = NULL;

    if (0 != getuid()) {
        g_printerr("please use root.\n");
        return NULL;
    }

    client = udisks_client_new_sync (NULL, /* GCancellable */ &error);

    objects = g_dbus_object_manager_get_objects (udisks_client_get_object_manager (client));
    for (l = objects; l != NULL; l = l->next)
    {
        UDisksObject *object = UDISKS_OBJECT (l->data);

        disk_info = get_disk_serial_interface(object);
        if (disk_info)
            break;
    }
    g_list_foreach (objects, (GFunc) g_object_unref, NULL);
    g_list_free (objects);
    return disk_info;
}


char *get_board_serial_number(void)
{
    char *board_info = NULL;

    if (0 != getuid()) {
        g_printerr("please use root.\n");
        return NULL;
    }
    board_info = get_board_serial_by_system();

    return board_info;
}

char *get_mac_address(void)
{
    char *mac_info = NULL;

    if (0 != getuid()) {
        g_printerr("please use root.\n");
        return NULL;
    }

    mac_info = get_mac_address_by_system();

    return mac_info;
}

char *get_cpu_id(void)
{
    char *cpu_info = NULL;

    if (0 != getuid()) {
        g_printerr("please use root.\n");
        return NULL;
    }

    cpu_info = get_cpu_id_by_asm();

    return cpu_info;
}
