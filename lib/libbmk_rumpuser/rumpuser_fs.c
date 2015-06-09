#include <bmk-rumpuser/core_types.h>
#include <bmk-rumpuser/rumpuser.h>
#include <bmk-core/errno.h>
#include <bmk-core/types.h>
#include <bmk-core/string.h>
#include <bmk-core/printf.h>

#include <assert.h>

char paws[524288];
#define PAWS_SIZE sizeof(char)*524288

int
rumpuser_getfileinfo(const char *name, uint64_t *size, int *type)
{
  int rv;

  if(bmk_strcmp(name, "paws") == 0)
  {
    *size = PAWS_SIZE;
    *type = RUMPUSER_FT_BLK;
    rv = 0;
  } else {
    rv = BMK_ENOSYS;
  }

  return rv;
}

int
rumpuser_open(const char *name, int mode, int *fdp)
{
  int rv;

  if(bmk_strcmp(name, "paws") == 0)
  {
    *fdp = 0;
    rv = 0;
  } else {
    rv = BMK_ENOSYS;
  }

  return rv;
}

int
rumpuser_close(int fd) {
  bmk_memset(&paws[0], 0, sizeof(paws));
  return 0;
}

void
rumpuser_bio(int fd, int op, void *data, size_t dlen, int64_t off,
    rump_biodone_fn biodone, void *donearg)
{
  bmk_printf("%d, ", op & RUMPUSER_BIO_READ);
  bmk_printf("%d\n", op & RUMPUSER_BIO_WRITE);

  size_t rv;
  int error;
  rv = 0; // The amount that is sucessfully read or written
  error = 0;

  assert(donearg != NULL);
  assert(data != NULL);
  assert(off < dlen);

  if(op & RUMPUSER_BIO_READ)
  {
    bmk_printf("operation: reading\n");
    //bmk_printf("offset: %ld\n", (long)off);
    assert(PAWS_SIZE >= dlen);
    char *returnstr = bmk_strcpy(data, &paws[off]);
    assert(returnstr != NULL);
    rv = dlen;
  }
  else if(op & RUMPUSER_BIO_WRITE)
  {
    bmk_printf("operation: writing\n");
    //bmk_printf("offset: %ld\n", (long)off);
    assert(PAWS_SIZE >= dlen);
    char *returnstr = bmk_strcpy(&paws[off], data);
    assert(returnstr != NULL);
    rv = dlen;
  }

  biodone(donearg, rv, error);
}
