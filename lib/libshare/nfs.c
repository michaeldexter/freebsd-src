/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "nfs.h"


static int nfs_lock_fd = -1;


/*
 * nfs_exports_[lock|unlock] are used to guard against conconcurrent
 * updates to the exports file. Each protocol is responsible for
 * providing the necessary locking to ensure consistency.
 */
__attribute__((visibility("hidden"))) int
nfs_exports_lock(const char *name)
{
	int err;

	nfs_lock_fd = open(name, O_RDWR | O_CREAT | O_CLOEXEC, 0600);
	if (nfs_lock_fd == -1) {
		err = errno;
		fprintf(stderr, "failed to lock %s: %s\n", name, strerror(err));
		return (err);
	}

	if (flock(nfs_lock_fd, LOCK_EX) != 0) {
		err = errno;
		fprintf(stderr, "failed to lock %s: %s\n", name, strerror(err));
		(void) close(nfs_lock_fd);
		nfs_lock_fd = -1;
		return (err);
	}

	return (0);
}

__attribute__((visibility("hidden"))) void
nfs_exports_unlock(const char *name)
{
	verify(nfs_lock_fd > 0);

	if (flock(nfs_lock_fd, LOCK_UN) != 0) {
		fprintf(stderr, "failed to unlock %s: %s\n",
		    name, strerror(errno));
	}

	(void) close(nfs_lock_fd);
	nfs_lock_fd = -1;
}
