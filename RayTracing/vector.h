//
//  vector.h
//  Raycasting
//  CS 430
//  Junzhe Wang
//

#ifndef vector_h
#define vector_h

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "csv.h"

typedef double Vector[3]; // array of double values to represent

double sqr(double v);
void Vector_zero(Vector vector);
void Vector_copy(Vector in, Vector out);
void normalize(double *v);
double Vector_len(Vector a);
void Vector_add(Vector a, Vector b, Vector c);
void Vector_sub(Vector a, Vector b, Vector c);
void Vector_scale(Vector a, double s, Vector b);
double Vector_dot(Vector a, Vector b);
void Vector_corss(Vector a, Vector b, Vector c);
void Vector_reflect(Vector v, Vector n, Vector v_r);

#endif /* vector_h */
