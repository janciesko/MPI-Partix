/*
//@HEADER
// ************************************************************************
//
//                        Partix 1.0
//              Copyright Type Date and Name (2022)
//
// Questions? Contact Jan Ciesko (jciesko@sandia.gov)
//
// ************************************************************************
//@HEADER
*/

#ifndef __PARTIX_TYPES_H__
#define __PARTIX_TYPES_H__

#if defined (OMP)
#include <omp_types.h>
#endif

#if defined (PTHREADS)
#include <pthreads_types.h>
#endif

#if defined (ARGOBOTS)
#include <argobots_types.h>
#endif

#if defined (QTHREADS)
#include <qthreads_types.h>
#endif


#endif /* __PARTIX_TYPES_H__ */
