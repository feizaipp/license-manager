#include "common.h"
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include "hwinfo.h"
#include "md5.h"
#include <fcntl.h>
#include <string.h>
#include<openssl/pem.h>
#include<openssl/ssl.h>
#include<openssl/rsa.h>
#include<openssl/evp.h>
#include<openssl/bio.h>
#include<openssl/err.h>

char pub_key[]="-----BEGIN PUBLIC KEY-----\n"\
                        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDFvBuyboKwa+BqFLzOqW70Fj91\n"\
                        "QA/cPLp82cMZgVOWfXFzT/V22BFIvH3xDS/0Jbniphf7VVLCsxi8bd1EUTN2oaZM\n"\
                        "9S7oxG06PgASKKF0rsAfFX0Rehjq/dGlmtJLmmyxWIgZIHV8lmum4ZLhWWd8xvov\n"\
                        "tDwEwVDnq+bAKSR//wIDAQAB\n"\
                        "-----END PUBLIC KEY-----\n";

static struct hardware_info *get_hw_info(void)
{
    struct hardware_info *hw_info = NULL;

    hw_info = malloc(sizeof(*hw_info));
    if (hw_info == NULL) {
        g_printerr("malloc failed.\n");
        return NULL;
    }

    hw_info->cpu_info = get_cpu_id();
    hw_info->disk_info = get_disk_serial_number();
    hw_info->mac_info = get_mac_address();
    hw_info->board_info = get_board_serial_number();
    return hw_info;
}

static int write_file(unsigned char *data, int len)
{
    int fd = -1;
    int i = 0;
    char val[3];

    fd = open("/root/feedback.txt", O_RDWR | O_CREAT | O_TRUNC);
    if (fd < 0) {
        g_printerr("open /root/feedback.txt failed.\n");
        return 0;
    }
    for (i=0; i<len; i++) {
        snprintf(val, sizeof(val), "%02x", data[i]);
        write(fd, val, strlen(val));
    }
    close(fd);
    return 1;
}

static char *get_hw_info_serial(void)
{
    char serial[2048];
    struct hardware_info *hw_info = NULL;
    char *info = NULL;

    hw_info = get_hw_info();
    if (hw_info == NULL) {
        g_printerr("hw_info is NULL.\n");
        return 0;
    }
    memset(serial, 0, sizeof(serial));
    strcat(serial, hw_info->board_info);
    strcat(serial, hw_info->cpu_info);
    strcat(serial, hw_info->disk_info);
    strcat(serial, hw_info->mac_info);

    g_free(hw_info->board_info);
    g_free(hw_info->cpu_info);
    g_free(hw_info->disk_info);
    g_free(hw_info->mac_info);
    free(hw_info);

    info = g_strdup(serial);
    return info;
}

static char *comput_md5_value(char *data)
{
    char *val = NULL;
    unsigned char decrypt[16];
    char str_code[33];
	MD5_CTX md5;
    int i = 0;
    char *p = str_code;

    memset(str_code, 0, sizeof(str_code));
	MD5Init(&md5);
	MD5Update(&md5, data, strlen((char *)data));
	MD5Final(&md5, decrypt);
    for (i=0; i<sizeof(decrypt); i++) {
        snprintf(p, 3, "%02x", decrypt[i]);
        p += 2;
    }

    val = g_strdup(str_code);
    return val;
}

static int write_data_to_file(char *data, int len)
{
    int ret;

	ret = write_file(data, len);
    return ret;
}

static char *get_md5_str(void)
{
    char *serial = NULL;
    char *md5 = NULL;

    serial = get_hw_info_serial();
    if (serial == NULL) {
        g_printerr("get hw info serial failed.\n");
        return 0;
    }

    md5 = comput_md5_value(serial);
    g_free(serial);
    if (md5 == NULL) {
        g_printerr("get md5 failed.\n");
        return NULL;
    }

    return md5;
}

static RSA* createRSA(unsigned char*key, int publi)
{
    RSA *rsa= NULL;
    BIO*keybio ;
    keybio= BIO_new_mem_buf(key, -1);
    if (keybio==NULL) {
        g_printerr("Failed to create key BIO\n");
        return 0;
    }

    if (publi) {
        rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
    } else {
        rsa= PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
    }
    if (rsa== NULL) {
        g_printerr("Failed to create RSA\n");
    }
    return rsa;
}

static int public_verify(const unsigned char *in_str, unsigned int in_len, unsigned char *outret, unsigned int outlen, unsigned char*key)
{
    RSA* rsa = createRSA(key, 1);
    int result = RSA_verify(NID_sha1, in_str, in_len, outret, outlen, rsa);
    if (result != 1) {
        g_printerr("verify error\n");
        return -1;
    }
    return result;
}

static long get_file_len(char *path)
{
    FILE *pFile;
    long size = 0;

    pFile = fopen(path, "rb");
    if (pFile==NULL) {
        g_printerr("open %s file failed.\n", path);
    } else {
        fseek(pFile, 0, SEEK_END);
        size = ftell(pFile);
        fclose(pFile);
    }
    return size;
}

static char *read_data(char *path, long *data_len)
{
    char *data = NULL;
    long len = 0;
    int fd = -1;

    len = get_file_len(path);
    data = malloc(len);
    if (data == NULL) {
        g_printerr("malloc failed\n");
        return NULL;
    }
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        g_printerr("open %s failed.\n", path);
        return NULL;
    }
    read(fd, data, len);
    *data_len = len;
    close(fd);
    return data;
}

static int judge_license(char *path)
{
    char *md5 = NULL;
    char *data = NULL;
    long data_len;
    int ret;

    md5 = get_md5_str();
    if (md5 == NULL) {
        g_printerr("get md5 failed.\n");
        return 0;
    }
    data = read_data(path, &data_len);
    if (data == NULL) {
        g_printerr("read data failed.\n");
        return 0;
    }
    ret =  public_verify((const unsigned char*)md5, strlen(md5), data, data_len, (unsigned char*)pub_key);
    free(data);
    if (ret == 1) {
        return 1;
    } else {
        return 0;
    }
}

static void store_license(char *path)
{
    int fd = -1;
    char *data = NULL;
    long data_len;

    data = read_data(path, &data_len);
    if (data == NULL) {
        g_printerr("read data failed.\n");
        return;
    }
    fd = open("/var/lib/lcsmgrservice/.license", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR|S_IWUSR);
    if (fd < 0) {
        g_printerr("open .license failed.\n");
        free(data);
        return;
    }
    write(fd, data, data_len);
    close(fd);
}

char *get_feedback(void)
{
#if 0
    char *md5 = NULL;

    md5 = get_md5_str();
    if (md5 == NULL) {
        g_printerr("get md5 failed.\n");
        return 0;
    }
    ret = write_data_to_file(md5, strlen(md5));
    g_free(md5);

    return ret;
#else
    char *md5 = NULL;

    md5 = get_md5_str();
    if (md5 == NULL) {
        g_printerr("get md5 failed.\n");
        return 0;
    }
    return md5;
#endif
}

int insert_license(char *path)
{
    if (judge_license(path)) {
        store_license(path);
        return 1;
    }
    return 0;
}

int check_license(void)
{
    struct stat st;
    const char *file_name = "/var/lib/lcsmgrservice/.license";

    if (stat(file_name, &st) < 0) {
        if (errno == ENOENT) {
            return 0;
        }
    }
    if (judge_license(file_name)) {
        return 1;
    }
    return 0;
}
