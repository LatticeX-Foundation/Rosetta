#include "snn.h"

namespace rosetta {
namespace mpc {
namespace debug {

// some usages
void usage() {
  {
    auto mop = std::make_shared<rosetta::mpc::Mul>("defaultkey");
    auto rop = std::make_shared<rosetta::mpc::Reconstruct2PC>(mop);
  }
  {
    msg_id_t key("thiskey");
    auto mop = GetMpcOpWithKey(Mul, key);
    auto rop = GetMpcOpWithKey(Reconstruct2PC, mop->msg_id());
  }
  {
    auto mop = GetMpcOpDefault(Mul);
    auto rop = GetMpcOpWithKey(Reconstruct2PC, mop->msg_id());
  }
}
} // namespace debug
} // namespace mpc
} // namespace rosetta