#ifndef EMP_OT_H__
#define EMP_OT_H__
#include <emp-tool/emp-tool.h>

namespace emp {

template<typename T>
class OT { public:
	virtual void send(const block* data0, const block* data1, int64_t length) = 0;
	virtual void recv(block* data, const bool* b, int64_t length)  = 0;
	virtual ~OT() {
	}
};

}
#endif
