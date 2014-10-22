#include "map_plus_primitives.h"

namespace tajo {

void map_plus_int4_vec_int4_vec(float* res, VecRowBlock* vec, int leftId, int rightId, int* sel, int selected) {

  float* lVec = vec->GetVector<float>(leftId);
  float* rVec = vec->GetVector<float>(rightId);

  if (sel == nullptr) {

    int mainLoopCnt = (vec->VectorSize() / 4) * 4;

    int i = 0;
    for (; i < mainLoopCnt; i += 4) {
    #ifdef __SSE4_2__
      __m128 a4 = _mm_loadu_ps(lVec + i);
      __m128 b4 = _mm_loadu_ps(rVec + i);
      __m128 sum = _mm_add_ps(a4, b4);
      _mm_storeu_ps(res + i, sum);
    #else
      res[i] = lVec[i] + rVec[i];
      res[i+1] = lVec[i+1] + rVec[i+1];
      res[i+2] = lVec[i+2] + rVec[i+2];
      res[i+3] = lVec[i+3] + rVec[i+3];
    #endif
    }

    // The epilogue to cover the last (vectorSize() % 4) elements.
    for (; i < vec->VectorSize(); i++) {
      res[i] = lVec[i] + rVec[i];
    }

  } else {

    int mainLoopCnt = (selected / 4) * 4;
    int i = 0;
    for (; i < mainLoopCnt; i+=4) {
      res[i] = lVec[sel[i]] + rVec[sel[i]];
      res[i+1] = lVec[sel[i+1]] + rVec[sel[i+1]];
      res[i+2] = lVec[sel[i+2]] + rVec[sel[i+2]];
      res[i+3] = lVec[sel[i+3]] + rVec[sel[i+3]];
    }

    // The epilogue to cover the last (vectorSize() % 4) elements.
    for (; i < selected; i++) {
      res[i] = lVec[i] + rVec[i];
    }
  }
}

} // namespace

