/*
 * sort.c
 *
 *  Created on: Jun 6, 2023
 *      Author: panasyuk
 */

#include <arm_math.h>
#include "sort_and_median.h"

static int MaxSizeInsertionSortIsBetter = 30; // we have to do a study and select this number. OK, 30 is about optimal,

void merge_sort(float *p, int N) {
  if(N <= MaxSizeInsertionSortIsBetter) insertion_sort(p, N);
  else {
    int N1 = N/2, N2 = N - N1;
    float *pa, *pb;
    merge_sort(pa = p,N1);
    merge_sort(pb = p + N1, N2);
    // combine
    float comb[N], *pc = comb;
    while(pc < comb + N) {
      *(pc++) = *pa < *pb ? *(pa++) : *(pb++);
      if(pa == p + N1) { // we ran out of A, so just copy the remnant of B
        arm_copy_f32(pb, pc, p + N - pb);
        break;
      }
      if(pb == p + N) { // we ran out of B, so just copy the remnant of A
        arm_copy_f32(pa, pc, p + N1 - pa);
        break;
      }
    }
    // copy back to input vector
    arm_copy_f32(comb, p, N);
  }
} // merge_sort

