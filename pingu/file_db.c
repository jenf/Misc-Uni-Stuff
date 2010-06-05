#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include "file_db.h"
#define BUFFER 1024

#define OSET_FILESIZE 0
#define OSET_MD5SUM 4
#define OSET_FILENAME MAX_DIGEST_BIN_BYTES+4
static time_t lastupdate = 0;
static GSList *filenametomd5 = NULL;

static void recurse_md5(const char *dirname);
static void create_filenametomd5(void);
static gchar *get_md5sum_file(char *filename);

const gchar *directory;

void set_file_directory(const gchar * dir)
{
	directory = dir;
}

const gchar *get_file_directory()
{
	return directory;
}

static void free_md5(struct filedata *f)
{
	g_free(f->fullfilename);
	g_free(f->filename);
	g_free(f->md5sum);
	g_free(f);
}

static void dump_table_single(struct filedata *f)
{
	gchar  *md5;
	md5 = get_md5sum_text(f->md5sum);
	printf("key:%s value:%s\n", f->fullfilename, md5);
	g_free(md5);
}

static GThreadPool *build_md5;
void md5_init(char *dir)
{
	// Set the dir and create the threads
	set_file_directory(dir);
	build_md5 =
		g_thread_pool_new((GFunc) create_filenametomd5, NULL, 1, FALSE, NULL);
	g_thread_pool_push(build_md5, "foo", NULL);

}

G_LOCK_DEFINE(create_filenametomd5);

GSList *get_file_database()
{
	G_LOCK(create_filenametomd5);
	G_UNLOCK(create_filenametomd5);	// Wait for the list to be built.
	return filenametomd5;
}

static void create_filenametomd5(void)
{
	// Note this function is wrong, it really has to scan the mtime of all the
	// directories and files, I haven't got time to fix it.
	struct stat buf;
	G_LOCK(create_filenametomd5);
	stat(get_file_directory(), &buf);
	if (buf.st_mtime != lastupdate || lastupdate == 0)
	{
		printf("Checking Directory updates\n");
		// This is broken.
		add_status_message("Begin MD5 Computations");
		GSList *bar;
		bar = filenametomd5;
		while (bar)
		{
			free_md5(bar->data);
			bar = bar->next;
		}
		// free up the list
		g_slist_free(filenametomd5);
		// set it to null
		filenametomd5 = NULL;
		// rescan the directory
		recurse_md5(get_file_directory());
		lastupdate = buf.st_mtime;
		add_status_message("End MD5 Computations");
	}
	G_UNLOCK(create_filenametomd5);
	sleep(1);
	request_do_timeout();
	g_thread_pool_push(build_md5, "foo", NULL);
}


static void recurse_md5(const char *dirname)
{
	struct filedata *f;
	DIR    *dir;
	GString *bar;
	struct dirent *st;
	struct stat buf;
	int     len;
	printf("%s\n", dirname);
	bar = g_string_new(dirname);
	g_string_append(bar, "/");
	len = strlen(bar->str);
	if (!(dir = opendir(dirname)))
	{
		perror("Couldn't open directory");
		return;
	}
	// Look through the directory
	while (st = readdir(dir))
	{
		if (*st->d_name != '.')
		{
			gchar  *foo;
			g_string_append(bar, st->d_name);
			stat(bar->str, &buf);
			if (S_ISDIR(buf.st_mode))
			{
				// Recurse through children
				recurse_md5(bar->str);
			}
			else
			{
				if (S_ISREG(buf.st_mode))
				{
					// Add it to the list.
					char   *filename;
					f = g_malloc(sizeof(struct filedata));
					foo = get_md5sum_file(bar->str);
					filename = g_strdup(st->d_name);
					f->fullfilename = g_strdup(bar->str);
					f->filename = g_strdup(st->d_name);
					f->md5sum = foo;
					f->filesize = buf.st_size;
					filenametomd5 = g_slist_append(filenametomd5, f);
				}
			}
			g_string_truncate(bar, len);
		}
	}
	g_string_free(bar, TRUE);
}

// Converts a bin md5sum to a hex md5sum,allocates memory
gchar  *get_md5sum_text(unsigned char *md5sum)
{
	GString *woo;
	int     i;
	gchar  *Ret;
	woo = g_string_new("");
	for (i = 0; i < (MAX_DIGEST_HEX_BYTES / 2); ++i)
	{
		g_string_append_printf(woo, "%02x", md5sum[i]);
	}
	Ret = woo->str;
	g_string_free(woo, FALSE);
	return Ret;
}

// Allocated new memory for the get_md5sum
static gchar *get_md5sum_file(char *filename)
{
	FILE   *fp;
	int     i;
	unsigned char *bin_buffer;

	bin_buffer = malloc(MAX_DIGEST_BIN_BYTES);

	if ((fp = fopen(filename, "r")) == NULL)
	{
		perror("Cannot open file");
		exit(-1);
	}
	bzero(bin_buffer, MAX_DIGEST_BIN_BYTES);
	if (md5_stream(fp, bin_buffer))
	{
		printf("failed\n");
		exit(-1);
	}
	fclose(fp);

	// Taken from md5sum.c in gnu coreutils
	return bin_buffer;
}
