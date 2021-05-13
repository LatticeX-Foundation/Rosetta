#include "net_helper.h"

#include <vector>
#include <thread>
#include <cassert>
#include <mutex>
using namespace std;

/*
simulate MPC random broadcasting
*/

int parties = 4;
int send_party = 2;
void run_case(int party) {
  vector<string> ips;
  for (int i = 0; i < parties; i++) {
    ips.push_back("127.0.0.1");
  }

  size_t size = 1000;
  vector<int64_t> vi64_send;
  rand_vec(vi64_send, size);

  shared_ptr<IO> io = make_shared<IO>(parties, party, 1, 6666, ips);
  io->set_server_cert("certs/server-nopass.cert");
  io->set_server_prikey("certs/server-prikey");
  io->init();
  io->sync();
  io->clear_statistics();

  ////////////////////////// BEGIN
  if (party == send_party) {
    io->broadcast(vi64_send, size);
  } else {
    vector<int64_t> vi64_recv(size);
    io->recv(send_party, vi64_recv, size);
    assert(vi64_recv.size() == vi64_send.size());
    for (int i = 0; i < size; i += size / 10) {
      //assert(vi64_recv[i] == vi64_send[i]);
    }
  }
  ////////////////////////// END

  io->statistics("broadcasting:");
  io->sync();
  sleep(1);
  cout << "PARTY: " << party << " ..............done! duang~" << endl;
}

int main1(int argc, char* argv[]) {
  parties = 4;
  send_party = 2;
  std::cout << "parties:" << parties << ", send_party id:" << send_party << std::endl;

  int party = atoi(argv[1]);
  run_case(party);
  cout << "end" << endl;
  return 0;
}

int main(int argc, char* argv[]) {
  srand(time(NULL));
  parties = rand() % 8 + 3; // [3,10]
  // parties = 4;
  send_party = rand() % parties;
  std::cout << "parties:" << parties << ", send_party id:" << send_party << std::endl;

  vector<thread> threads(parties);
  for (int i = 0; i < parties; i++) {
    threads[i] = thread(run_case, i);
  }
  for (int i = 0; i < parties; i++) {
    threads[i].join();
  }

  return 0;
}
