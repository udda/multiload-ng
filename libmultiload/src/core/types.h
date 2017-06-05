/*
 * Copyright (C) 2017 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of Multiload-ng.
 *
 * Multiload-ng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Multiload-ng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Multiload-ng.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ML_HEADER__CORE_TYPES_H__INCLUDED
#define ML_HEADER__CORE_TYPES_H__INCLUDED

// extern(C) for C++ compilers
#ifdef __cplusplus
#define ML_HEADER_BEGIN extern "C" {
#define ML_HEADER_END   }
#else
#define ML_HEADER_BEGIN
#define ML_HEADER_END
#endif


ML_HEADER_BEGIN


/* Invalid return value in enums. Some compilers (like GCC) has unsigned enums,
 * so we can't just use -1. To play safe, we use the largest value of the smallest
 * signed type (int8_t), assuming that there are no enums that reaches 127 */
#define ML_INVALID (INT8_MAX)


// convenience typedef for pointers (increases readability)
typedef void * mlPointer;


// Function type to be used when a malloc'd object needs to be destroyed (free() is a valid MlDestroyFunc)
typedef void (*MlDestroyFunc)(mlPointer);

// Function type to be used when calculating byte size of objects
typedef size_t (*MlSizeofFunc)(mlPointer);

// Function type to be used when comparing objects (usually when sorting arrays)
//typedef int (*MlCompareFunc)(const void *, const void *);
typedef int (*MlCompareFunc)(const void *, const void *);



/* gettext convenience macros */
#define _(str) (dgettext((GETTEXT_PACKAGE), (str)))
#define N_(str) (str)


///
/// The following are GCC-specific attributes and optimizations
///

// Definitions for function attributes
// See: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html
#define	ML_FN_RETURNS_NONNULL			__attribute__((returns_nonnull))
#define	ML_FN_NONNULL_ALL				__attribute__((nonnull))
#define	ML_FN_NONNULL(...)				__attribute__((nonnull (__VA_ARGS__)))
#define ML_FN_ALLOC_SIZE(...)			__attribute__((alloc_size (__VA_ARGS__)))
#define ML_FN_MALLOC					__attribute__((malloc))
#define ML_FN_HOT						__attribute__((hot))
#define ML_FN_COLD						__attribute__((cold))
#define ML_FN_CONSTRUCTOR				__attribute__((constructor))
#define ML_FN_DESTRUCTOR				__attribute__((destructor))
#define ML_FN_PURE						__attribute__((pure))
#define ML_FN_CONST						__attribute__((const))
#define ML_FN_WARN_UNUSED_RESULT		__attribute__((warn_unused_result))
#define ML_FN_PRINTF(fmt,va_start)		__attribute__((format (printf, (fmt), (va_start))))
#define ML_FN_NORETURN					__attribute__((noreturn))
#define ML_FN_EXPORT					__attribute__((visibility("default")))
#define ML_FN_CONSTRUCTOR				__attribute__((constructor))
#define ML_FN_DESTRUCTOR				__attribute__((destructor))

// Attribute collections for common functions
#define ML_FN_SIZEOF					__attribute__((cold, pure))
#define ML_FN_READ_PROPERTY				__attribute__((pure, warn_unused_result))
#define ML_FN_READ_DYNAMIC_PROPERTY		__attribute__((warn_unused_result))

// Definitions for type attributes
// See: https://gcc.gnu.org/onlinedocs/gcc/Common-Type-Attributes.html
#define ML_TYPE_ALIGNED(x)				__attribute__((aligned (x)))
#define ML_TYPE_PACKED					__attribute__((packed))
#define ML_TYPE_MAY_ALIAS				__attribute__((may_alias))

// Definitions for variable attributes
// See: https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html
#define ML_VAR_CLEANUP(fn)				__attribute__((cleanup (fn)))

// Definitions for general purpose attributes
#define ML_UNUSED						__attribute__((unused))


/* Automatic variable type: actual variable type is
 * determined by initializer (variable must be initialized) */
#define var								__auto_type

/* Automatic cleanup type: variables of type ExampleType will
 * be automatically cleared when out of scope, using a function
 * named ml_cleanup_ExampleType (that must be implemented).
 * Here's how to use it:
 *
 *  void ml_cleanup_MlGraph (MlGraph ** p) {
 *    if (p != NULL)
 *      ml_graph_destroy (*p);
 *  }
 *
 *  auto(MlGraph) g = ml_graph_new (...);
 *
 * Now variable g will be automatically destroyed when out of scope. */
#define auto(type)						__attribute__((cleanup (ml_cleanup_##type))) type*


// branch prediction
#define if_likely(x)					if (__builtin_expect (!!(x), 1))
#define if_unlikely(x)					if (__builtin_expect (!!(x), 0))


ML_HEADER_END
#endif /* ML_HEADER__CORE_TYPES_H__INCLUDED */
