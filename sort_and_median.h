/*
 * sort_and_median.h
 *
 *  Created on: Jun 6, 2023
 *      Author: panasyuk
 */

#ifndef SORT_AND_MEDIAN_H_
#define SORT_AND_MEDIAN_H_

/**
 * insertion_sort is O(N^2), it is not effective for big number of elements, but very good for small numbers as we have here.
 * I think the boundary were it gets slower than merge_sort is about 30
 * Sorts N elements array spaced by STEP in place
 * @param p - pointer to the first element
 * @param step - step between elements in floats
 * @param N - number of elements
 */
static void inline insertion_sort_step(float * const p, int N, int step) {
  for(float *p_cur = p + step; p_cur < p + N*step; p_cur += step)
    if(*p_cur < *(p_cur-step)) {
      float x = *p_cur;
      *p_cur = *(p_cur-step);
      float *p_insert = p_cur - 2*step;
      for(; p_insert >= p; p_insert -= step)
        if(*p_insert > x) *(p_insert + step) = *p_insert; else break;
      *(p_insert + step) = x;
    }
}  // insertion_sort

static void inline insertion_sort(float *p, int N) { insertion_sort_step(p,N,1); }

/**
 * sort array in place
 * merge sort in O(N*log(N)) in any case, very fast for big vectors, but slow for small because it is recursive. So
 * I use insertion_sort when vectors are small. The boundary between small and big is "MaxSizeInsertionSortIsBetter" defined in sort.c
 * @note Current implementation keeps copy of the input array in stack  which may overwhelm it
 */
void merge_sort(float *p, int N);

/**
 * calculates median of a N elements vector starting with P and spaced by STEP. Sorts the vector in place.
 * It is a macro so that all the indexing is done by compiler instead of real-time.
 * Handles even and odd N.
 */
#define MEDIAN_STEP(p,STEP,N) ((N) % 2 ? (p)[(STEP)*((N) - 1) / 2] : \
    ((p)[(STEP) * ((N) / 2)] + (p)[(STEP)*((N) / 2 - 1)])/2)
#define MEDIAN(p,N) MEDIAN_STEP(p,1,N)
#define SORT_AND_MEDIAN_STEP(p,STEP,N) (insertion_sort_step(p, N, STEP), MEDIAN_STEP(p,STEP,N))
#define SORT_AND_MEDIAN(p,N) (merge_sort(p, N), MEDIAN(p,N))

#if 0
#define MEDIAN3(a_1, a0, a1) \
  (a_1 < a0?\
     a_1 < a1?\
       a0 < a1?a0:a1:\
       a_1:\
     a_1 > a1?\
       a0 < a1?a1:a0:\
       a_1)
#endif

#endif /* SORT_AND_MEDIAN_H_ */
