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
#include <stdarg.h>

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
//

void D_DoomMain (void);

#define UART ((volatile uint8_t *)0x10000000)

const char HEXTABLE[16] = "0123456789ABCDEF";

void print_hex(size_t a) {
	printf("HEX: ");
	for (int i = 15; i >= 0; i--) {
		unsigned int index = (a >> (i*4)) & 0xF;
		char c = HEXTABLE[index];
		UART[0] = c;
	}
	printf("\n");
}

int printf(const char *format, ...) {
	int i = 0;
	while (format[i] != 0x00) {
		UART[0] = format[i];
		i++;
	}
	return 0;
}

/** add padding to string */
static void
print_pad(char** at, size_t* left, int* ret, char p, int num)
{
	while(num--) {
		if(*left > 1) {
			*(*at)++ = p;
			(*left)--;
		}
		(*ret)++;
	}
}

/** get negative symbol, 0 if none */
static char
get_negsign(int negative, int plus, int space)
{
	if(negative)
		return '-';
	if(plus)
		return '+';
	if(space)
		return ' ';
	return 0;
}

#define PRINT_DEC_BUFSZ 32 /* 20 is enough for 64 bit decimals */
/** print decimal into buffer, returns length */
static int
print_dec(char* buf, int max, unsigned int value)
{
	int i = 0;
	if(value == 0) {
		if(max > 0) {
			buf[0] = '0';
			i = 1;
		}
	} else while(value && i < max) {
		buf[i++] = '0' + value % 10;
		value /= 10;
	}
	return i;
}

/** print long decimal into buffer, returns length */
static int
print_dec_l(char* buf, int max, unsigned long value)
{
	int i = 0;
	if(value == 0) {
		if(max > 0) {
			buf[0] = '0';
			i = 1;
		}
	} else while(value && i < max) {
		buf[i++] = '0' + value % 10;
		value /= 10;
	}
	return i;
}

/** print long decimal into buffer, returns length */
static int
print_dec_ll(char* buf, int max, unsigned long long value)
{
	int i = 0;
	if(value == 0) {
		if(max > 0) {
			buf[0] = '0';
			i = 1;
		}
	} else while(value && i < max) {
		buf[i++] = '0' + value % 10;
		value /= 10;
	}
	return i;
}

/** print hex into buffer, returns length */
static int
print_hexa(char* buf, int max, unsigned int value)
{
	const char* h = "0123456789abcdef";
	int i = 0;
	if(value == 0) {
		if(max > 0) {
			buf[0] = '0';
			i = 1;
		}
	} else while(value && i < max) {
		buf[i++] = h[value & 0x0f];
		value >>= 4;
	}
	return i;
}

/** print long hex into buffer, returns length */
static int
print_hex_l(char* buf, int max, unsigned long value)
{
	const char* h = "0123456789abcdef";
	int i = 0;
	if(value == 0) {
		if(max > 0) {
			buf[0] = '0';
			i = 1;
		}
	} else while(value && i < max) {
		buf[i++] = h[value & 0x0f];
		value >>= 4;
	}
	return i;
}

/** print long long hex into buffer, returns length */
static int
print_hex_ll(char* buf, int max, unsigned long long value)
{
	const char* h = "0123456789abcdef";
	int i = 0;
	if(value == 0) {
		if(max > 0) {
			buf[0] = '0';
			i = 1;
		}
	} else while(value && i < max) {
		buf[i++] = h[value & 0x0f];
		value >>= 4;
	}
	return i;
}

/** copy string into result, reversed */
static void
spool_str_rev(char** at, size_t* left, int* ret, const char* buf, int len)
{
	int i = len;
	while(i) {
		if(*left > 1) {
			*(*at)++ = buf[--i];
			(*left)--;
		} else --i;
		(*ret)++;
	}
}

/** copy string into result */
static void
spool_str(char** at, size_t* left, int* ret, const char* buf, int len)
{
	int i;
	for(i=0; i<len; i++) {
		if(*left > 1) {
			*(*at)++ = buf[i];
			(*left)--;
		}
		(*ret)++;
	}
}

/** print number formatted */
static void
print_num(char** at, size_t* left, int* ret, int minw, int precision,
	int prgiven, int zeropad, int minus, int plus, int space,
	int zero, int negative, char* buf, int len)
{
	int w = len; /* excludes minus sign */
	char s = get_negsign(negative, plus, space);
	if(minus) {
		/* left adjust the number into the field, space padding */
		/* calc numw = [sign][zeroes][number] */
		int numw = w;
		if(precision == 0 && zero) numw = 0;
		if(numw < precision) numw = precision;
		if(s) numw++;

		/* sign */
		if(s) print_pad(at, left, ret, s, 1);

		/* number */
		if(precision == 0 && zero) {
			/* "" for the number */
		} else {
			if(w < precision)
				print_pad(at, left, ret, '0', precision - w);
			spool_str_rev(at, left, ret, buf, len);
		}
		/* spaces */
		if(numw < minw)
			print_pad(at, left, ret, ' ', minw - numw);
	} else {
		/* pad on the left of the number */
		/* calculate numw has width of [sign][zeroes][number] */
		int numw = w;
		if(precision == 0 && zero) numw = 0;
		if(numw < precision) numw = precision;
		if(!prgiven && zeropad && numw < minw) numw = minw;
		else if(s) numw++;

		/* pad with spaces */
		if(numw < minw)
			print_pad(at, left, ret, ' ', minw - numw);
		/* print sign (and one less zeropad if so) */
		if(s) {
			print_pad(at, left, ret, s, 1);
			numw--;
		}
		/* pad with zeroes */
		if(w < numw)
			print_pad(at, left, ret, '0', numw - w);
		if(precision == 0 && zero)
			return;
		/* print the characters for the value */
		spool_str_rev(at, left, ret, buf, len);
	}
}

/** print %d and %i */
static void
print_num_d(char** at, size_t* left, int* ret, int value,
	int minw, int precision, int prgiven, int zeropad, int minus,
	int plus, int space)
{
	char buf[PRINT_DEC_BUFSZ];
	int negative = (value < 0);
	int zero = (value == 0);
	int len = print_dec(buf, (int)sizeof(buf),
		(unsigned int)(negative?-value:value));
	print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
		plus, space, zero, negative, buf, len);
}

/** print %ld and %li */
static void
print_num_ld(char** at, size_t* left, int* ret, long value,
	int minw, int precision, int prgiven, int zeropad, int minus,
	int plus, int space)
{
	char buf[PRINT_DEC_BUFSZ];
	int negative = (value < 0);
	int zero = (value == 0);
	int len = print_dec_l(buf, (int)sizeof(buf),
		(unsigned long)(negative?-value:value));
	print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
		plus, space, zero, negative, buf, len);
}

/** print %lld and %lli */
static void
print_num_lld(char** at, size_t* left, int* ret, long long value,
	int minw, int precision, int prgiven, int zeropad, int minus,
	int plus, int space)
{
	char buf[PRINT_DEC_BUFSZ];
	int negative = (value < 0);
	int zero = (value == 0);
	int len = print_dec_ll(buf, (int)sizeof(buf),
		(unsigned long long)(negative?-value:value));
	print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
		plus, space, zero, negative, buf, len);
}

/** print %u */
static void
print_num_u(char** at, size_t* left, int* ret, unsigned int value,
	int minw, int precision, int prgiven, int zeropad, int minus,
	int plus, int space)
{
	char buf[PRINT_DEC_BUFSZ];
	int negative = 0;
	int zero = (value == 0);
	int len = print_dec(buf, (int)sizeof(buf), value);
	print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
		plus, space, zero, negative, buf, len);
}

/** print %lu */
static void
print_num_lu(char** at, size_t* left, int* ret, unsigned long value,
	int minw, int precision, int prgiven, int zeropad, int minus,
	int plus, int space)
{
	char buf[PRINT_DEC_BUFSZ];
	int negative = 0;
	int zero = (value == 0);
	int len = print_dec_l(buf, (int)sizeof(buf), value);
	print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
		plus, space, zero, negative, buf, len);
}

/** print %llu */
static void
print_num_llu(char** at, size_t* left, int* ret, unsigned long long value,
	int minw, int precision, int prgiven, int zeropad, int minus,
	int plus, int space)
{
	char buf[PRINT_DEC_BUFSZ];
	int negative = 0;
	int zero = (value == 0);
	int len = print_dec_ll(buf, (int)sizeof(buf), value);
	print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
		plus, space, zero, negative, buf, len);
}

/** print %x */
static void
print_num_x(char** at, size_t* left, int* ret, unsigned int value,
	int minw, int precision, int prgiven, int zeropad, int minus,
	int plus, int space)
{
	char buf[PRINT_DEC_BUFSZ];
	int negative = 0;
	int zero = (value == 0);
	int len = print_hexa(buf, (int)sizeof(buf), value);
	print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
		plus, space, zero, negative, buf, len);
}

/** print %lx */
static void
print_num_lx(char** at, size_t* left, int* ret, unsigned long value,
	int minw, int precision, int prgiven, int zeropad, int minus,
	int plus, int space)
{
	char buf[PRINT_DEC_BUFSZ];
	int negative = 0;
	int zero = (value == 0);
	int len = print_hex_l(buf, (int)sizeof(buf), value);
	print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
		plus, space, zero, negative, buf, len);
}

/** print %llx */
static void
print_num_llx(char** at, size_t* left, int* ret, unsigned long long value,
	int minw, int precision, int prgiven, int zeropad, int minus,
	int plus, int space)
{
	char buf[PRINT_DEC_BUFSZ];
	int negative = 0;
	int zero = (value == 0);
	int len = print_hex_ll(buf, (int)sizeof(buf), value);
	print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
		plus, space, zero, negative, buf, len);
}

/** print %llp */
static void
print_num_llp(char** at, size_t* left, int* ret, void* value,
	int minw, int precision, int prgiven, int zeropad, int minus,
	int plus, int space)
{
	char buf[PRINT_DEC_BUFSZ];
	int negative = 0;
	int zero = (value == 0);
#if defined(UINTPTR_MAX) && defined(UINT32_MAX) && (UINTPTR_MAX == UINT32_MAX)
	/* avoid warning about upcast on 32bit systems */
	unsigned long long llvalue = (unsigned long)value;
#else
	unsigned long long llvalue = (unsigned long long)value;
#endif
	int len = print_hex_ll(buf, (int)sizeof(buf), llvalue);
	if(zero) {
		buf[0]=')';
		buf[1]='l';
		buf[2]='i';
		buf[3]='n';
		buf[4]='(';
		len = 5;
	} else {
		/* put '0x' in front of the (reversed) buffer result */
		if(len < PRINT_DEC_BUFSZ)
			buf[len++] = 'x';
		if(len < PRINT_DEC_BUFSZ)
			buf[len++] = '0';
	}
	print_num(at, left, ret, minw, precision, prgiven, zeropad, minus,
		plus, space, zero, negative, buf, len);
}

#define PRINT_FLOAT_BUFSZ 64 /* xx.yy with 20.20 about the max */
/** spool remainder after the decimal point to buffer, in reverse */
static int
print_remainder(char* buf, int max, double r, int prec)
{
	unsigned long long cap = 1;
	unsigned long long value;
	int len, i;
	if(prec > 19) prec = 19; /* max we can do */
	if(max < prec) return 0;
	for(i=0; i<prec; i++) {
		cap *= 10;
	}
	r *= (double)cap;
	value = (unsigned long long)r;
	/* see if we need to round up */
	if(((unsigned long long)((r - (double)value)*10.0)) >= 5) {
		value++;
		/* that might carry to numbers before the comma, if so,
		 * just ignore that rounding. failure because 64bitprintout */
		if(value >= cap)
			value = cap-1;
	}
	len = print_dec_ll(buf, max, value);
	while(len < prec) { /* pad with zeroes, e.g. if 0.0012 */
		buf[len++] = '0';
	}
	if(len < max)
		buf[len++] = '.';
	return len;
}

/** spool floating point to buffer */
static int
print_float(char* buf, int max, double value, int prec)
{
	/* as xxx.xxx  if prec==0, no '.', with prec decimals after . */
	/* no conversion for NAN and INF, because we do not want to require
	   linking with -lm. */
	/* Thus, the conversions use 64bit integers to convert the numbers,
	 * which makes 19 digits before and after the decimal point the max */
	unsigned long long whole = (unsigned long long)value;
	double remain = value - (double)whole;
	int len = 0;
	if(prec != 0)
		len = print_remainder(buf, max, remain, prec);
	len += print_dec_ll(buf+len, max-len, whole);
	return len;
}

/** print %f */
static void
print_num_f(char** at, size_t* left, int* ret, double value,
	int minw, int precision, int prgiven, int zeropad, int minus,
	int plus, int space)
{
	char buf[PRINT_FLOAT_BUFSZ];
	int negative = (value < 0);
	int zero = 0;
	int len;
	if(!prgiven) precision = 6;
	len = print_float(buf, (int)sizeof(buf), negative?-value:value,
		precision);
	print_num(at, left, ret, minw, 1, 0, zeropad, minus,
		plus, space, zero, negative, buf, len);
}



void *memmove(void * restrict dst, const void *src, size_t n) {
	return memcpy(dst, src, n);
}




static int
my_strnlen(const char* s, int max)
{
	int i;
	for(i=0; i<max; i++)
		if(s[i]==0)
			return i;
	return max;
}

/** print %s */
static void
print_str(char** at, size_t* left, int* ret, char* s,
	int minw, int precision, int prgiven, int minus)
{
	size_t w = my_strnlen(s,precision); /* up to the nul */
	if(w < minw && !minus)
		print_pad(at, left, ret, ' ', minw - w);
	spool_str(at, left, ret, s, w);
	if(w < minw && minus)
		print_pad(at, left, ret, ' ', minw - w);
}

/** print %c */
static void
print_char(char** at, size_t* left, int* ret, int c,
	int minw, int minus)
{
	if(1 < minw && !minus)
		print_pad(at, left, ret, ' ', minw - 1);
	print_pad(at, left, ret, c, 1);
	if(1 < minw && minus)
		print_pad(at, left, ret, ' ', minw - 1);
}

int vsnprintf(char* str, size_t size, const char* format, va_list arg)
{
	char* at = str;
	size_t left = size;
	int ret = 0;
	const char* fmt = format;
	int conv, minw, precision, prgiven, zeropad, minus, plus, space, length;
	while(*fmt) {
		/* copy string before % */
		while(*fmt && *fmt!='%') {
			if(left > 1) {
				*at++ = *fmt++;
				left--;
			} else fmt++;
			ret++;
		}

		/* see if we are at end */
		if(!*fmt) break;

		/* fetch next argument % designation from format string */
		fmt++; /* skip the '%' */

		/********************************/
		/* get the argument designation */
		/********************************/
		/* we must do this vararg stuff inside this function for
		 * portability.  Hence, get_designation, and print_designation
		 * are not their own functions. */

		/* printout designation:
		 * conversion specifier: x, d, u, s, c, n, m, p
		 * flags: # not supported
		 *        0 zeropad (on the left)
		 *	  - left adjust (right by default)
		 *	  ' ' printspace for positive number (in - position).
		 *	  + alwayssign
		 * fieldwidth: [1-9][0-9]* minimum field width.
		 * 	if this is * then type int next argument specifies the minwidth.
		 * 	if this is negative, the - flag is set (with positive width).
		 * precision: period[digits]*, %.2x.
		 * 	if this is * then type int next argument specifies the precision.
		 *	just '.' or negative value means precision=0.
		 *		this is mindigits to print for d, i, u, x
		 *		this is aftercomma digits for f
		 *		this is max number significant digits for g
		 *		maxnumber characters to be printed for s
		 * length: 0-none (int), 1-l (long), 2-ll (long long)
		 * 	notsupported: hh (char), h (short), L (long double), q, j, z, t
		 * Does not support %m$ and *m$ argument designation as array indices.
		 * Does not support %#x
		 *
		 */
		minw = 0;
		precision = 1;
		prgiven = 0;
		zeropad = 0;
		minus = 0;
		plus = 0;
		space = 0;
		length = 0;

		/* get flags in any order */
		for(;;) {
			if(*fmt == '0')
				zeropad = 1;
			else if(*fmt == '-')
				minus = 1;
			else if(*fmt == '+')
				plus = 1;
			else if(*fmt == ' ')
				space = 1;
			else break;
			fmt++;
		}

		/* field width */
		if(*fmt == '*') {
			fmt++; /* skip char */
			minw = va_arg(arg, int);
			if(minw < 0) {
				minus = 1;
				minw = -minw;
			}
		} else while(*fmt >= '0' && *fmt <= '9') {
			minw = minw*10 + (*fmt++)-'0';
		}

		/* precision */
		if(*fmt == '.') {
			fmt++; /* skip period */
			prgiven = 1;
			precision = 0;
			if(*fmt == '*') {
				fmt++; /* skip char */
				precision = va_arg(arg, int);
				if(precision < 0)
					precision = 0;
			} else while(*fmt >= '0' && *fmt <= '9') {
				precision = precision*10 + (*fmt++)-'0';
			}
		}

		/* length */
		if(*fmt == 'l') {
			fmt++; /* skip char */
			length = 1;
			if(*fmt == 'l') {
				fmt++; /* skip char */
				length = 2;
			}
		}

		/* get the conversion */
		if(!*fmt) conv = 0;
		else	conv = *fmt++;

		/***********************************/
		/* print that argument designation */
		/***********************************/
		switch(conv) {
		case 'i':
		case 'd':
			if(length == 0)
			    print_num_d(&at, &left, &ret, va_arg(arg, int),
				minw, precision, prgiven, zeropad, minus, plus, space);
			else if(length == 1)
			    print_num_ld(&at, &left, &ret, va_arg(arg, long),
				minw, precision, prgiven, zeropad, minus, plus, space);
			else if(length == 2)
			    print_num_lld(&at, &left, &ret,
				va_arg(arg, long long),
				minw, precision, prgiven, zeropad, minus, plus, space);
			break;
		case 'u':
			if(length == 0)
			    print_num_u(&at, &left, &ret,
				va_arg(arg, unsigned int),
				minw, precision, prgiven, zeropad, minus, plus, space);
			else if(length == 1)
			    print_num_lu(&at, &left, &ret,
				va_arg(arg, unsigned long),
				minw, precision, prgiven, zeropad, minus, plus, space);
			else if(length == 2)
			    print_num_llu(&at, &left, &ret,
				va_arg(arg, unsigned long long),
				minw, precision, prgiven, zeropad, minus, plus, space);
			break;
		case 'x':
			if(length == 0)
			    print_num_x(&at, &left, &ret,
				va_arg(arg, unsigned int),
				minw, precision, prgiven, zeropad, minus, plus, space);
			else if(length == 1)
			    print_num_lx(&at, &left, &ret,
				va_arg(arg, unsigned long),
				minw, precision, prgiven, zeropad, minus, plus, space);
			else if(length == 2)
			    print_num_llx(&at, &left, &ret,
				va_arg(arg, unsigned long long),
				minw, precision, prgiven, zeropad, minus, plus, space);
			break;
		case 's':
			print_str(&at, &left, &ret, va_arg(arg, char*),
				minw, precision, prgiven, minus);
			break;
		case 'c':
			print_char(&at, &left, &ret, va_arg(arg, int),
				minw, minus);
			break;
		case 'n':
			*va_arg(arg, int*) = ret;
			break;
		case 'm':
			//print_str(&at, &left, &ret, strerror(errno),
			//	minw, precision, prgiven, minus);
			break;
		case 'p':
			print_num_llp(&at, &left, &ret, va_arg(arg, void*),
				minw, precision, prgiven, zeropad, minus, plus, space);
			break;
		case '%':
			print_pad(&at, &left, &ret, '%', 1);
			break;
		case 'f':
			print_num_f(&at, &left, &ret, va_arg(arg, double),
				minw, precision, prgiven, zeropad, minus, plus, space);
			break;
		/* unknown */
		default:
		case 0: break;
		}
	}

	/* zero terminate */
	if(left > 0)
		*at = 0;
	return ret;
}



int snprintf(char *str, size_t size, const char *format, ...) {
     size_t ret;
         va_list ap;
         va_start(ap, format);
         ret = vsnprintf(str, size, format, ap);
         va_end(ap);
         return ret;
}

int fprintf(FILE *stream, const char *format, ...){
	printf("entering fprintf\n");
	__asm__("wfi");
	return 0;
}

int sscanf ( const char * s, const char * format, ...) {
	printf("entering sscanf\n");
	__asm__("wfi");
	return 0;
}

void *memcpy(void *dest, const void * src, size_t n) {
	char* d = dest;
	char* s = src;
	for (int i = 0; i < n; i++) {
		d[i] = s[i];
	}
	return dest;
}

void *memset(void *str, int c, size_t n) {
	//printf("entering memset\n");

	for (int i =0; i < n; i++) {
		char * s = str;
		*s = c;
		str++;
	}
	return str;
}

int abs(int v) {
 	return v * ((v>0) - (v<0));
}

void free( void *ptr ) {
	printf("entering free\n");

}

void *calloc(size_t nitems, size_t size) {
	printf("calloc\n");
	size_t c = nitems *  size;
	void * b;
    	__asm__("mv a1, %1; ecall; mv %1, a0": "=r"(b): "r"(c):"%a0");

	return b;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while ( n && *s1 && ( *s1 == *s2 ) )
    {
        ++s1;
        ++s2;
        --n;
    }
    if ( n == 0 )
    {
        return 0;
    }
    else
    {
        return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
    }
}

char* strncpy ( char * destination, const char * source, size_t num ) {
	//	printf("entering strncpy\n");
		boolean end = false;
 	for(int i =0; i < num; i++) {
 		if (!end) {
 			if (source[i] == 0x00) {
 				end = true;
 			}
 			destination[i] = source[i];
 		} else {
 			destination[i] =0x00;
 		}

 	}
	return destination;
}

int strcasecmp(const char *s1, const char *s2) {
printf("strcasecmp\n");
 const unsigned char *p1 = (const unsigned char *) s1;
  const unsigned char *p2 = (const unsigned char *) s2;
  int result;
  if (p1 == p2)
    return 0;
  while ((result = tolower (*p1) - tolower (*p2++)) == 0)
    if (*p1++ == '\0')
      break;
  return result;
}

char tocharlower(char *str)
{
        if(*str >=65 && *str<=90)
        {
            // It fails in the below assignment
            *str = *str + 32;
        }
        return str;
}

void tolower(char *str)
{
printf("tolower\n");
    while(*str != '\0')
    {
        if(*str >=65 && *str<=90)
        {
            // It fails in the below assignment
            *str = *str + 32;
        }
        str++;
    }

}

size_t strlen(const char *str) {
	int i =0 ;
	while (str[i] != 0x00) {
		i++;
	}
	return i;
}

void *malloc(size_t size) {
	printf("entering malloc(\n");
	print_hex(size);
	void * b;
	__asm__("mv a1, %1; ecall; mv %1, a0": "=r"(b): "r"(size):"%a0");

	print_hex((size_t)(b));
	return b;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	printf("entering fread\n");
	__asm__("wfi");
	return 0;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
		printf("entering fwrite\n");
  __asm__("wfi");
	return 0;
}

long int ftell(FILE *stream) {
		printf("entering ftell\n");
    	__asm__("wfi");
	return 0;
}

int toupper(int ch) {
	//printf("entering toupper\n");
	if( ch >= 97 && ch <= 122) {
		return ch - 32;
	}
	return ch;
}

int strncasecmp(const char *s1, const char *s2, size_t n) {
//	printf("entering strncasecmp\n");
	//printf(s1);
	//	printf(" == ");
	//printf(s2);
	//printf("\n");
   if (n > 0 && s1 != s2)
    {
        do {
            if (toupper(*s1) != toupper(*s2) || *s1 == '\0' || *s2 == '\0') {
                return toupper(*s1) - toupper(*s2);
            }
            s1++;
            s2++;
        } while (--n);
    }
    return 0;
}


char * strdup( const char *str1 ) {
	printf("entering strdup\n");
	__asm__("wfi");
	return 0;
}

//https://stackoverflow.com/questions/34873209/implementation-of-strcmp
int strcmp (const char* s1, const char* s2) {
	//printf("entering strcmp\n");
	//printf(s1);
	//printf(" =?= ");
	//printf(s2);
	//printf("\n");
    while(*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

const char * strstr ( const char * str1, const char * str2 ) {
	printf("entering strstr\n");
	__asm__("wfi");
	return 0;
}

const char * strrchr ( const char * str, int character ) {
		printf("entering strrchr\n");
    	__asm__("wfi");
	return 0;
}

int atoi (const char * str) {
		printf("entering atoi\n");
    	__asm__("wfi");
	return 0;
}

FILE * fopen ( const char * filename, const char * mode ) {
		printf("entering fopen ");
		printf(filename);
				printf("\n");

	return 0;
}

int fclose(FILE *stream) {
		printf("entering fclose\n");
    	__asm__("wfi");
	return 0;
}

void *realloc( void *ptr, size_t new_size ) {
		printf("entering realloc\n");
    	//__asm__("wfi");
	return malloc(new_size);
}



int isspace(int argument) {
		printf("entering isspace\n");
    	__asm__("wfi");
	return 0;
}


int vanilla_keyboard_mapping = true;

void log(int c) {
	__asm__("ecall");
}


void _start()
{

// https://github.com/fwsGonzo/libriscv/blob/master/docs/FREESTANDING.md
asm volatile
("   .option push 				\t\n\
	 .option norelax 			\t\n\
	 1:auipc gp, %pcrel_hi(__global_pointer$) \t\n\
	 addi  gp, gp, %pcrel_lo(1b) \t\n\
	.option pop					\t\n\
");
// make sure all accesses to static memory happen after:
asm volatile("" ::: "memory");



	//__asm__("ecall");
   // M_FindResponseFile();

    // start doom
    printf("Starting D_DoomMain\r\n");
    D_DoomMain ();

}

