/*
 * opencl_atomic.h
 *
 * Atomic operations for misc data types
 * --
 * Copyright 2011-2014 (C) KaiGai Kohei <kaigai@kaigai.gr.jp>
 * Copyright 2014 (C) The PG-Strom Development Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef OPENCL_ATOMIC_H
#define OPENCL_ATOMIC_H

/*
 * atomic_add - float, double and numeric
 */
static float
atomic_add(volatile __local float *ptr, float value)
{
	uint	oldval = to_uint(*ptr);
	uint	newval = to_uint(to_float(oldval) + value);
	uint	curval;

	while ((curval = atomic_cmpxchg((__global uint *)ptr,
									oldval, newval)) != oldval)
	{
		oldval = curval;
		newval = to_uint(to_float(oldval) + value);
	}
	return to_float(oldval);
}

static float
atomic_add(volatile __global float *ptr, float value)
{
	uint	oldval = to_uint(*ptr);
	uint	newval = to_uint(to_float(oldval) + value);
	uint	curval;

	while ((curval = atomic_cmpxchg((__global uint *)ptr,
									oldval, newval)) != oldval)
	{
		oldval = curval;
		newval = to_uint(to_float(oldval) + value);
	}
	return to_float(oldval);
}

static double
atomic_add(volatile __local double *ptr, double value)
{
	ulong	oldval = to_ulong(*ptr);
	ulong	newval = to_ulong(to_double(oldval) + value);
	ulong	curval;

	while ((curval = atom_cmpxchg((__local ulong *)ptr,
								  oldval, newval)) != oldval)
	{
		oldval = curval;
		newval = to_ulong(to_double(oldval) + value);
	}
	return to_double(oldval);
}

static double
atomic_add(volatile __global double *ptr, double value)
{
	ulong	oldval = to_ulong(*ptr);
	ulong	newval = to_ulong(to_double(oldval) + value);
	ulong	curval;

	while ((curval = atom_cmpxchg((__global ulong *)ptr,
								  oldval, newval)) != oldval)
	{
		oldval = curval;
		newval = to_ulong(to_double(oldval) + value);
	}
	return to_double(oldval);
}

#ifdef PG_NUMERIC_TYPE_DEFINED
static pg_numeric_t
pgfn_atomic_numeric_add(volatile __global pg_numeric_t *ptr,
						pg_numeric_t addval)
{
	pg_numeric_t	oldval;
	pg_

atomic_add_numeric(volatile __global cl_ulong *ptr, cl_ulong value)
{
	pg_numeric_t	oldval;
	pg_numeric_t	addval;
	pg_numeric_t	newval;
	ulong			curval;
	int				errcode;

	addval.isnull = 
	oldval.isnull = false;
	oldval.value = *ptr;



	x.isnull = y.isnull = false;
	x.value = *ptr;
	y.value = value;
	z = pgfn_numeric_add(&errcode, x, y);

	while (hoge = atom_cmpxchg((__global ulong *)ptr,
							   





}







#endif /* PG_NUMERIC_TYPE_DEFINED */


static float
atomic_min(volatile __local float *ptr, float value)
{
	uint	oldval = to_uint(*ptr);
	uint	curval;

	while ((curval = atomic_cmpxchg((__local cl_uint *)ptr,
									oldval,
									min(to_float(oldval),
										value))) != oldval)
		oldval = curval;

	return to_float(oldval);
}

static float
atomic_min(volatile __global float *ptr, float value)
{
	uint	oldval = to_uint(*ptr);
	uint	curval;

	while ((curval = atomic_cmpxchg((__global cl_uint *)ptr,
									oldval,
									min(to_float(oldval),
										value))) != oldval)
		oldval = curval;

	return to_float(oldval);
}

static float
atomic_max(volatile __local float *ptr, float value)
{
	uint	oldval = to_uint(*ptr);
	uint	curval;

	while ((curval = atomic_cmpxchg((__local cl_uint *)ptr,
									oldval,
									max(to_float(oldval),
										value))) != oldval)
		oldval = curval;

	return to_float(oldval);
}

static float
atomic_max(volatile __global float *ptr, float value)
{
	uint	oldval = to_uint(*ptr);
	uint	curval;

	while ((curval = atomic_cmpxchg((__global cl_uint *)ptr,
									oldval,
									max(to_float(oldval),
										value))) != oldval)
		oldval = curval;

	return to_float(oldval);
}









#ifdef OPENCL_NUMERIC_H
#endif /* OPENCL_ATOMIC_H */

#endif	/* OPENCL_ATOMIC_H */
