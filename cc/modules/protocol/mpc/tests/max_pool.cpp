#include "snn.h"

namespace rosetta {
namespace mpc {
namespace debug {

void debugMax() {
  auto op = GetMpcOpDefault(Max);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  size_t rows = 3;
  size_t columns = 4;
  vector<mpc_t> a(rows * columns, 0);

  if (partyNum == PARTY_A || partyNum == PARTY_C) {
    // clang-format off
    a[0] = 0;     a[1] = 1;    a[2] = 0;     a[3] = 4;
    a[4] = 5;     a[5] = 3;    a[6] = 10;    a[7] = 6;
    a[8] = 11;    a[9] = 9;    a[10] = 3;    a[11] = 7;
    // clang-format on
  }

  vector<mpc_t> max(rows), maxIndex(rows);
  op->Run(a, max, maxIndex, rows, columns);

  if (PRIMARY) {
    oprec->Run(a, rows * columns, "a"); // 0 1 0 4 5 ...
    oprec->Run(max, rows, "max"); // 4 10 11
    oprec->Run(maxIndex, rows, "maxIndex"); // 3 2 0
  }
}

void debugMaxIndex() {
  auto op = GetMpcOpDefault(MaxIndex);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void testMaxPool(size_t p_range, size_t q_range, size_t px, size_t py, size_t D, size_t iter) {
  auto op = GetMpcOpDefault(Max);

  size_t B = 32;
  size_t size_x = p_range * q_range * D * B;

  vector<mpc_t> y(size_x, 0);
  vector<mpc_t> maxPoolShaped(size_x, 0);
  vector<mpc_t> act(size_x / (px * py), 0);
  vector<mpc_t> maxIndex(size_x / (px * py), 0);

  for (size_t i = 0; i < iter; ++i) {
    maxPoolReshape(y, maxPoolShaped, p_range, q_range, D, B, py, px, py, px);

    if (STANDALONE) {
      size_t size = (size_x / (px * py)) * (px * py);
      vector<mpc_t> diff(size);

      for (size_t i = 0; i < (size_x / (px * py)); ++i) {
        act[i] = maxPoolShaped[i * (px * py)];
        maxIndex[i] = 0;
      }

      for (size_t i = 1; i < (px * py); ++i)
        for (size_t j = 0; j < (size_x / (px * py)); ++j) {
          if (maxPoolShaped[j * (px * py) + i] > act[j]) {
            act[j] = maxPoolShaped[j * (px * py) + i];
            maxIndex[j] = i;
          }
        }
    }

    if (MPC)
      op->Run(maxPoolShaped, act, maxIndex, size_x / (px * py), px * py);
  }
}

void testMaxPoolDerivative(
  size_t p_range, size_t q_range, size_t px, size_t py, size_t D, size_t iter) {
  auto op = GetMpcOpDefault(MaxIndex);
  auto opdot = GetMpcOpWithKey(DotProduct, op->msg_id());

  size_t B = 32;
  size_t alpha_range = p_range / py;
  size_t beta_range = q_range / px;
  size_t size_y = (p_range * q_range * D * B);
  vector<mpc_t> deltaMaxPool(size_y, 0);
  vector<mpc_t> deltas(size_y / (px * py), 0);
  vector<mpc_t> maxIndex(size_y / (px * py), 0);

  size_t size_delta = alpha_range * beta_range * D * B;
  vector<mpc_t> thatMatrixTemp(size_y, 0), thatMatrix(size_y, 0);

  for (size_t i = 0; i < iter; ++i) {
    if (STANDALONE)
      for (size_t i = 0; i < size_delta; ++i)
        thatMatrixTemp[i * px * py + maxIndex[i]] = 1;

    if (MPC)
      op->Run(thatMatrixTemp, maxIndex, size_delta, px * py);

    // Reshape thatMatrix
    size_t repeat_size = D * B;
    size_t alpha_offset, beta_offset, alpha, beta;
    for (size_t r = 0; r < repeat_size; ++r) {
      size_t size_temp = p_range * q_range;
      for (size_t i = 0; i < size_temp; ++i) {
        alpha = (i / (px * py * beta_range));
        beta = (i / (px * py)) % beta_range;
        alpha_offset = (i % (px * py)) / px;
        beta_offset = (i % py);
        thatMatrix
          [((py * alpha + alpha_offset) * q_range) + (px * beta + beta_offset) + r * size_temp] =
            thatMatrixTemp[r * size_temp + i];
      }
    }

    // Replicate delta martix appropriately
    vector<mpc_t> largerDelta(size_y, 0);
    size_t index_larger, index_smaller;
    for (size_t r = 0; r < repeat_size; ++r) {
      size_t size_temp = p_range * q_range;
      for (size_t i = 0; i < size_temp; ++i) {
        index_smaller =
          r * size_temp / (px * py) + (i / (q_range * py)) * beta_range + ((i % q_range) / px);
        index_larger = r * size_temp + (i / q_range) * q_range + (i % q_range);
        largerDelta[index_larger] = deltas[index_smaller];
      }
    }

    if (STANDALONE)
      for (size_t i = 0; i < size_y; ++i)
        deltaMaxPool[i] = largerDelta[i] * thatMatrix[i];

    if (MPC)
      opdot->Run(largerDelta, thatMatrix, deltaMaxPool, size_y);
  }
}
} // namespace debug
} // namespace mpc
} // namespace rosetta