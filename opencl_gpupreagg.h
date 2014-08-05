/*
 * opencl_gpupreagg.h
 *
 * Preprocess of aggregate using GPU acceleration, to reduce number of
 * rows to be processed by CPU; including the Sort reduction.
 * --
 * Copyright 2011-2014 (C) KaiGai Kohei <kaigai@kaigai.gr.jp>
 * Copyright 2014 (C) The PG-Strom Development Team
 *
 * This software is an extension of PostgreSQL; You can use, copy,
 * modify or distribute it under the terms of 'LICENSE' included
 * within this package.
 */
#ifndef OPENCL_GPUPREAGG_H
#define OPENCL_GPUPREAGG_H


typedef struct
{
	cl_int			status;	/* result of kernel execution */
	cl_int			buf_nindex;	/* length of rindex/gindex */
	kern_paramuf	kparams;
	/*
	 * NOTE: In the kernel space, buffer for rindex and gindex shall
	 * be allocated, next to the kparams. Its length is nindex that
	 * is the least power of 2, but larger than or equal to the original
	 * nitems of kern_data_store. Use get_next_log2() to calculate.
	 */
} kern_gpupreagg;

/* macro definitions to reference packed values */
#define KERN_GPUPREAGG_PARAMBUF(kgpreagg)					\
	((__global kern_parambuf *)(&(kgpreagg)->kparams))
#define KERN_GPUPREAGG_PARAMBUF_LENGTH(kgpreagg)			\
	(KERN_GPUPREAGG_PARAMBUF(kgpreagg)->length)
#define KERN_GPUPREAGG_ROW_INDEX(kgpreagg)					\
	((__global cl_int *)((__global char *)(kgpreagg) +		\
			STROMALIGN(offsetof(kern_gpupreagg, kparams) +	\
					   KERN_GPUPREAGG_PARAMBUF_LENGTH(kgpreagg))))
#define KERN_GPUPREAGG_GROUP_INDEX(kgpreagg)				\
	(KERN_GPUPREAGG_ROW_INDEX(kgpreagg) + (kgpreagg)->nindex)
#define KERN_GPUPREAGG_HOST_LENGTH(kgpreagg)				\
	((uintptr_t)KERN_GPUPREAGG_ROW_INDEX(kgpreagg) -		\
	 (uintptr_t)(kgpreagg))
#define KERN_GPUPREAGG_DEVICE_LENGTH(kgpreagg)				\
	((uintptr_t)(KERN_GPUPREAGG_ROW_INDEX(kgpreagg) +		\
				 2 * (kgpreagg)->nindex) - (uintptr_t)(kgpreagg))




/*
 * pagg_data - data structure to store a running total, to be generated
 * by PG-Strom on the fly, according to the query requirement.
 * typically, it contains multiple 8bytes integer/float fields in addition
 * to the number of rows. gpupreagg_aggcomp() can aggregate two pagg_data
 * into one datum.
 */
struct pagg_data;

/*
 * comparison function - to be generated by PG-Strom on the fly
 *
 * It compares two records indexed by 'x_index' and 'y_index' on the supplied
 * kern_data_store, then returns -1 if record[X] is less than record[Y],
 * 0 if record[X] is equivalent to record[Y], or 1 if record[X] is greater
 * than record[Y].
 */
static cl_int
gpupreagg_keycomp(__private cl_int *errcode,
				  __global kern_data_store *kds,
				  __global kern_toastbuf *ktoast,
				  size_t x_index,
				  size_t y_index);
/*
 * calculation function - to be generated by PG-Strom on the fly
 *
 * It updates the supplied 'accum' value by 'newval' value. Both of data
 * structure is expected to be on the local memory.
 */
static void
gpupreagg_aggcalc(__private cl_int *errcode,
				  __local struct pagg_data *accum,
				  __local struct pagg_data *newval);

/*
 * load the data from kern_data_store to pagg_data structure
 */
static void
gpupreagg_data_load(__private cl_int *errcode,
					__local struct pagg_data *values,
					__global kern_data_store *kds,
					__global kern_toastbuf *ktoast,
					size_t kds_index);

/*
 * store the data from pagg_data structure to kern_data_store
 */
static void
gpupreagg_data_store(__private cl_int *errcode,
					 __local struct pagg_data *values,
					 __global kern_data_store *kds,
					 __global kern_toastbuf *ktoast,
					 size_t kds_index);
/*
 * translate a kern_data_store (input) into an output form
 */
static void
gpupreagg_projection(__private cl_int *errcode,
					 __global kern_data_store *kds_in,
					 __global kern_data_store *kds_out,
					 __global kern_toastbuf *ktoast,
					 size_t kds_index);

/*
 * gpupreagg_preparation - It translaes a usual kern_data_store (that
 * reflects outer relation's tupdesc) into a form of running results
 * of gpupreagg.
 */
__kernel void
gpupreagg_preparation(__global kern_gpupreagg *kgpreagg,
					  __global kern_data_store *kds_in,
					  __global kern_data_store *kds_out,
					  __global kern_toastbuf *ktoast,
					  __local void *local_workbuf)
{
	__global kern_parambuf *kparams = KERN_GPUPREAGG_PARAMBUF(kgpreagg);
	cl_int		errcode = StromError_Success;

	if (get_global_id(0) < kds_in->nitems)
		gpupreagg_projection(&errcode,
							 kds_in, kds_out, ktoast,
							 get_global_id(0));

	kern_writeback_error_status(&kgpreagg->status, errcode, local_workbuf);
}

/*
 * gpupreagg_bitonic_local
 *
 * It tries to apply each steps of bitonic-sorting until its unitsize
 * reaches the workgroup-size (that is expected to power of 2).
 */
__kernel void
gpupreagg_bitonic_local(__global kern_gpupreagg *kgpreagg,
						__global kern_data_store *kds,
						__global kern_toastbuf *ktoast,
						__local void *local_workbuf)
{
	__global kern_parambuf *kparams = KERN_GPUPREAGG_PARAMBUF(kgpreagg);
	cl_int		errcode = StromError_Success;



	kern_writeback_error_status(&kgpreagg->status, errcode, local_workbuf);
}

/*
 * gpupreagg_bitonic_step
 *
 * It tries to apply individual steps of bitonic-sorting for each step,
 * but does not have restriction of workgroup size. The host code has to
 * control synchronization of each step not to overrun.
 */
__kernel void
gpupreagg_bitonic_step(__global kern_gpupreagg *kgpreagg,
					   cl_int bitonic_unitsz,
					   __global kern_data_store *kds,
					   __global kern_toastbuf *ktoast,
					   __local void *local_workbuf)
{
	__global kern_parambuf *kparams = KERN_GPUPREAGG_PARAMBUF(kgpreagg);
	cl_bool		reversing = (bitonic_unitsz < 0 ? true : false);
	size_t		unitsz = (bitonic_unitsz < 0
                          ? 1U << -bitonic_unitsz
                          : 1U << bitonic_unitsz);
	cl_int		errcode = StromError_Success;


	kern_writeback_error_status(&kgpreagg->status, errcode, local_workbuf);
}

/*
 * gpupreagg_bitonic_merge
 *
 * It handles the merging step of bitonic-sorting if unitsize becomes less
 * than or equal to the workgroup size.
 */
__kernel void
gpupreagg_bitonic_merge(__global kern_gpupreagg *kgpreagg,
						__global kern_data_store *kds,
						__global kern_toastbuf *ktoast,
						__local void *local_workbuf)
{
	__global kern_parambuf *kparams = KERN_GPUPREAGG_PARAMBUF(kgpreagg);

	kern_writeback_error_status(&kgpreagg->status, errcode, local_workbuf);
}





/*
 * gpupreagg_main - run aggregation main
 */
__kernel void
gpupreagg_main(__global kern_gpupreagg *kgpreagg,
			   __global kern_data_store *kds_src,
			   __global kern_data_store *kds_dst,
			   __global kern_toastbuf *ktoast,
			   __local void *local_workbuf)
{
	__global kern_parambuf *kparams = KERN_GPUPREAGG_PARAMBUF(kgpreagg);
	__global cl_int *rindex = KERN_GPUPREAGG_ROW_INDEX(kgpreagg);
	__global cl_int *gindex = KERN_GPUPREAGG_GROUP_INDEX(kgpreagg);





}

/*
 * gpupreagg_check_next - decision making whether we need to run next
 * reduction step actually.
 */
__kernel void
gpupreagg_check_next(__global kern_gpupreagg *kgpreagg,
					 __global kern_data_store *kds_old,
					 __global kern_data_store *kds_new)
{



}

#endif