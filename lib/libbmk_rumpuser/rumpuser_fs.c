#include <bmk-rumpuser/core_types.h>
#include <bmk-rumpuser/rumpuser.h>
#include <bmk-core/errno.h>
#include <bmk-core/types.h>
#include <bmk-core/string.h>
#include <bmk-core/printf.h>

#include <assert.h>



int
rumpuser_getfileinfo(const char *name, uint64_t *size, int *type)
{
  int rv;

  if(bmk_strcmp(name, "paws") == 0)
  {
    // For paws, we don't care about if it is a valid host file or not. We don't need info on it.
    *size = sizeof(char)*524288;
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
