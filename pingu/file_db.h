#define MAX_DIGEST_BIN_BYTES 128/8
#define MAX_DIGEST_HEX_BYTES 128/4

struct filedata
{
        char *fullfilename;
        char *filename;
        unsigned char *md5sum; // Binary
        size_t filesize;
};

const gchar *get_file_directory();
GSList *get_file_database();
gchar *get_md5sum_text(unsigned char *md5sum);
