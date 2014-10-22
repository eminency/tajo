#ifndef TAJO_VECTOR_MAP_PLUS_PRIMITIVES_
#define TAJO_VECTOR_MAP_PLUS_PRIMITIVES_

#include "vectors.h"
#include "util/simd-util.h"

namespace tajo {

void map_plus_int4_vec_int4_vec(float *res, VecRowBlock *vec, int leftId, int rightId, int *sel, int selected);

} // namespace tajo

#endif