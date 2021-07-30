#include "helix__test.h"
#include <unordered_map>

using namespace std;

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
    vector<double> XX = {3.0, 1.0, 2.0,  3.4, -2.1};
    vector<int> power = {7, 10, 8, 6, 5};

    vector<Share> shareX;
    hi->Input(node_id_2, XX, shareX);
    vector<Share> shareZ(shareX.size());
    Share tmp_v;

    // pre-case
    Share tmp_in = shareX[0];
    int tmp_p = power[0];
    hi->PowV2(tmp_in, tmp_p, tmp_v);
    shareZ[0] = tmp_v;
    hi->RevealAndPrint(shareZ, "power single shareZ:");
    hi->PowV2(shareX, power, shareZ);
    hi->RevealAndPrint(shareZ, "power vector shareZ:");

    vector<double> XXX = {0.1, 1.0, 2.0, 2.718, 5.0};
    hi->Input(node_id_2, XXX, shareX);
    hi->LogV2(shareX, shareZ);
    hi->RevealAndPrint(shareZ, "LogV2 shareZ:");
    double ori = 0.05;
    mpc_t a = FloatToMpcType(ori, hi->context_->FLOAT_PRECISION);
    double aa = MpcTypeToFloat(a, hi->context_->FLOAT_PRECISION);
    cout << "CONF:" << hi->GetMpcContext()->FLOAT_PRECISION << endl;
    cout << ori << ": " << to_readable_dec(a) << "<=>" << aa << endl;
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);