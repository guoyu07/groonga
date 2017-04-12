/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2017 Brazil

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 2.1 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "grn_file_lock.h"
#include "grn_ctx.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

void
grn_file_lock_init(grn_ctx *ctx,
                   grn_file_lock *file_lock,
                   const char *path)
{
  file_lock->path = path;
  file_lock->fd = -1;
}

grn_bool
grn_file_lock_acquire(grn_ctx *ctx,
                      grn_file_lock *file_lock,
                      int timeout,
                      const char *error_message_tag)
{
  int i;
  int n_lock_tries = timeout;

  if (!file_lock->path) {
    return GRN_TRUE;
  }

  for (i = 0; i < n_lock_tries; i++) {
    file_lock->fd = open(file_lock->path, O_CREAT | O_EXCL);
    if (file_lock->fd != -1) {
      break;
    }
    grn_nanosleep(GRN_LOCK_WAIT_TIME_NANOSECOND);
  }

  if (file_lock->fd == -1) {
    ERR(GRN_NO_LOCKS_AVAILABLE,
        "%s failed to acquire lock: <%s>",
        error_message_tag, file_lock->path);
    return GRN_FALSE;
  } else {
    return GRN_TRUE;
  }
}

void
grn_file_lock_release(grn_ctx *ctx, grn_file_lock *file_lock)
{
  if (file_lock->fd == -1) {
    return;
  }

  close(file_lock->fd);
  unlink(file_lock->path);
}

void
grn_file_lock_fin(grn_ctx *ctx, grn_file_lock *file_lock)
{
  if (file_lock->fd != -1) {
    grn_file_lock_release(ctx, file_lock);
  }
}