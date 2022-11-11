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
//	Main program, simply calls D_DoomMain high level loop.
//

#include "config.h"

#include "stdio.h"

#include "doomtype.h"
#include "i_system.h"
#include "m_argv.h"

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
//

void D_DoomMain (void);

int printf(const char *format, ...) {
	return 0;
}

int snprintf(char *str, size_t size, const char *format, ...) {
	return 0;
}

int fprintf(FILE *stream, const char *format, ...){
	return 0;
}

int sscanf ( const char * s, const char * format, ...) {
	return 0;
}

void *memcpy(void *dest, const void * src, size_t n) {
	return 0;
}

void *memset(void *str, int c, size_t n) {
	return 0;
}

int abs(int x) {
	return x;
}

void free( void *ptr ) {
}

void *calloc(size_t nitems, size_t size) {
	return 0;
}

int strncmp(const char *str1, const char *str2, size_t n) {
	return 0;
}

char* strncpy ( char * destination, const char * source, size_t num ) {
	return 0;
}

int strcasecmp(const char *string1, const char *string2) {
	return 0;
}

size_t strlen(const char *str) {
	return 0;
}

void *malloc(size_t size) {
	return 0;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	return 0;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
	return 0;
}

long int ftell(FILE *stream) {
	return 0;
}

int toupper(int ch) {
	return 0;
}

int strncasecmp(const char *string1, const char *string2, size_t count) {
	return 0;
}

char * strdup( const char *str1 ) {
	return 0;
}

int strcmp (const char* str1, const char* str2) {
	return 0;
}

const char * strstr ( const char * str1, const char * str2 ) {
	return 0;
}

const char * strrchr ( const char * str, int character ) {
	return 0;
}

int atoi (const char * str) {
	return 0;
}

FILE * fopen ( const char * filename, const char * mode ) {
	return 0;
}

int fclose(FILE *stream) {
	return 0;
}

void *realloc( void *ptr, size_t new_size ) {
	return 0;
}

int vanilla_keyboard_mapping = true;

void log(int c) {
	__asm__("ecall");
}
void * memmove ( void * destination, const void * source, size_t num ) {
	return 0;
}

int isspace(int argument) {
	return 0;
}


void _start()
{

	__asm__("ecall");
    M_FindResponseFile();

    // start doom
    printf("Starting D_DoomMain\r\n");
    D_DoomMain ();

}

