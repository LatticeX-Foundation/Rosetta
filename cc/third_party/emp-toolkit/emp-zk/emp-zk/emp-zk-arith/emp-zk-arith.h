#ifndef EMP_ZK_ARITH_H__
#define EMP_ZK_ARITH_H__
#include "emp-zk/emp-zk-arith/zk_fp_exec.h"
#include "emp-zk/emp-zk-arith/zk_fp_exec_prover.h"
#include "emp-zk/emp-zk-arith/zk_fp_exec_verifier.h"
#include "emp-zk/emp-zk-arith/triple_auth.h"
#include "emp-zk/emp-zk-arith/ostriple.h"
#include "emp-zk/emp-zk-arith/int_fp.h"
#include "emp-zk/emp-zk-arith/conversion.h"
#include "emp-zk/emp-zk-arith/polynomial.h"

template<typename IO>
inline void setup_zk_arith(IO** ios, int threads, int party, bool enable_conversion = false) {
	if(enable_conversion) {
		if(CircuitExecution::circ_exec == nullptr) {
			error("Boolean ZK backend is not set up!\n");
		}
	}

	if(party == ALICE) {
		ZKFpExec::zk_exec = new ZKFpExecPrv<IO>(ios, threads);
		FpPolyProof<IO>::fppolyproof = new FpPolyProof<IO>(ALICE, (IO*)ios[0], ((ZKFpExecPrv<IO>*)(ZKFpExec::zk_exec))->ostriple);

		if(enable_conversion)
			EdaBits<IO>::conv = new EdaBits<IO>(ALICE, threads, ios, ((ZKFpExecPrv<IO>*)(ZKFpExec::zk_exec))->ostriple->vole);
		
	} else {
		ZKFpExec::zk_exec = new ZKFpExecVer<IO>(ios, threads);
		FpPolyProof<IO>::fppolyproof = new FpPolyProof<IO>(BOB, (IO*)ios[0], ((ZKFpExecVer<IO>*)(ZKFpExec::zk_exec))->ostriple);
		if(enable_conversion) {
			EdaBits<IO>::conv = new EdaBits<IO>(BOB, threads, ios, ((ZKFpExecVer<IO>*)(ZKFpExec::zk_exec))->ostriple->vole);
			EdaBits<IO>::conv->install_boolean(((ZKBoolCircExecVer<IO>*)CircuitExecution::circ_exec)->delta);
		}
	}
}

template<typename IO>
inline void finalize_zk_arith() {
	if (EdaBits<IO>::conv != nullptr)
		delete EdaBits<IO>::conv;
	delete FpPolyProof<IO>::fppolyproof;
	delete ZKFpExec::zk_exec;
}
#endif
