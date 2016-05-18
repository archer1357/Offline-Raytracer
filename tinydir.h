/*
Copyright (c) 2013-2014, Cong Xu, Baudouin Feildel
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TINYDIR_H
#define TINYDIR_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning (disable : 4996)
#else
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#endif


/* types */

#define _TINYDIR_PATH_MAX 4096
#ifdef _MSC_VER
/* extra chars for the "\\*" mask */
#define _TINYDIR_PATH_EXTRA 2
#else
#define _TINYDIR_PATH_EXTRA 0
#endif
#define _TINYDIR_FILENAME_MAX 256

// #ifdef _MSC_VER
// #define _TINYDIR_FUNC static __inline
// #else
// #define _TINYDIR_FUNC static __inline__
// #endif
#define _TINYDIR_FUNC

typedef struct
{
	char path[_TINYDIR_PATH_MAX];
	char name[_TINYDIR_FILENAME_MAX];
	char *extension;
	int is_dir;
	int is_reg;

#ifdef _MSC_VER
#else
	struct stat _s;
#endif
} tinydir_file;

typedef struct
{
	char path[_TINYDIR_PATH_MAX];
	int has_next;
	size_t n_files;

	tinydir_file *_files;
#ifdef _MSC_VER
	HANDLE _h;
	WIN32_FIND_DATA _f;
#else
	DIR *_d;
	struct dirent *_e;
#endif
} tinydir_dir;


/* declarations */

_TINYDIR_FUNC
int tinydir_open(tinydir_dir *dir, const char *path);
_TINYDIR_FUNC
int tinydir_open_sorted(tinydir_dir *dir, const char *path);
_TINYDIR_FUNC
void tinydir_close(tinydir_dir *dir);

_TINYDIR_FUNC
int tinydir_next(tinydir_dir *dir);
_TINYDIR_FUNC
int tinydir_readfile(const tinydir_dir *dir, tinydir_file *file);
_TINYDIR_FUNC
int tinydir_readfile_n(const tinydir_dir *dir, tinydir_file *file, size_t i);
_TINYDIR_FUNC
int tinydir_open_subdir_n(tinydir_dir *dir, size_t i);

_TINYDIR_FUNC
void _tinydir_get_ext(tinydir_file *file);
_TINYDIR_FUNC
int _tinydir_file_cmp(const void *a, const void *b);


#endif
