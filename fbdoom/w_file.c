//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	WAD I/O functions.
//

#include "stdio.h"

#include "config.h"

#include "doomtype.h"
#include "m_argv.h"

#include "w_file.h"
#include "stdlib.h"
extern wad_file_class_t stdc_wad_file;

#ifdef _WIN32
extern wad_file_class_t win32_wad_file;
#endif

#ifdef HAVE_MMAP
extern wad_file_class_t posix_wad_file;
#endif 

static wad_file_class_t *wad_file_classes[] = 
{
#ifdef _WIN32
    &win32_wad_file,
#endif
#ifdef HAVE_MMAP
    &posix_wad_file,
#endif
    &stdc_wad_file,
};

#define DOOMWAD ((volatile uint8_t *)0x000000080018000)

wad_file_t *W_OpenFile(char *path)
{
    wad_file_t *result = NULL;

    result = malloc(sizeof(wad_file_t));
    result->mapped = DOOMWAD;
    result->length = 10396254;

   	return result;
}

void W_CloseFile(wad_file_t *wad)
{
    //wad->file_class->CloseFile(wad);
}

size_t W_Read(wad_file_t *wad, unsigned int offset,
              void *buffer, size_t buffer_len)
{
	printf("file read\n");
	print_hex(offset);
	print_hex(buffer_len);
	char * buf = buffer;
	printf("reading DOOM data\n");
	for (int i=offset; i<offset+buffer_len; i++) {
		//print_hex(DOOMWAD[i]);
		buf[i-offset] = DOOMWAD[i];
	}
	printf("end file read...\n");
	return buffer_len;
   // return wad->file_class->Read(wad, offset, buffer, buffer_len);
}

