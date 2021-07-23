#include "emp-zk/emp-zk-bool/cheat_record.h"

vector<string> CheatRecord::message;
void CheatRecord::put(const string &s) {
	message.push_back(s);
}

