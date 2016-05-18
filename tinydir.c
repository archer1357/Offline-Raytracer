

#include "tinydir.h"

/* definitions*/

_TINYDIR_FUNC
int tinydir_open(tinydir_dir *dir, const char *path)
{
	if (dir == NULL || path == NULL || strlen(path) == 0)
	{
		errno = EINVAL;
		return -1;
	}
	if (strlen(path) + _TINYDIR_PATH_EXTRA >= _TINYDIR_PATH_MAX)
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	/* initialise dir */
	dir->_files = NULL;
#ifdef _MSC_VER
	dir->_h = INVALID_HANDLE_VALUE;
#else
	dir->_d = NULL;
#endif
	tinydir_close(dir);

	strcpy(dir->path, path);
#ifdef _MSC_VER
	strcat(dir->path, "\\*");

  /*modified start*/
#ifdef UNICODE
  wchar_t path2[512];
  MultiByteToWideChar(CP_ACP,0,dir->path,-1,path2,512);
  dir->_h = FindFirstFile(path2, &dir->_f);
#else
	dir->_h = FindFirstFile(dir->path, &dir->_f); //same
#endif
  /*modified end*/
	dir->path[strlen(dir->path) - 2] = '\0';
	if (dir->_h == INVALID_HANDLE_VALUE)
#else
	dir->_d = opendir(path);
	if (dir->_d == NULL)
#endif
	{
		errno = ENOENT;
		goto bail;
	}

	/* read first file */
	dir->has_next = 1;
#ifndef _MSC_VER
	dir->_e = readdir(dir->_d);
	if (dir->_e == NULL)
	{
		dir->has_next = 0;
	}
#endif

	return 0;

bail:
	tinydir_close(dir);
	return -1;
}

_TINYDIR_FUNC
int tinydir_open_sorted(tinydir_dir *dir, const char *path)
{
	/* Count the number of files first, to pre-allocate the files array */
	size_t n_files = 0;
	if (tinydir_open(dir, path) == -1)
	{
		return -1;
	}
	while (dir->has_next)
	{
		n_files++;
		if (tinydir_next(dir) == -1)
		{
			goto bail;
		}
	}
	tinydir_close(dir);

	if (tinydir_open(dir, path) == -1)
	{
		return -1;
	}

	dir->n_files = 0;
	dir->_files = (tinydir_file *)malloc(sizeof *dir->_files * n_files);
	if (dir->_files == NULL)
	{
		errno = ENOMEM;
		goto bail;
	}
	while (dir->has_next)
	{
		tinydir_file *p_file;
		dir->n_files++;

		p_file = &dir->_files[dir->n_files - 1];
		if (tinydir_readfile(dir, p_file) == -1)
		{
			goto bail;
		}

		if (tinydir_next(dir) == -1)
		{
			goto bail;
		}

		/* Just in case the number of files has changed between the first and
		second reads, terminate without writing into unallocated memory */
		if (dir->n_files == n_files)
		{
			break;
		}
	}

	qsort(dir->_files, dir->n_files, sizeof(tinydir_file), _tinydir_file_cmp);

	return 0;

bail:
	tinydir_close(dir);
	return -1;
}

_TINYDIR_FUNC
void tinydir_close(tinydir_dir *dir)
{
	if (dir == NULL)
	{
		return;
	}

	memset(dir->path, 0, sizeof(dir->path));
	dir->has_next = 0;
	dir->n_files = 0;
	if (dir->_files != NULL)
	{
		free(dir->_files);
	}
	dir->_files = NULL;
#ifdef _MSC_VER
	if (dir->_h != INVALID_HANDLE_VALUE)
	{
		FindClose(dir->_h);
	}
	dir->_h = INVALID_HANDLE_VALUE;
#else
	if (dir->_d)
	{
		closedir(dir->_d);
	}
	dir->_d = NULL;
	dir->_e = NULL;
#endif
}

_TINYDIR_FUNC
int tinydir_next(tinydir_dir *dir)
{
	if (dir == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	if (!dir->has_next)
	{
		errno = ENOENT;
		return -1;
	}

#ifdef _MSC_VER
	if (FindNextFile(dir->_h, &dir->_f) == 0)
#else
	dir->_e = readdir(dir->_d);
	if (dir->_e == NULL)
#endif
	{
		dir->has_next = 0;
#ifdef _MSC_VER
		if (GetLastError() != ERROR_SUCCESS &&
			GetLastError() != ERROR_NO_MORE_FILES)
		{
			tinydir_close(dir);
			errno = EIO;
			return -1;
		}
#endif
	}

	return 0;
}

_TINYDIR_FUNC
int tinydir_readfile(const tinydir_dir *dir, tinydir_file *file)
{
	if (dir == NULL || file == NULL)
	{
		errno = EINVAL;
		return -1;
	}
#ifdef _MSC_VER
	if (dir->_h == INVALID_HANDLE_VALUE)
#else
	if (dir->_e == NULL)
#endif
	{
		errno = ENOENT;
		return -1;
	}

  /*modified start*/
#ifdef _MSC_VER
#ifdef UNICODE
  char cFileName2[512];

  WideCharToMultiByte(CP_ACP,0,dir->_f.cFileName,-1,
    cFileName2,512,NULL,NULL);
  //WideCharToMultiByte(CP_ACP,WC_DEFAULTCHAR,dir->_f.cFileName,-1,
  //  cFileName2,512,NULL,NULL);
#endif
#endif
  /*modified end*/

	if (strlen(dir->path) +
		strlen(
#ifdef _MSC_VER
    /*modified start*/
#ifdef UNICODE
    cFileName2
#else
    dir->_f.cFileName //same
#endif
    /*modified end*/
#else
			dir->_e->d_name
#endif
		) + 1 + _TINYDIR_PATH_EXTRA >=
		_TINYDIR_PATH_MAX)
	{
		/* the path for the file will be too long */
		errno = ENAMETOOLONG;
		return -1;
	}
	if (strlen(
#ifdef _MSC_VER
    /*modified start*/
#ifdef UNICODE
    cFileName2
#else
    dir->_f.cFileName //same
#endif
    /*modified end*/
#else
			dir->_e->d_name
#endif
		) >= _TINYDIR_FILENAME_MAX)
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	strcpy(file->path, dir->path);
	strcat(file->path, "/");
	strcpy(file->name,
#ifdef _MSC_VER
    /*modified start*/
#ifdef UNICODE
    cFileName2
#else
    dir->_f.cFileName //same
#endif
    /*modified end*/
#else
		dir->_e->d_name
#endif
	);
	strcat(file->path, file->name);
#ifndef _MSC_VER
	if (stat(file->path, &file->_s) == -1)
	{
		return -1;
	}
#endif
	_tinydir_get_ext(file);

	file->is_dir =
#ifdef _MSC_VER
		!!(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
#else
		S_ISDIR(file->_s.st_mode);
#endif
	file->is_reg =
#ifdef _MSC_VER
		!!(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) ||
		(
			!(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) &&
			!(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			!(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) &&
#ifdef FILE_ATTRIBUTE_INTEGRITY_STREAM
			!(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_INTEGRITY_STREAM) &&
#endif
#ifdef FILE_ATTRIBUTE_NO_SCRUB_DATA
			!(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_NO_SCRUB_DATA) &&
#endif
			!(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) &&
			!(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY));
#else
		S_ISREG(file->_s.st_mode);
#endif

	return 0;
}

_TINYDIR_FUNC
int tinydir_readfile_n(const tinydir_dir *dir, tinydir_file *file, size_t i)
{
	if (dir == NULL || file == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	if (i >= dir->n_files)
	{
		errno = ENOENT;
		return -1;
	}

	memcpy(file, &dir->_files[i], sizeof(tinydir_file));
	_tinydir_get_ext(file);

	return 0;
}

_TINYDIR_FUNC
int tinydir_open_subdir_n(tinydir_dir *dir, size_t i)
{
	char path[_TINYDIR_PATH_MAX];
	if (dir == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	if (i >= dir->n_files || !dir->_files[i].is_dir)
	{
		errno = ENOENT;
		return -1;
	}

	strcpy(path, dir->_files[i].path);
	tinydir_close(dir);
	if (tinydir_open_sorted(dir, path) == -1)
	{
		return -1;
	}

	return 0;
}

/* Open a single file given its path */
_TINYDIR_FUNC
int tinydir_file_open(tinydir_file *file, const char *path)
{
	tinydir_dir dir;
	int result = 0;
	int found = 0;
	char dir_name_buf[_TINYDIR_PATH_MAX];
	char file_name_buf[_TINYDIR_FILENAME_MAX];
	char *dir_name;
	char *base_name;
#ifdef _MSC_VER
	char drive_buf[_TINYDIR_PATH_MAX];
	char ext_buf[_TINYDIR_FILENAME_MAX];
#endif

	if (file == NULL || path == NULL || strlen(path) == 0)
	{
		errno = EINVAL;
		return -1;
	}
	if (strlen(path) + _TINYDIR_PATH_EXTRA >= _TINYDIR_PATH_MAX)
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	/* Get the parent path */
#ifdef _MSC_VER
	if (_splitpath_s(
			path,
			drive_buf, sizeof drive_buf,
			dir_name_buf, sizeof dir_name_buf,
			file_name_buf, sizeof file_name_buf,
			ext_buf, sizeof ext_buf))
	{
		errno = EINVAL;
		return -1;
	}
	/* Concatenate the drive letter and dir name to form full dir name */
	strcat(drive_buf, dir_name_buf);
	dir_name = drive_buf;
	/* Concatenate the file name and extension to form base name */
	strcat(file_name_buf, ext_buf);
	base_name = file_name_buf;
#else
	strcpy(dir_name_buf, path);
	dir_name = dirname(dir_name_buf);
	strcpy(file_name_buf, path);
	base_name = basename(file_name_buf);
#endif

	/* Open the parent directory */
	if (tinydir_open(&dir, dir_name) == -1)
	{
		return -1;
	}

	/* Read through the parent directory and look for the file */
	while (dir.has_next)
	{
		if (tinydir_readfile(&dir, file) == -1)
		{
			result = -1;
			goto bail;
		}
		if (strcmp(file->name, base_name) == 0)
		{
			/* File found */
			found = 1;
			goto bail;
		}
		tinydir_next(&dir);
	}
	if (!found)
	{
		result = -1;
		errno = ENOENT;
	}

bail:
	tinydir_close(&dir);
	return result;
}

_TINYDIR_FUNC
void _tinydir_get_ext(tinydir_file *file)
{
	char *period = strrchr(file->name, '.');
	if (period == NULL)
	{
		file->extension = &(file->name[strlen(file->name)]);
	}
	else
	{
		file->extension = period + 1;
	}
}

_TINYDIR_FUNC
int _tinydir_file_cmp(const void *a, const void *b)
{
	const tinydir_file *fa = (const tinydir_file *)a;
	const tinydir_file *fb = (const tinydir_file *)b;
	if (fa->is_dir != fb->is_dir)
	{
		return -(fa->is_dir - fb->is_dir);
	}
	return strncmp(fa->name, fb->name, _TINYDIR_FILENAME_MAX);
}
