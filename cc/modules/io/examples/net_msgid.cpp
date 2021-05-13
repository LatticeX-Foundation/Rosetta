#include "net_helper.h"

#include <vector>
#include <cassert>
#include <thread>
#include <mutex>
using namespace std;

static const int parties = 3;

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

int N2 = 100;
int us2 = 10000;
static void run_p0(shared_ptr<IO>& io) {
  // 1
  string msg1("1234567890");
  io->send(1, msg1.data(), 10);

  // 2
  for (int i = 0; i < N2; i++) {
    vector<int64_t> vi64(10);
    io->recv(1, vi64, vi64.size());
    print_vector(vi64, 10);

    vector<int64_t> vu64(10);
    io->recv(1, vu64, vu64.size());
    print_vector(vu64, 10);
    usleep(us2);
  }

  // 3
  {
    size_t size = 100;
    vector<uint64_t> vu64(size);
    io->recv(2, vu64, vu64.size());
    print_vector(vu64, 10);
  }
}
static void run_p1(shared_ptr<IO>& io) {
  // 1
  string msg1;
  msg1.resize(11);
  io->recv(0, (char*)msg1.data(), 10);
  print_string(msg1);

  // 2
  for (int i = 0; i < N2; i++) {
    vector<int64_t> vi64 = {-4L, -3L, -2L, -1L, 0L, 1L, 2L, 3L, 4L, 5L};
    io->send(0, vi64, vi64.size());
    vector<uint64_t> vu64 = {-4UL, -3UL, -2UL, -1UL, 0UL, 1UL, 2UL, 3UL, 4UL, 5UL};
    io->send(0, vu64, vu64.size());
    usleep(us2);
  }

  // 3
  {
    size_t size = 100;
    vector<uint64_t> vu64(size);
    io->recv(2, vu64, vu64.size());
    print_vector(vu64, 10);
  }
}
static void run_p2(shared_ptr<IO>& io) {
  // 3
  size_t size = 100;
  vector<uint64_t> vu64(size);
  for (int i = 0; i < size; i++) {
    vu64[i] = (UINT64_MAX / size) * i;
  }
  io->broadcast(vu64, size);
}

static void run_px(int party) {
  vector<string> ips;
  for (int i = 0; i < parties; i++) {
    ips.push_back("127.0.0.1");
  }

  // init io
  shared_ptr<ParallelIO> io = make_shared<ParallelIO>(parties, party, 1, 7777, ips);
  io->set_server_cert("certs/server-nopass.cert");
  io->set_server_prikey("certs/server-prikey");
  io->init();

  int parallel_nums = 10;
  auto xf = [&](int id) {
    //msg_id_t msg_id(std::to_string(id) + "111111111111111");
    msg_id_t msg_id(std::to_string(id) + "+=-_");
    cout << "msgid:" << msg_id << endl;
    io->sync_with(msg_id);
    int size = 1000;
    switch (party) {
      case 0: {
        // 1
        string msg1("i'am a message..");
        msg1.resize(size);
        io->send(1, msg1.data(), size, msg_id);
        break;
      }
      case 1: {
        // 1
        string msg1;
        msg1.resize(size);
        io->recv(0, (char*)msg1.data(), size, msg_id);
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

  vector<thread> threads(parallel_nums);
  for (int i = 0; i < parallel_nums; i++) {
    threads[i] = thread(xf, i);
  }
  for (int i = 0; i < parallel_nums; i++) {
    threads[i].join();
  }

  sleep(5);
  io->sync();
  sleep(rand() % 5);
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
