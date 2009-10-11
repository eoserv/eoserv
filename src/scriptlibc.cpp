
/* scriptlibc.cpp
 * Copyright 2009 the EOSERV development team (http://eoserv.net/devs)
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "scriptlibc.h"

// *printf and *scanf functions are excluded due to a lack of variable argument support
// setjmp and longjmp are excluded due to being completely useless

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <string>

BEGIN_AS_NAMESPACE

#define asFUNCTION2(a, b) asFunctionPtr(a, b)

static char *sp_cast(std::string &s){ return &s[0]; }
static std::string ps_cast(void *p){ return std::string(static_cast<const char *>(p)); }

template<typename T> T &generic_ptr_index(T *p, int i){ return p[i]; }
template<typename T1, typename T2> T2 generic_ptr_cast(T1 p){ return static_cast<T2>(p); }
template<typename T1, typename T2> T2 generic_cast(T1 p){ return static_cast<T2>(p); }

void
*NULL_const = 0;

// errno.h
int
EDOM_const = EDOM,
ERANGE_const = ERANGE;

// float.h
int
FLT_RADIX_const = FLT_RADIX,
FLT_ROUNDS_const = FLT_ROUNDS,
FLT_DIG_const = FLT_DIG,
DBL_DIG_const = DBL_DIG,
LDBL_DIG_const = LDBL_DIG,
FLT_MANT_DIG_const = FLT_MANT_DIG,
DBL_MANT_DIG_const = DBL_MANT_DIG,
LDBL_MANT_DIG_const = LDBL_MANT_DIG,
FLT_MAX_EXP_const = FLT_MAX_EXP,
DBL_MAX_EXP_const = DBL_MAX_EXP,
LDBL_MAX_EXP_const = LDBL_MAX_EXP,
FLT_MIN_EXP_const = FLT_MIN_EXP,
DBL_MIN_EXP_const = DBL_MIN_EXP,
LDBL_MIN_EXP_const = LDBL_MIN_EXP;
float
FLT_EPSILON_const = FLT_EPSILON,
FLT_MAX_const = FLT_MAX,
FLT_MIN_const = FLT_MIN;
double
DBL_EPSILON_const = DBL_EPSILON,
DBL_MAX_const = DBL_MAX,
DBL_MIN_const = DBL_MIN;
//long double
double
LDBL_EPSILON_const = LDBL_EPSILON,
LDBL_MAX_const = LDBL_MAX,
LDBL_MIN_const = LDBL_MIN;

//limits.h
int
CHAR_BIT_const = CHAR_BIT;
char
CHAR_MAX_const = CHAR_MAX,
CHAR_MIN_const = CHAR_MIN;
signed char
SCHAR_MAX_const = SCHAR_MAX,
SCHAR_MIN_const = SCHAR_MIN;
unsigned char
UCHAR_MAX_const = UCHAR_MAX;
short
SHRT_MAX_const = SHRT_MAX,
SHRT_MIN_const = SHRT_MIN;
unsigned short
USHRT_MAX_const = USHRT_MAX;
int
INT_MAX_const = INT_MAX,
INT_MIN_const = INT_MIN;
unsigned int
UINT_MAX_const = UINT_MAX;
long
LONG_MAX_const = LONG_MAX,
LONG_MIN_const = LONG_MIN;
unsigned long
ULONG_MAX_const = ULONG_MAX;

// locale.h
int
LC_ALL_const = LC_ALL,
LC_NUMERIC_const = LC_NUMERIC,
LC_MONETARY_const = LC_MONETARY,
LC_COLLATE_const = LC_COLLATE,
LC_CTYPE_const = LC_CTYPE,
LC_TIME_const = LC_TIME;

// math.h
long double
HUGE_VAL_const = HUGE_VAL;

// signal.h
int
SIGABRT_const = SIGABRT,
SIGFPE_const = SIGFPE,
SIGILL_const = SIGILL,
SIGSEGV_const = SIGSEGV,
SIGTERM_const = SIGTERM;

void
*SIG_DFL_const = reinterpret_cast<void *>(0),
*SIG_ERR_const = reinterpret_cast<void *>(1),
*SIG_IGN_const = reinterpret_cast<void *>(-1);

// stdio.h
int
BUFSIZ_const = BUFSIZ,
EOF_const = EOF,
FILENAME_MAX_const = FILENAME_MAX,
FOPEN_MAX_const = FOPEN_MAX,
L_tmpnam_const = L_tmpnam,
SEEK_CUR_const = SEEK_CUR,
SEEK_END_const = SEEK_END,
SEEK_SET_const = SEEK_SET,
TMP_MAX_const = TMP_MAX,
_IOFBF_const = _IOFBF,
_IOLBF_const = _IOLBF,
_IONBF_const = _IONBF;

// stdlib.h
int
EXIT_FAILURE_const = EXIT_FAILURE,
EXIT_SUCCESS_const = EXIT_SUCCESS,
RAND_MAX_const = RAND_MAX;

// time.h
unsigned long
CLOCKS_PER_SEC_const = CLOCKS_PER_SEC;


// standard streams as ptr

// stdio.h
FILE
*stdinptr = stdin,
*stdoutptr = stdout,
*stderrptr = stderr;

void RegisterScriptLibC(asIScriptEngine *engine)
{
	// types/structs
	engine->RegisterObjectType("char", sizeof(char), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("uchar", sizeof(unsigned char), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("schar", sizeof(signed char), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("short", sizeof(short), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("ushort", sizeof(unsigned short), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("long", sizeof(long), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("ulong", sizeof(unsigned long), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("ldouble", sizeof(long double), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterTypedef("size_t", "uint");
	// locale.h
	engine->RegisterObjectType("lconv", sizeof(lconv), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	// setjmp.h
	engine->RegisterObjectType("jmp_buf", sizeof(jmp_buf), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	// stddef.h
	engine->RegisterObjectType("ptrdiff_t", sizeof(ptrdiff_t), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	// stdio.h
	engine->RegisterObjectType("fpos_t", sizeof(jmp_buf), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	// stdlib.h
	engine->RegisterObjectType("div_t", sizeof(div_t), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("ldiv_t", sizeof(ldiv_t), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	// time.h
	engine->RegisterObjectType("tm", sizeof(tm), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);

	// pointers
	engine->RegisterObjectType("int_ptr", sizeof(int *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("float_ptr", sizeof(float *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("void_ptr", sizeof(void *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("char_ptr", sizeof(char *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("schar_ptr", sizeof(signed char *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("uchar_ptr", sizeof(unsigned char *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("short_ptr", sizeof(signed short *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("ushort_ptr", sizeof(unsigned short *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("uint_ptr", sizeof(unsigned int *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("long_ptr", sizeof(long *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("ulong_ptr", sizeof(unsigned long *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("double_ptr", sizeof(double *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("ldouble_ptr", sizeof(long double *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	// stdio.h
	engine->RegisterObjectType("FILE_ptr", sizeof(FILE *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	// locale.h
	engine->RegisterObjectType("lconv_ptr", sizeof(lconv *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	// setjmp.h
	engine->RegisterObjectType("jmp_buf_ptr", sizeof(jmp_buf *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	// file.h
	engine->RegisterObjectType("fpos_t_ptr", sizeof(fpos_t *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	// time.h
	engine->RegisterObjectType("time_t_ptr", sizeof(fpos_t *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectType("tm_ptr", sizeof(fpos_t *), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);

	// casts
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "char_ptr f()", asFUNCTION2(generic_ptr_cast<void *, char *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "schar_ptr f()", asFUNCTION2(generic_ptr_cast<void *, signed char *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "uchar_ptr f()", asFUNCTION2(generic_ptr_cast<void *, unsigned char *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "int_ptr f()", asFUNCTION2(generic_ptr_cast<void *, int *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "uint_ptr f()", asFUNCTION2(generic_ptr_cast<void *, unsigned int *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "FILE_ptr f()", asFUNCTION2(generic_ptr_cast<void *, FILE *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "lconv_ptr f()", asFUNCTION2(generic_ptr_cast<void *, lconv *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "fpos_t_ptr f()", asFUNCTION2(generic_ptr_cast<void *, fpos_t *>), asCALL_CDECL_OBJFIRST);

	engine->RegisterObjectBehaviour("char_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<char *, void *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("schar_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<signed char *, void *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("uchar_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<unsigned char *, void *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("int_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<int *, void *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("uint_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<unsigned int *, void *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("FILE_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<FILE *, void *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("lconv_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<lconv *, void *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("fpos_t_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<fpos_t *, void *>), asCALL_CDECL_OBJFIRST);

	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "char_ptr f()", asFUNCTION2(generic_ptr_cast<void *, char *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "schar_ptr f()", asFUNCTION2(generic_ptr_cast<void *, signed char *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "uchar_ptr f()", asFUNCTION2(generic_ptr_cast<void *, unsigned char *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "int_ptr f()", asFUNCTION2(generic_ptr_cast<void *, int *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "uint_ptr f()", asFUNCTION2(generic_ptr_cast<void *, unsigned int *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "FILE_ptr f()", asFUNCTION2(generic_ptr_cast<void *, FILE *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "lconv_ptr f()", asFUNCTION2(generic_ptr_cast<void *, lconv *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("void_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "fpos_t_ptr f()", asFUNCTION2(generic_ptr_cast<void *, fpos_t *>), asCALL_CDECL_OBJFIRST);

	engine->RegisterObjectBehaviour("char_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<char *, void *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("schar_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<signed char *, void *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("uchar_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<unsigned char *, void *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("int_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<int *, void *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("uint_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<unsigned int *, void *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("FILE_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<FILE *, void *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("lconv_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<lconv *, void *>), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("fpos_t_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "void_ptr f()", asFUNCTION2(generic_ptr_cast<fpos_t *, void *>), asCALL_CDECL_OBJFIRST);

	engine->RegisterObjectBehaviour("char_ptr", asBEHAVE_IMPLICIT_VALUE_CAST, "string f()", asFUNCTION(ps_cast), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("string", asBEHAVE_IMPLICIT_VALUE_CAST, "char_ptr f()", asFUNCTION(sp_cast), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectBehaviour("char_ptr", asBEHAVE_INDEX, "int8 &f(int)", asFUNCTION(generic_ptr_index<char>), asCALL_CDECL_OBJFIRST);

	// functions, variables and constants

	// stddef.h
	engine->RegisterGlobalProperty("const void_ptr NULL", NULL_const);

	// ctype.h
	engine->RegisterGlobalFunction("int isalnum(int)", asFUNCTION(isalnum), asCALL_CDECL);
	engine->RegisterGlobalFunction("int isalpha(int)", asFUNCTION(isalpha), asCALL_CDECL);
	engine->RegisterGlobalFunction("int iscntrl(int)", asFUNCTION(iscntrl), asCALL_CDECL);
	engine->RegisterGlobalFunction("int isdigit(int)", asFUNCTION(isdigit), asCALL_CDECL);
	engine->RegisterGlobalFunction("int isgraph(int)", asFUNCTION(isgraph), asCALL_CDECL);
	engine->RegisterGlobalFunction("int islower(int)", asFUNCTION(islower), asCALL_CDECL);
	engine->RegisterGlobalFunction("int isprint(int)", asFUNCTION(isprint), asCALL_CDECL);
	engine->RegisterGlobalFunction("int ispunct(int)", asFUNCTION(ispunct), asCALL_CDECL);
	engine->RegisterGlobalFunction("int isspace(int)", asFUNCTION(isspace), asCALL_CDECL);
	engine->RegisterGlobalFunction("int isupper(int)", asFUNCTION(isupper), asCALL_CDECL);
	engine->RegisterGlobalFunction("int isxdigit(int)", asFUNCTION(isxdigit), asCALL_CDECL);
	engine->RegisterGlobalFunction("int tolower(int)", asFUNCTION(tolower), asCALL_CDECL);
	engine->RegisterGlobalFunction("int toupper(int)", asFUNCTION(toupper), asCALL_CDECL);

	// errno.h
	engine->RegisterGlobalProperty("int errno", &errno);
	engine->RegisterGlobalProperty("const int EDOM", &EDOM_const);
	engine->RegisterGlobalProperty("const int ERANGE", &ERANGE_const);

	// float.h
	engine->RegisterGlobalProperty("const int FLT_RADIX", &FLT_RADIX_const);
	engine->RegisterGlobalProperty("const int FLT_ROUNDS", &FLT_ROUNDS_const);
	engine->RegisterGlobalProperty("const float FLT_EPSILON", &FLT_EPSILON_const);
	engine->RegisterGlobalProperty("const double DBL_EPSILON", &DBL_EPSILON_const);
	engine->RegisterGlobalProperty("const double LDBL_EPSILON", &LDBL_EPSILON_const);
	engine->RegisterGlobalProperty("const int FLT_MANT_DIG", &FLT_MANT_DIG_const);
	engine->RegisterGlobalProperty("const int DBL_MANT_DIG", &DBL_MANT_DIG_const);
	engine->RegisterGlobalProperty("const int LDBL_MANT_DIG", &LDBL_MANT_DIG_const);
	engine->RegisterGlobalProperty("const float FLT_MAX", &FLT_MAX_const);
	engine->RegisterGlobalProperty("const double DBL_MAX", &DBL_MAX_const);
	engine->RegisterGlobalProperty("const double LDBL_MAX", &LDBL_MAX_const);
	engine->RegisterGlobalProperty("const int FLT_MAX_EXP", &FLT_MAX_EXP_const);
	engine->RegisterGlobalProperty("const int DBL_MAX_EXP", &DBL_MAX_EXP_const);
	engine->RegisterGlobalProperty("const int LDBL_MAX_EXP", &LDBL_MAX_EXP_const);
	engine->RegisterGlobalProperty("const float FLT_MIN", &FLT_MIN_const);
	engine->RegisterGlobalProperty("const double DBL_MIN", &DBL_MIN_const);
	engine->RegisterGlobalProperty("const double LDBL_MIN", &LDBL_MIN_const);
	engine->RegisterGlobalProperty("const int FLT_MIN_EXP", &FLT_MIN_EXP_const);
	engine->RegisterGlobalProperty("const int DBL_MIN_EXP", &DBL_MIN_EXP_const);
	engine->RegisterGlobalProperty("const int LDBL_MIN_EXP", &LDBL_MIN_EXP_const);

	// limits.h
	engine->RegisterGlobalProperty("const int8 CHAR_BIT", &CHAR_BIT_const);
	engine->RegisterGlobalProperty("const int8 CHAR_MAX", &CHAR_MAX_const);
	engine->RegisterGlobalProperty("const int8 CHAR_MIN", &CHAR_MIN_const);
	engine->RegisterGlobalProperty("const int8 SCHAR_MAX", &SCHAR_MAX_const);
	engine->RegisterGlobalProperty("const int8 SCHAR_MIN", &SCHAR_MIN_const);
	engine->RegisterGlobalProperty("const int8 UCHAR_MAX", &UCHAR_MAX_const);
	engine->RegisterGlobalProperty("const int16 SHRT_MAX", &SHRT_MAX_const);
	engine->RegisterGlobalProperty("const int16 SHRT_MIN", &SHRT_MIN_const);
	engine->RegisterGlobalProperty("const int16 USHRT_MAX", &USHRT_MAX_const);
	engine->RegisterGlobalProperty("const int32 INT_MAX", &INT_MAX_const);
	engine->RegisterGlobalProperty("const int32 INT_MIN", &INT_MIN_const);
	engine->RegisterGlobalProperty("const int32 UINT_MAX", &UINT_MAX_const);
	engine->RegisterGlobalProperty("const int32 LONG_MAX", &LONG_MAX_const);
	engine->RegisterGlobalProperty("const int32 LONG_MIN", &LONG_MIN_const);
	engine->RegisterGlobalProperty("const int32 ULONG_MAX", &ULONG_MAX_const);

	// locale.h
	engine->RegisterObjectProperty("lconv", "char_ptr decimal_point", offsetof(lconv, decimal_point));
	engine->RegisterObjectProperty("lconv", "char_ptr grouping", offsetof(lconv, grouping));
	engine->RegisterObjectProperty("lconv", "char_ptr thousands_sep", offsetof(lconv, thousands_sep));
	engine->RegisterObjectProperty("lconv", "char_ptr currency_symbol", offsetof(lconv, currency_symbol));
	engine->RegisterObjectProperty("lconv", "char_ptr int_curr_symbol", offsetof(lconv, int_curr_symbol));
	engine->RegisterObjectProperty("lconv", "char_ptr mon_decimal_point", offsetof(lconv, mon_decimal_point));
	engine->RegisterObjectProperty("lconv", "char_ptr mon_grouping", offsetof(lconv, mon_grouping));
	engine->RegisterObjectProperty("lconv", "char_ptr mon_thousands_sep", offsetof(lconv, mon_thousands_sep));
	engine->RegisterObjectProperty("lconv", "char_ptr negative_sign", offsetof(lconv, negative_sign));
	engine->RegisterObjectProperty("lconv", "char_ptr positive_sign", offsetof(lconv, positive_sign));
	engine->RegisterObjectProperty("lconv", "int8 frac_digits", offsetof(lconv, frac_digits));
	engine->RegisterObjectProperty("lconv", "int8 int_frac_digits", offsetof(lconv, int_frac_digits));
	engine->RegisterObjectProperty("lconv", "int8 n_cs_precedes", offsetof(lconv, n_cs_precedes));
	engine->RegisterObjectProperty("lconv", "int8 n_sep_by_space", offsetof(lconv, n_sep_by_space));
	engine->RegisterObjectProperty("lconv", "int8 n_sign_posn", offsetof(lconv, n_sign_posn));
	engine->RegisterObjectProperty("lconv", "int8 p_cs_precedes", offsetof(lconv, p_cs_precedes));
	engine->RegisterObjectProperty("lconv", "int8 p_sep_by_space", offsetof(lconv, p_sep_by_space));
	engine->RegisterObjectProperty("lconv", "int8 p_sign_posn", offsetof(lconv, p_sign_posn));
	engine->RegisterGlobalFunction("lconv_ptr localeconv()", asFUNCTION(localeconv), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr setlocale(int, char_ptr)", asFUNCTION(setlocale), asCALL_CDECL);
	engine->RegisterGlobalProperty("const int LC_ALL", &LC_ALL_const);
	engine->RegisterGlobalProperty("const int LC_NUMERIC", &LC_NUMERIC_const);
	engine->RegisterGlobalProperty("const int LC_MONETARY", &LC_MONETARY_const);
	engine->RegisterGlobalProperty("const int LC_COLLATE", &LC_COLLATE_const);
	engine->RegisterGlobalProperty("const int LC_CTYPE", &LC_CTYPE_const);
	engine->RegisterGlobalProperty("const int LC_TIME", &LC_TIME_const);

	// math.h
	engine->RegisterGlobalProperty("const int HUGE_VAL", &HUGE_VAL_const);
	engine->RegisterGlobalFunction("double exp(double)", asFUNCTION(exp), asCALL_CDECL);
	engine->RegisterGlobalFunction("double log(double)", asFUNCTION(log), asCALL_CDECL);
	engine->RegisterGlobalFunction("double log10(double)", asFUNCTION(log10), asCALL_CDECL);
	engine->RegisterGlobalFunction("double pow(double, double)", asFUNCTION(pow), asCALL_CDECL);
	engine->RegisterGlobalFunction("double sqrt(double)", asFUNCTION(sqrt), asCALL_CDECL);
	engine->RegisterGlobalFunction("double ceil(double)", asFUNCTION(ceil), asCALL_CDECL);
	engine->RegisterGlobalFunction("double floor(double)", asFUNCTION(floor), asCALL_CDECL);
	engine->RegisterGlobalFunction("double fabs(double)", asFUNCTION(fabs), asCALL_CDECL);
	engine->RegisterGlobalFunction("double ldexp(double)", asFUNCTION(ldexp), asCALL_CDECL);
	engine->RegisterGlobalFunction("double frexp(double, int_ptr exp)", asFUNCTION(frexp), asCALL_CDECL);
	engine->RegisterGlobalFunction("double modf(double, double_ptr ip)", asFUNCTION(modf), asCALL_CDECL);
	engine->RegisterGlobalFunction("double fmod(double, double)", asFUNCTION(fmod), asCALL_CDECL);
	engine->RegisterGlobalFunction("double sin(double)", asFUNCTION(sin), asCALL_CDECL);
	engine->RegisterGlobalFunction("double cos(double)", asFUNCTION(cos), asCALL_CDECL);
	engine->RegisterGlobalFunction("double tan(double)", asFUNCTION(tan), asCALL_CDECL);
	engine->RegisterGlobalFunction("double asin(double)", asFUNCTION(asin), asCALL_CDECL);
	engine->RegisterGlobalFunction("double acos(double)", asFUNCTION(acos), asCALL_CDECL);
	engine->RegisterGlobalFunction("double atan(double)", asFUNCTION(atan), asCALL_CDECL);
	engine->RegisterGlobalFunction("double atan2(double, double)", asFUNCTION(atan2), asCALL_CDECL);
	engine->RegisterGlobalFunction("double sinh(double)", asFUNCTION(sinh), asCALL_CDECL);
	engine->RegisterGlobalFunction("double cosh(double)", asFUNCTION(cosh), asCALL_CDECL);
	engine->RegisterGlobalFunction("double tanh(double)", asFUNCTION(tanh), asCALL_CDECL);

	// signal.h
	engine->RegisterGlobalProperty("const int SIGABRT", &SIGABRT_const);
	engine->RegisterGlobalProperty("const int SIGFPE", &SIGFPE_const);
	engine->RegisterGlobalProperty("const int SIGILL", &SIGILL_const);
	engine->RegisterGlobalProperty("const int SIGSEGV", &SIGSEGV_const);
	engine->RegisterGlobalProperty("const int SIGTERM", &SIGTERM_const);
	engine->RegisterGlobalProperty("const void_ptr SIG_DFL", &SIG_DFL_const);
	engine->RegisterGlobalProperty("const void_ptr SIG_ERR", &SIG_ERR_const);
	engine->RegisterGlobalProperty("const void_ptr SIG_IGN", &SIG_IGN_const);
	engine->RegisterGlobalFunction("void signal(int, void_ptr)", asFUNCTION(signal), asCALL_CDECL);
	engine->RegisterGlobalFunction("void raise(int)", asFUNCTION(raise), asCALL_CDECL);

	// stdio.h
	engine->RegisterGlobalProperty("const int BUFSIZ", &BUFSIZ_const);
	engine->RegisterGlobalProperty("const int EOF", &EOF_const);
	engine->RegisterGlobalProperty("const int FILENAME_MAX", &FILENAME_MAX_const);
	engine->RegisterGlobalProperty("const int FOPEN_MAX", &FOPEN_MAX_const);
	engine->RegisterGlobalProperty("const int L_tmpnam", &L_tmpnam_const);
	engine->RegisterGlobalProperty("const int SEEK_CUR", &SEEK_CUR_const);
	engine->RegisterGlobalProperty("const int SEEK_END", &SEEK_END_const);
	engine->RegisterGlobalProperty("const int SEEK_SET", &SEEK_SET_const);
	engine->RegisterGlobalProperty("const int TMP_MAX", &TMP_MAX_const);
	engine->RegisterGlobalProperty("const int _IOFBF", &_IOFBF_const);
	engine->RegisterGlobalProperty("const int _IOLBF", &_IOLBF_const);
	engine->RegisterGlobalProperty("const int _IONBF", &_IONBF_const);
	engine->RegisterGlobalProperty("const FILE_ptr stdin", &stdinptr);
	engine->RegisterGlobalProperty("const FILE_ptr stdout", &stdoutptr);
	engine->RegisterGlobalProperty("const FILE_ptr stderr", &stderrptr);
	engine->RegisterGlobalFunction("FILE_ptr fopen(const char_ptr, const char_ptr)", asFUNCTION(fopen), asCALL_CDECL);
	engine->RegisterGlobalFunction("FILE_ptr freopen(const char_ptr, const char_ptr, FILE_ptr)", asFUNCTION(freopen), asCALL_CDECL);
	engine->RegisterGlobalFunction("int fflush(FILE_ptr)", asFUNCTION(fflush), asCALL_CDECL);
	engine->RegisterGlobalFunction("int fclose(FILE_ptr)", asFUNCTION(fclose), asCALL_CDECL);
	engine->RegisterGlobalFunction("int remove(const char_ptr)", asFUNCTION(remove), asCALL_CDECL);
	engine->RegisterGlobalFunction("int rename(const char_ptr, const char_ptr)", asFUNCTION(rename), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr tmpnam(char_ptr)", asFUNCTION(tmpnam), asCALL_CDECL);
	engine->RegisterGlobalFunction("int setvbuf(FILE_ptr, char_ptr, int, size_t)", asFUNCTION(setvbuf), asCALL_CDECL);
	engine->RegisterGlobalFunction("int setbuf(FILE_ptr, char_ptr)", asFUNCTION(setbuf), asCALL_CDECL);
	engine->RegisterGlobalFunction("int fgetc(FILE_ptr)", asFUNCTION(fgetc), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr fgets(char_ptr, int, FILE_ptr)", asFUNCTION(fgets), asCALL_CDECL);
	engine->RegisterGlobalFunction("int fputc(int, FILE_ptr)", asFUNCTION(fputc), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr fputs(const char_ptr, FILE_ptr)", asFUNCTION(fputs), asCALL_CDECL);
	engine->RegisterGlobalFunction("int getc(FILE_ptr)", asFUNCTION(getc), asCALL_CDECL);
	engine->RegisterGlobalFunction("int getchar()", asFUNCTION(getchar), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr gets(char_ptr)", asFUNCTION(gets), asCALL_CDECL);
	engine->RegisterGlobalFunction("int putc(int, FILE_ptr)", asFUNCTION(putc), asCALL_CDECL);
	engine->RegisterGlobalFunction("int putchar(int)", asFUNCTION(putchar), asCALL_CDECL);
	engine->RegisterGlobalFunction("int puts(const char_ptr)", asFUNCTION(puts), asCALL_CDECL);
	engine->RegisterGlobalFunction("int ungetc(int, FILE_ptr)", asFUNCTION(ungetc), asCALL_CDECL);
	engine->RegisterGlobalFunction("size_t fread(void_ptr, size_t, size_t, FILE_ptr)", asFUNCTION(fread), asCALL_CDECL);
	engine->RegisterGlobalFunction("size_t fwrite(const void_ptr, size_t, size_t, FILE_ptr)", asFUNCTION(fwrite), asCALL_CDECL);
	engine->RegisterGlobalFunction("int fseek(FILE_ptr, int, int)", asFUNCTION(fseek), asCALL_CDECL);
	engine->RegisterGlobalFunction("int ftell(FILE_ptr)", asFUNCTION(ftell), asCALL_CDECL);
	engine->RegisterGlobalFunction("void rewind(FILE_ptr)", asFUNCTION(rewind), asCALL_CDECL);
	engine->RegisterGlobalFunction("int fgetpos(FILE_ptr, fpos_t_ptr)", asFUNCTION(fgetpos), asCALL_CDECL);
	engine->RegisterGlobalFunction("int fsetpos(FILE_ptr, const fpos_t_ptr)", asFUNCTION(fsetpos), asCALL_CDECL);
	engine->RegisterGlobalFunction("void clearerr(FILE_ptr)", asFUNCTION(clearerr), asCALL_CDECL);
	engine->RegisterGlobalFunction("int feof(FILE_ptr)", asFUNCTION(feof), asCALL_CDECL);
	engine->RegisterGlobalFunction("int ferror(FILE_ptr)", asFUNCTION(ferror), asCALL_CDECL);
	engine->RegisterGlobalFunction("void perror(const char_ptr)", asFUNCTION(perror), asCALL_CDECL);

	// stdlib.h
	engine->RegisterObjectProperty("div_t", "int quot", offsetof(div_t, quot));
	engine->RegisterObjectProperty("div_t", "int rem", offsetof(div_t, rem));
	engine->RegisterObjectProperty("ldiv_t", "int quot", offsetof(ldiv_t, quot));
	engine->RegisterObjectProperty("ldiv_t", "int rem", offsetof(ldiv_t, rem));
	engine->RegisterGlobalFunction("int abs(int)", asFUNCTION(abs), asCALL_CDECL);
	engine->RegisterGlobalFunction("int labs(int)", asFUNCTION(labs), asCALL_CDECL);
	engine->RegisterGlobalFunction("div_t div(int, int)", asFUNCTION(div), asCALL_CDECL);
	engine->RegisterGlobalFunction("ldiv_t ldiv(int, int)", asFUNCTION(ldiv), asCALL_CDECL);
	engine->RegisterGlobalFunction("double atof(const char_ptr)", asFUNCTION(atof), asCALL_CDECL);
	engine->RegisterGlobalFunction("int atoi(const char_ptr)", asFUNCTION(atoi), asCALL_CDECL);
	engine->RegisterGlobalFunction("int atol(const char_ptr)", asFUNCTION(atol), asCALL_CDECL);
	engine->RegisterGlobalFunction("double strtod(const char_ptr, void_ptr)", asFUNCTION(strtod), asCALL_CDECL);
	engine->RegisterGlobalFunction("int strtol(const char_ptr, void_ptr, int)", asFUNCTION(strtol), asCALL_CDECL);
	engine->RegisterGlobalFunction("uint strtoul(const char_ptr, void_ptr, int)", asFUNCTION(strtoul), asCALL_CDECL);
	engine->RegisterGlobalFunction("void_ptr calloc(size_t, size_t)", asFUNCTION(calloc), asCALL_CDECL);
	engine->RegisterGlobalFunction("void_ptr malloc(size_t)", asFUNCTION(malloc), asCALL_CDECL);
	engine->RegisterGlobalFunction("void_ptr realloc(void_ptr, size_t)", asFUNCTION(realloc), asCALL_CDECL);
	engine->RegisterGlobalFunction("void free(void_ptr)", asFUNCTION(realloc), asCALL_CDECL);
	engine->RegisterGlobalFunction("void abort()", asFUNCTION(abort), asCALL_CDECL);
	engine->RegisterGlobalFunction("void exit(int)", asFUNCTION(exit), asCALL_CDECL);
	engine->RegisterGlobalFunction("int atexit(void_ptr)", asFUNCTION(atexit), asCALL_CDECL);
	engine->RegisterGlobalFunction("int system(char_ptr)", asFUNCTION(system), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr getenv(char_ptr)", asFUNCTION(getenv), asCALL_CDECL);
	engine->RegisterGlobalFunction("void_ptr bsearch(void_ptr, void_ptr, size_t, size_t, void_ptr)", asFUNCTION(bsearch), asCALL_CDECL);
	engine->RegisterGlobalFunction("void qsort(void_ptr, size_t, size_t, void_ptr)", asFUNCTION(qsort), asCALL_CDECL);
	engine->RegisterGlobalFunction("int rand()", asFUNCTION(rand), asCALL_CDECL);
	engine->RegisterGlobalFunction("void srand(uint)", asFUNCTION(srand), asCALL_CDECL);

	// string.h
	engine->RegisterGlobalFunction("char_ptr strcpy(char_ptr, const char_ptr)", asFUNCTION(strcpy), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr strncpy(char_ptr, const char_ptr, size_t)", asFUNCTION(strncpy), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr strcat(char_ptr, const char_ptr)", asFUNCTION(strcat), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr strncat(char_ptr, const char_ptr, size_t)", asFUNCTION(strncat), asCALL_CDECL);
	engine->RegisterGlobalFunction("int strcmp(const char_ptr, const char_ptr)", asFUNCTION(strcmp), asCALL_CDECL);
	engine->RegisterGlobalFunction("int strncmp(const char_ptr, const char_ptr, size_t)", asFUNCTION(strncmp), asCALL_CDECL);
	engine->RegisterGlobalFunction("int strcoll(const char_ptr, const char_ptr)", asFUNCTION(strcoll), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr strchr(const char_ptr, int c)", asFUNCTION(strchr), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr strrchr(const char_ptr, int c)", asFUNCTION(strrchr), asCALL_CDECL);
	engine->RegisterGlobalFunction("size_t strspn(const char_ptr, const char_ptr)", asFUNCTION(strspn), asCALL_CDECL);
	engine->RegisterGlobalFunction("size_t strcspn(const char_ptr, const char_ptr)", asFUNCTION(strcspn), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr strpbrk(const char_ptr, const char_ptr)", asFUNCTION(strpbrk), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr strstr(const char_ptr, const char_ptr)", asFUNCTION(strstr), asCALL_CDECL);
	engine->RegisterGlobalFunction("size_t strlen(const char_ptr)", asFUNCTION(strlen), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr strerror(int)", asFUNCTION(strerror), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr strtok(char_ptr, const char_ptr)", asFUNCTION(strtok), asCALL_CDECL);
	engine->RegisterGlobalFunction("size_t strxfrm(char_ptr, const char_ptr, size_t)", asFUNCTION(strxfrm), asCALL_CDECL);
	engine->RegisterGlobalFunction("void_ptr memcpy(void_ptr, const void_ptr, size_t)", asFUNCTION(memcpy), asCALL_CDECL);
	engine->RegisterGlobalFunction("void_ptr memmove(void_ptr, const void_ptr, size_t)", asFUNCTION(memmove), asCALL_CDECL);
	engine->RegisterGlobalFunction("int memcmp(void_ptr, const void_ptr, size_t)", asFUNCTION(memcmp), asCALL_CDECL);
	engine->RegisterGlobalFunction("void_ptr memchr(const void_ptr, int, size_t)", asFUNCTION(memchr), asCALL_CDECL);
	engine->RegisterGlobalFunction("void_ptr memset(void_ptr, int, size_t)", asFUNCTION(memset), asCALL_CDECL);

	// time.h
	engine->RegisterGlobalProperty("const uint CLOCKS_PER_SEC", &CLOCKS_PER_SEC_const);
	engine->RegisterObjectProperty("tm", "int tm_sec", offsetof(tm, tm_sec));
	engine->RegisterObjectProperty("tm", "int tm_min", offsetof(tm, tm_min));
	engine->RegisterObjectProperty("tm", "int tm_hour", offsetof(tm, tm_hour));
	engine->RegisterObjectProperty("tm", "int tm_mday", offsetof(tm, tm_mday));
	engine->RegisterObjectProperty("tm", "int tm_mon", offsetof(tm, tm_mon));
	engine->RegisterObjectProperty("tm", "int tm_year", offsetof(tm, tm_year));
	engine->RegisterObjectProperty("tm", "int tm_wday", offsetof(tm, tm_wday));
	engine->RegisterObjectProperty("tm", "int tm_yday", offsetof(tm, tm_yday));
	engine->RegisterObjectProperty("tm", "int tm_isdst", offsetof(tm, tm_isdst));
	engine->RegisterGlobalFunction("uint clock()", asFUNCTION(clock), asCALL_CDECL);
	engine->RegisterGlobalFunction("uint time(time_t_ptr)", asFUNCTION(time), asCALL_CDECL);
	engine->RegisterGlobalFunction("double difftime(uint, uint)", asFUNCTION(difftime), asCALL_CDECL);
	engine->RegisterGlobalFunction("uint mktime(tm_ptr)", asFUNCTION(mktime), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr asctime(const tm_ptr)", asFUNCTION(asctime), asCALL_CDECL);
	engine->RegisterGlobalFunction("char_ptr ctime(const time_t_ptr)", asFUNCTION(ctime), asCALL_CDECL);
	engine->RegisterGlobalFunction("tm_ptr gmtime(const time_t_ptr)", asFUNCTION(gmtime), asCALL_CDECL);
	engine->RegisterGlobalFunction("tm_ptr localtime(const time_t_ptr)", asFUNCTION(localtime), asCALL_CDECL);
	engine->RegisterGlobalFunction("size_t strftime(char_ptr, size_t, const char_ptr, const tm_ptr)", asFUNCTION(strftime), asCALL_CDECL);
}

END_AS_NAMESPACE
