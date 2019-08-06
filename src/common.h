#ifndef _COMMON_H
#define _COMMON_H

struct hardware_info {
    char *board_info;
    char *cpu_info;
    char *disk_info;
    char *mac_info;
};

char *get_feedback(void);
int insert_license(char *path);
int check_license(void);

#endif