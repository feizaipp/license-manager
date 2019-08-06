#include<openssl/pem.h>
#include<openssl/ssl.h>
#include<openssl/rsa.h>
#include<openssl/evp.h>
#include<openssl/bio.h>
#include<openssl/err.h>
#include <stdio.h> 
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>

#define SIGNLEN (1024/8)

char priv_key[]="-----BEGIN RSA PRIVATE KEY-----\n"\
                            "MIICXAIBAAKBgQDFvBuyboKwa+BqFLzOqW70Fj91QA/cPLp82cMZgVOWfXFzT/V2\n"\
                            "2BFIvH3xDS/0Jbniphf7VVLCsxi8bd1EUTN2oaZM9S7oxG06PgASKKF0rsAfFX0R\n"\
                            "ehjq/dGlmtJLmmyxWIgZIHV8lmum4ZLhWWd8xvovtDwEwVDnq+bAKSR//wIDAQAB\n"\
                            "AoGAXknz3yMJWL0oNw2KdvWeffA59FxG89tVhgGFIh1kUYeh2L3RLAmePFP9rjt+\n"
                            "mWp/+E8BcozeOQ+UBQ5Si6g45Ep80IL80v3Wgpa30TWHFK0HEeyeY2R9gRCZ0oXf\n"\
                            "+PVECmuOFHbMZ1G1sftz+rWyOB1DyPY44ERTbsYivnoAN/kCQQD2afUqTBeU8Mvu\n"\
                            "imlPBu5M4eGORLc6C2rg3UW8PFZOuEUORc3Kb8K5/6+T+6wy1skEQCAOl35F0h3W\n"\
                            "xV3v/TTDAkEAzW1apy8EUBn4jWLFIAR1dRA+PDjiiYJG/cLpaUap3T9VN55goJnd\n"\
                            "TCxeLpGgsca2PSDqK3bYHIXQ7AONdAFkFQJAG3szsXTtCFpWlBLxrbObLg3fBuvY\n"\
                            "92tAjzV+SoD8KylX4kCcs+AE+pNudHWT/dOAda3lJVt15LmLRGGcmWBG2wJARaIm\n"\
                            "03rthFV5Wju7xEGeqwLJhdJmf+QoOkaCpkvssnGQal0GNgpR6Es11aVJilloVso8\n"\
                            "dmU/llOJ4SbHISaDjQJBAOZztjrh1dzSIoUAdLz1RBjPD/8aPom4o8jpDkJZi5O8\n"\
                            "rqVN+OWd4srq1nsCD+kvAZXUU2FMBQydyHxQd6ClxaE=\n"\
                            "-----END RSA PRIVATE KEY-----\n";

static struct option const long_options[] =
{
    {"help", no_argument, 0, 'h'},
    {"sign", required_argument, 0, 's'},
    {NULL, 0, NULL, 0}
};

static RSA* createRSA(unsigned char*key, int publi)
{
    RSA *rsa= NULL;
    BIO*keybio ;
    keybio= BIO_new_mem_buf(key, -1);
    if (keybio==NULL) {
        printf("Failed to create key BIO\n");
        return 0;
    }

    if (publi) {
        rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
    } else {
        rsa= PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
    }
    if (rsa== NULL) {
        printf("Failed to create RSA\n");
    }
    return rsa;
}

static int private_sign(const unsigned char *in_str, unsigned int in_str_len, unsigned char *outret, unsigned int *outlen, unsigned char*key)
{
    RSA* rsa = createRSA(key, 0);  
    int result = RSA_sign(NID_sha1, in_str, in_str_len, outret, outlen, rsa);
    if (result != 1) {
        printf("sign error\n");
        return -1;
    }
    return result;
}

static void store_license(unsigned char *data, int len)
{
    int fd = -1;

    fd = open("license.dat", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 0) {
        printf("open license.dat failed.\n");
        return;
    }
    write(fd, data, len);
    close(fd);
}

static void usage(char *name)
{
    printf("\
Usage: %s [OPTION]... [FILE]\n\
\t-h, --help      help info\n\
\t-s, --sign [FILE]        sign feedback\n", name);
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

static char *read_data(char *path, long *data_len)
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
    *data_len = len;
    return data;
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
    int ret;

    while ((optc = getopt_long (argc, argv, "hs:", long_options, NULL)) != -1) {
        switch (optc) {
        case 'h':
            usage(argv[0]);
            break;
            
        case 's':
            license_file = optarg;
            break;
        }
    }
    realpath(license_file, resolved_path);
    data = read_data(resolved_path, &data_len);
    if (data == NULL) {
        printf("read data failed.\n");
        return 0;
    }
    ret = private_sign(data, data_len, signret, &siglen, (unsigned char*)priv_key);
    free(data);
    if (ret == 1) {
        store_license(signret, SIGNLEN);
        return 1;
    } else {
        return 0;
    }
}