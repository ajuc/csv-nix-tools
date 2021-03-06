/*
 * Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>

#include "usr-grp.h"
#include "utils.h"

struct csv_user *users;
size_t nusers;

struct csv_group *groups;
size_t ngroups;

void
load_users(void)
{
	while (1) {
		errno = 0;

		struct passwd *p = getpwent();
		if (!p) {
			if (!errno)
				return;

			perror("getpwent");
			exit(2);
		}

		users = xrealloc_nofail(users, nusers + 1, sizeof(*users));

		struct csv_user *u = &users[nusers];

		u->name = xstrdup_nofail(p->pw_name);
		u->passwd = xstrdup_nofail(p->pw_passwd);
		u->uid = p->pw_uid;
		u->gid = p->pw_gid;
		u->gecos = xstrdup_nofail(p->pw_gecos);
		u->dir = xstrdup_nofail(p->pw_dir);
		u->shell = xstrdup_nofail(p->pw_shell);
		u->groups = NULL;
		u->ngroups = 0;

		nusers++;
	}

	endpwent();
}

void
free_users(void)
{
	for (size_t i = 0; i < nusers; ++i) {
		struct csv_user *u = &users[i];
		free(u->name);
		free(u->passwd);
		free(u->gecos);
		free(u->dir);
		free(u->shell);
		free(u->groups);
	}

	free(users);
	users = NULL;
}

void
load_groups(void)
{
	while (1) {
		errno = 0;

		struct group *g = getgrent();
		if (!g) {
			if (!errno)
				return;

			perror("getgrent");
			exit(2);
		}

		groups = xrealloc_nofail(groups, ngroups + 1, sizeof(*groups));

		struct csv_group *cg = &groups[ngroups];

		cg->name = xstrdup_nofail(g->gr_name);
		cg->passwd = xstrdup_nofail(g->gr_passwd);
		cg->gid = g->gr_gid;

		size_t nmembers = 0;
		while (g->gr_mem[nmembers])
			nmembers++;

		cg->members = xmalloc_nofail(nmembers, sizeof(cg->members[0]));
		for (size_t i = 0; i < nmembers; ++i)
			cg->members[i] = xstrdup_nofail(g->gr_mem[i]);
		cg->nmembers = nmembers;
		cg->users = NULL;

		ngroups++;
	}

	endgrent();
}

void
free_groups(void)
{
	for (size_t i = 0; i < ngroups; ++i) {
		struct csv_group *g = &groups[i];
		free(g->name);
		free(g->passwd);
		for (size_t j = 0; j < g->nmembers; ++j)
			free(g->members[j]);
		free(g->members);
		free(g->users);
	}

	free(groups);
	groups = NULL;
}

struct csv_user *
find_user_by_name(const char *name)
{
	for (size_t i = 0; i < nusers; ++i)
		if (strcmp(users[i].name, name) == 0)
			return &users[i];
	return NULL;
}

struct csv_group *
find_group_by_gid(gid_t gid)
{
	for (size_t i = 0; i < ngroups; ++i)
		if (groups[i].gid == gid)
			return &groups[i];
	return NULL;
}
