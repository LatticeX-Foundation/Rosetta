#include "snn.h"

namespace rosetta {
namespace mpc {
namespace debug {

/**
 * This function will debug-output each node's send and receive bytes and counts \n
 * of all basic ops defined in opsets.h. \n
 * 
 * The results will be written into a .csv file.\n
 * 
 * 1. unary op
 * 2. binary op
 * 3. matmul
 */
void testEachOpComm() {
  //! @todo
  cout << "BEGIN" << endl;
  cout << "END" << endl;
}

} // namespace debug
} // namespace mpc
} // namespace rosetta
