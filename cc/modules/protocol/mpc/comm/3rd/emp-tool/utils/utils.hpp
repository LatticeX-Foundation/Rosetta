template<class... Ts>
void run_function(void *function, const Ts&... args) {	
	reinterpret_cast<void(*)(Ts...)>(function)(args...);
}

template<typename T>
void inline delete_array_null(T * ptr){
	if(ptr != nullptr) {
		delete[] ptr;
		ptr = nullptr;
	}
}

inline time_point<high_resolution_clock> clock_start() { 
	return high_resolution_clock::now();
}

inline double time_from(const time_point<high_resolution_clock>& s) {
	return std::chrono::duration_cast<std::chrono::microseconds>(high_resolution_clock::now() - s).count();
}

inline void error(const char * s, int line, const char * file) {
	fprintf(stderr, s, "\n");
	if(file != nullptr) {
		fprintf(stderr, "at %d, %s\n", line, file);
	}
	exit(1);
}

inline void parse_party_and_port(char ** arg, int argc, int * party, int * port) {
	if (argc == 1)
		error("ERROR: argc = 1, need two argsm party ID {1,2} and port.");
	*party = atoi (arg[1]);
	*port = atoi (arg[2]);
}

template<typename t>
inline t bool_to_int(const bool * data, size_t len) {
	if (len != 0) len = (len > sizeof(t)*8 ? sizeof(t)*8 : len);
	else len = sizeof(t)*8;
	t res = 0;
	for(size_t i = 0; i < len-1; ++i) {
		if(data[i])
			res |= (1LL<<i);
	}
	if(data[len-1]) return -1*res;
	else return res;
}

inline uint64_t bool_to64(const bool * data) {
	uint64_t res = 0;
	for(int i = 0; i < 64; ++i) {
		if(data[i])
			res |= (1ULL<<i);
	}
	return res;
}
inline block bool_to128(const bool * data) {
	return makeBlock(bool_to64(data+64), bool_to64(data));
}

inline void int64_to_bool(bool * data, uint64_t input, int length) {
	for (int i = 0; i < length; ++i) {
		data[i] = (input & 1)==1;
		input >>= 1;
	}
}
