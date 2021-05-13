#include "net_helper.h"

#include <vector>
#include <cassert>
#include <thread>
#include <mutex>
using namespace std;

static const int parties = 3;

static void run_px(int party) {
  vector<string> ips;
  for (int i = 0; i < parties; i++) {
    ips.push_back("127.0.0.1");
  }

  // init io
  shared_ptr<ParallelIO> io = make_shared<ParallelIO>(parties, party, 1, 11111, ips);
  io->set_server_cert("certs/server-nopass.cert");
  io->set_server_prikey("certs/server-prikey");
  io->init();
  io->sync();

  switch (party) {
    case 0: {
      sleep(rand() % 3);
      io->sync();
      break;
    }
    case 1: {
      sleep(rand() % 5);
      io->sync();
      break;
    }
    case 2: {
      sleep(rand() % 7);
      io->sync();
      break;
    }
    default:
      break;
  }
  cout << "sync 1 ok" << endl;
  io->sync();
  sleep(rand() % 3);
}

int main1(int argc, char* argv[]) {
  int party = atoi(argv[1]);
  run_px(party);
  cout << "end" << endl;
  return 0;
}

int main(int argc, char* argv[]) {
  vector<thread> threads(parties);
  for (int i = 0; i < parties; i++) {
    threads[i] = thread(run_px, i);
  }
  for (int i = 0; i < parties; i++) {
    threads[i].join();
  }
  cout << "end" << endl;
  return 0;
}
