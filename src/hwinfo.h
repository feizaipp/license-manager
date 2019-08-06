#ifndef  __HWINFO_H_
#define __HWINFO_H_
#include <udisks/udisks.h>

char *get_mac_address(void);
char *get_cpu_id(void);
char *get_board_serial_number(void);
char *get_disk_serial_number(void);

#endif