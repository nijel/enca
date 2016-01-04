/*
  buffers, input/output operations with magic buffering and small utils

  Copyright (C) 2000-2002 David Necas (Yeti) <yeti@physics.muni.cz>

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as published
  by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/
#include "common.h"

/* XXX: We can't go on w/o this, defining struct stat manually is
 * braindamaged. */
#include <sys/types.h>
#include <sys/stat.h>

#if HAVE_FCNTL_H
# include <fcntl.h>
#else /* HAVE_FCNTL_H */
int open(const char *pathname, int flags, mode_t mode);
#endif /* HAVE_FCNTL_H */

/* MS simply cannot use the same function names as everyone else... */
#ifndef HAVE_FTRUNCATE
#  ifdef WIN32
#     include <io.h>
#     define ftruncate _chsize
#  else
     /* XXX: We are in trouble. */
#  endif
#endif

/* stdin and stdout names */
static const char *stdin_fname = "STDIN";
static const char *stdout_fname = "STDOUT";

/* Local prototypes. */
static off_t file_size(File *file);
static int file_fileno(File *file);
static ssize_t file_read_limited(File *file,
                                 ssize_t limit);

/* I/O buffer size. */
size_t buffer_size = 0x10000;

/*
   Implementation notes:

   All the file_* functions below contain a considerable amount of magic.
   This means you don't have to care about many things when using them;
   unfortunately, some other things you would intuitively expect to work,
   don't work and can break them totally.  So be careful and RTFS in case
   of doubts.
*/

/* create and return a new byte buffer of size size
   when size is zero use size provided by the system as `good' */
Buffer*
buffer_new(size_t size)
{
  Buffer *buf;

  buf = NEW(Buffer, 1);
  buf->size = (size == 0 ? BUFSIZ : size);
  buf->data = (byte*)enca_malloc(buf->size);
  buf->pos = 0;

  return buf;
}

/* destroy a buffer object buf
   can be safely called repeatedly */
void
buffer_free(Buffer *buf)
{
  if (buf == NULL)
    return;

  enca_free(buf->data);
  enca_free(buf);
}

/* Find abbreviated name in atable and return pointer to corresponding data.
 * When name is NULL, print list of all abbreviations, instead (and
 * return NULL too).
 *
 * Returns NULL when no unique abbreviation is found (or when name
 * is NULL too), object_name is then used as object name for diagnostics. */
const Abbreviation*
expand_abbreviation(const char *name,
                    const Abbreviation *atable,
                    size_t size,
                    const char *object_name)
{
  size_t n, i, pos, acount;

  /* Print list for NULL name. */
  if (name == NULL) {
    for (i = 0; i < size; i++)
      puts(atable[i].name);

    return NULL;
  }

  /* Find all matching abbreviations. */
  n = strlen(name);
  pos = 0;
  acount = 0;
  for (i = 0; i < size; i++) {
    if (strncmp(name, atable[i].name, n) == 0) {
      acount++;
      pos = i;
    }
  }

  /* No match? */
  if (acount == 0) {
    fprintf(stderr, "%s: `%s' doesn't look like a valid %s name\n",
                    program_name,
                    name,
                    object_name);
    return NULL;
  }

  /* More than one match? */
  if (acount > 1) {
    fprintf(stderr, "%s: Abbreviation `%s' is ambiguous, matches:\n",
                    program_name,
                    name);
    for (i = 0; i < size; i++) {
      if (strncmp(name, atable[i].name, n) == 0)
        fprintf(stderr, "  %s\n", atable[i].name);
    }
    return NULL;
  }

  /* OK, exactly one. */
  return atable + pos;
}


/* filename wrapper, return STDIN_FNAME on NULL */
const char*
ffname_r(const char *fname)
{
  return (fname == NULL) ? stdin_fname : fname;
}

/* filename wrapper, return STDOUT_FNAME on NULL */
const char*
ffname_w(const char *fname)
{
  return (fname == NULL) ? stdout_fname : fname;
}

/* file_read_limited() wrapper, limit is buffer size */
ssize_t
file_read(File *file)
{
  return file_read_limited(file, file->buffer->size);
}

/* read from file file storing data to the associated buffer
   limit is the desired total number of bytes that should be in the buffer
   (not maximum bytes to read)
   returns the number of bytes in file's buffer after read, -1 on failure
   is up to caller to provide a limit value not greater than buffer size */
static ssize_t
file_read_limited(File *file, ssize_t limit)
{
  FILE *stream;

  assert(limit >= 0);
  assert(file != NULL);
  assert(file->buffer != NULL);

  if (limit < file->buffer->pos)
    return file->buffer->pos;

  assert((size_t)limit <= file->buffer->size);

  if (file->name == NULL)
    stream = stdin;
  else
    stream = file->stream;

  file->buffer->pos += fread(file->buffer->data + file->buffer->pos,
                             1,
                             limit - file->buffer->pos,
                             stream);

  /* read failed? */
  if (file->buffer->pos < limit && ferror(stream)) {
    fprintf(stderr, "%s: Cannot read file `%s': %s\n",
                    program_name,
                    ffname_r(file->name),
                    strerror(errno));
    file_close(file);
    file->buffer->pos = -1;
  }

  return file->buffer->pos;
}

/* file_read_limited() wrapper, is like fgets(), but generates MUCH less
 * system calls (usually exactly two per call) than GNU libc's fgets() due to
 * buffering. Anyway, getline() is not available on non-GNU.

 * XXX: must NOT be used on non-seekable streams!
 */
byte*
file_getline(File *file)
{
  static const size_t READ_AHEAD = 256; /* 256 is the minimum guaranteed value
                                           of BUFSIZE and also a good maximum
                                           expected line length */

  ssize_t err;
  size_t want_bytes,old;
  Buffer *buf; /* file->buffer alias */
  byte *p = NULL;

  assert(file != NULL);
  assert(file->buffer != NULL);

  /* overwrite what could remain in the buffer */
  buf = file->buffer;
  buf->pos = want_bytes = old = 0;

  /* read chunks of approximately one line size until we hit EOF or find an
     EOL */
  do {
    want_bytes += READ_AHEAD;
    if (want_bytes >= buf->size)
      want_bytes = buf->size - 1;

    err = file_read_limited(file, want_bytes);
    if (err == -1 || buf->pos == 0)
      return NULL;

    p = memchr(buf->data + old, '\n', buf->pos - old);
    if (p != NULL || (size_t)(buf->pos) < want_bytes)
      break;

    old = buf->pos;
  } while (1);

  /* something found? */
  if (p != NULL) {
    *(p+1) = '\0';
    file_seek(file, (off_t)(p - buf->data - buf->pos) + 1, SEEK_CUR);
  }
  /* or just EOF? */
  else
    file->buffer->data[file->buffer->pos + 1] = '\0';

  return file->buffer->data;
}

/* write to file @file the associated buffer and reset buffer position
   returns nonzero if it was successfully written, -1 on failure */
ssize_t
file_write(File *file)
{
  ssize_t bw;
  FILE *stream;

  assert(file != NULL);
  assert(file->buffer != NULL);

  if (file->buffer->pos == 0)
    return 0;

  if (file->name == NULL)
    stream = stdout;
  else
    stream = file->stream;

  bw = fwrite(file->buffer->data, 1, file->buffer->pos, stream);
  /* write failed? */
  if (bw < file->buffer->pos) {
    fprintf(stderr, "%s: Cannot write to file `%s': %s\n",
                    program_name,
                    ffname_w(file->name),
                    strerror(errno));
    file_close(file);
    return -1;
  }
  file->buffer->pos = 0;

  return bw;
}

/* create temporary file, open it readwrite and return the created File
   retuns NULL when we are unable to create a temporary file
   when ulink is true, delete it right after opening
   (XXX: this requires POSIX-conformant system)

   FIXME: we always create temporary files in /tmp */
File*
file_temporary(Buffer *buffer, int ulink)
{
  /* `template' for temporary file creation */
  char *temp_filename;

  File *file = NULL;
  int fd;

  temp_filename = strdup("/tmp/" PACKAGE_TARNAME "XXXXXX");

  if ((fd = mkstemp(temp_filename)) < 0) {
    /* trieed heavy without success? that's bad */
    fprintf(stderr, "%s: Unable to create a temporary file\n"
                   "do you have write permissions in /tmp?\n",
                    program_name);

    free(temp_filename);
    return NULL;
  }

  file = file_new(temp_filename, buffer);

  free(temp_filename);

  /* fdopen it to get the stream */
  if ((file->stream = fdopen(fd, "w+b")) == NULL) {
    fprintf(stderr, "%s: Cannot get stream for an open filedescriptor %d: %s\n",
                    program_name,
                    fd,
                    strerror(errno));
    exit(EXIT_TROUBLE);
  }
  /* here, we have a unique temporary file opened readwrite */
  if (ulink)
    file_unlink(file->name);
  return file;
}

/* reposition file file
   see lseek(2) for description
   returns current position from begining of file, -1 on failure */
off_t
file_seek(File *file, off_t offset, int whence)
{
  off_t i;

  assert(file != NULL);

  i = fseek(file->stream, offset, whence);
  if (i == -1) {
    fprintf(stderr, "%s: Cannot seek in file `%s': %s\n",
                    program_name,
                    file->name,
                    strerror(errno));
    file_close(file);
  }

  return i;
}

/* truncate file file
   see truncate(2) for description
   returns zero on success, -1 on failure */
int
file_truncate(File *file, off_t length)
{
  int fd;

  assert(file != NULL);
  assert(file->name != NULL);  /* we should not be called on stdin/stdout */

  fd = file_fileno(file);
  if (options.verbosity_level > 8)
    fprintf(stderr, "Truncating `%s' to %ld\n", file->name, (long int)length);
  if (ftruncate(fd, length) == 0)
    return 0;

  fprintf(stderr, "%s: Cannot truncate file `%s' to %ld: %s\n",
                  program_name,
                  file->name,
                  (long int)length,
                  strerror(errno));
  file_close(file);
  return -1;
}

/* unlink file fname
   see unlink(2) for description
   returns 0 on success, nonzero on failure */
int
file_unlink(const char *fname)
{
  int err;

  assert(fname != NULL);

  if (options.verbosity_level > 8)
    fprintf(stderr, "Unlinking `%s'\n", fname);
  err = unlink(fname);
  if (err != 0) {
    fprintf(stderr, "%s: Cannot unlink file `%s': %s\n",
                    program_name,
                    fname,
                    strerror(errno));
  }

  return err;
}

/* open a file file in mode mode
   see fopen(3) for description
   however, we disable buffering by default
   and opening file whose name is NULL assings the `magic' stdin/stdout
   returns 0 on success, nonzero otherwise */
int
file_open(File *file, const char *mode)
{
  assert(file != NULL);
  assert(file->buffer != NULL);
  assert(mode != NULL);
  assert(*mode);

  if (mode[0] == 'r')
    file->buffer->pos = 0;

  /* fake stdin/stdout opening */
  if (file->name == NULL) {
    if (options.verbosity_level > 8)
      fprintf(stderr, "Fake-opening stdin/stdout in mode %s\n", mode);
    if (mode[0] == 'r' || mode[0] =='w') {
      file->stream = NULL;
      file->size = -1;
      return 0;
    }
    fprintf(stderr, "%s: Cannot open stdin/stdout in mode %s\n",
                    program_name,
                    mode);
    return 1;
  }

  /* open a regular file */
  if (options.verbosity_level > 8)
    fprintf(stderr, "Opening file `%s' in mode %s\n", file->name, mode);
  file->stream = fopen(file->name, mode);
  if (file->stream == NULL) {
    fprintf(stderr, "%s: Cannot open file `%s' in mode %s: %s\n",
                    program_name,
                    file->name,
                    mode,
                    strerror(errno));
    return 1;
  }

  /* get length */
  if (mode[0] == 'r') {
    file->size = file_size(file);
    if (options.verbosity_level > 8)
      fprintf(stderr, "File `%s' size is %ld\n",
                      file->name, (long int)file->size);
    if (file->size == -1)
      return 1;
  }
  else file->size = -1;

  return 0;
}

/* close file file
   see fclose(3) for description
   but this one can be safely called repeatedly on the same stream or on
   a not-yet-opened stream */
int
file_close(File *file)
{
  assert(file != NULL);

  /* it's OK to close files any times we want and to close files that have
     never been opened
     also fake `magic' stdin/stdout closing */
  if (file->name == NULL) {
    if (options.verbosity_level > 8)
      fprintf(stderr, "Fake-closing stdin/stdout\n");
    return 0;
  }

  if (file->stream == NULL) {
    if (options.verbosity_level > 8)
      fprintf(stderr, "Closing an already closed file (noop)\n");
    return 0;
  }

  /* close a regular file */
  if (options.verbosity_level > 8)
    fprintf(stderr, "Closing file `%s'\n", file->name);
  if (fclose(file->stream) == EOF) {
    fprintf(stderr, "%s: Cannot close file `%s': %s\n",
                    program_name,
                    file->name,
                    strerror(errno));
    exit(EXIT_TROUBLE);
  }
  file->stream = NULL;

  return 0;
}

/* return the size of an open stream (by stat()-ing it)
   returns -1 on failure */
static off_t
file_size(File *file)
{
  int fd;
  struct stat st;

  assert(file != NULL);

  if (file->name == NULL)
    return -1;

  fd = file_fileno(file);
  if (options.verbosity_level > 8)
    fprintf(stderr, "stat()-ing `%s' (fd %d) for its size\n", file->name, fd);
  if (fstat(fd, &st) != 0) {
    fprintf(stderr, "%s: Cannot stat file `%s': %s\n",
                    program_name,
                    ffname_r(file->name),
                    strerror(errno));
    return -1;
  }

  return st.st_size;
}

/* return a filedescriptor given a stream, see fileno(3) for description
 * never fails, on failure aborts program. */
static int
file_fileno(File *file)
{
  int fd;

  fd = fileno(file->stream);
  if (fd != -1)
    return fd;

  fprintf(stderr, "%s: Cannot get filedescriptor for an open stream `%s': %s\n",
                  program_name,
                  ffname_r(file->name),
                  strerror(errno));
  exit(EXIT_TROUBLE);

  return -1;
}

/* create and return a file object whose filename is fname
   and which uses buffer buffer for i/o operations */
File*
file_new(const char *fname, Buffer *buffer) {
  File *file;

  file = NEW(File, 1);
  file->name = enca_strdup(fname);
  file->stream = NULL;
  file->size = -1;
  file->buffer = buffer;

  return file;
}

/* destroy a file object file, possibly closing it when opened
   does NOT destroy the associated buffer
   can be safely called repeatedly */
void
file_free(File *file)
{
  if (file == NULL)
    return;

  if (file->stream != NULL)
    file_close(file);
  enca_free(file->name);
  enca_free(file);
}

/* vim: ts=2
 */
