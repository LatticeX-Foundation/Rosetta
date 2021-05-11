#include "net_helper.h"

#include <vector>
#include <cassert>
#include <thread>
#include <mutex>
using namespace std;

/*
simulate MPC (3PC) operations in M (3) threads as M (3) nodes
See unit tests
*/
int parties = 3;

mutex mtx;
template <typename T>
static void print_vector(vector<T>& v, int size) {
  if (size > v.size())
    size = v.size();
  unique_lock<mutex> lck(mtx);
  cout << endl << "v:";
  for (int i = 0; i < size; i++) {
    cout << v[i] << " ";
  }
  cout << endl;
}
static void print_string(string& s) {
  unique_lock<mutex> lck(mtx);
  cout << endl << "s:" << s << endl;
}

void run_case(int party) {
  vector<string> ips;
  for (int i = 0; i < parties; i++) {
    ips.push_back("127.0.0.1");
  }

  size_t size = 1000;
  vector<int64_t> vi64_send;
  rand_vec(vi64_send, size);

  int ids = 3;
  shared_ptr<IO> io = make_shared<IO>(parties, party, ids, 4444, ips);
  io->set_server_cert("certs/server-nopass.cert");
  io->set_server_prikey("certs/server-prikey");
  io->init();
  io->sync();
  cout << "PARTY: " << party << " ..............begin!" << endl;

  auto xf = [&](int id) {
    switch (party) {
      case 0: {
        // 1
        string msg1("1234567890");
        io->send(1, msg1.data(), 10, id);
        break;
      }
      case 1: {
        // 1
        string msg1;
        msg1.resize(11);
        io->recv(0, (char*)msg1.data(), 10, id);
        print_string(msg1);
        break;
      }
      case 2: {
        break;
      }
      default:
        break;
    }
  };

  vector<thread> threads(ids);
  for (int i = 0; i < ids; i++) {
    threads[i] = thread(xf, i);
  }
  for (int i = 0; i < ids; i++) {
    threads[i].join();
  }

  cout << "PARTY: " << party << " ..............begin --!" << endl;
  io->sync();
  sleep(rand() % 3);
  cout << "PARTY: " << party << " ..............done! duang~" << endl;
}

int main1(int argc, char* argv[]) {
  int party = atoi(argv[1]);
  run_case(party);
  cout << "end" << endl;
  return 0;
}

int main(int argc, char* argv[]) {
  vector<thread> threads(parties);
  for (int i = 0; i < parties; i++) {
    threads[i] = thread(run_case, i);
  }
  for (int i = 0; i < parties; i++) {
    threads[i].join();
  }

  return 0;
}
