/*
 * common_cpp.cpp
 *
 * Created: 11/11/2013 3:02:38 PM
 *  Author: panasyuk
 */ 
// Vector.h

static void default_bad_index_func() { while(1); }

void (*bad_index_func)() = default_bad_index_func;
