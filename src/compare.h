/*
 * compare.h
 *
 *  Created on: Sep 23, 2013
 *      Author: zhxfl
 */

#ifndef COMPARE_H_
#define COMPARE_H_
#include "MarsInc.h"
#include "global.h"

//--------------------------------------------------------
//No Sort in this application
//--------------------------------------------------------
__device__ int compare(const void *d_a, int len_a, const void *d_b, int len_b);

#endif /* COMPARE_H_ */
