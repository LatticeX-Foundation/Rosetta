  // part of inner definition of snn_internal.h
  
  // from tools.hpp beg
  template <typename T>
  void gen_side_sharesModuloOdd(vector<mpc_t>& shares_1, size_t size, string r_type) {
    assert(
      ((r_type == "a_1") || (r_type == "a_2")) &&
      "invalid randomness type for gen_side_sharesModuloOdd");

    if (r_type == "a_1") {
      for (size_t i = 0; i < size; ++i) {
        shares_1[i] = aes_a_1->randModuloOdd();
      }
    }

    if (r_type == "a_2") {
      for (size_t i = 0; i < size; ++i) {
        shares_1[i] = aes_a_2->randModuloOdd();
      }
    }
  }

  template <typename T>
  void sharesModuloOdd(
    vector<mpc_t>& shares_1, vector<mpc_t>& shares_2, const vector<T>& x, size_t size,
    string r_type) {
    assert(
      (r_type == "COMMON" or r_type == "INDEP" or r_type == "a_1" or r_type == "a_2") &&
      "invalid randomness type for sharesOfBits");

    if (r_type == "a_1") {
      for (size_t i = 0; i < size; ++i)
        shares_1[i] = aes_a_1->randModuloOdd();
    }

    if (r_type == "a_2") {
      for (size_t i = 0; i < size; ++i)
        shares_1[i] = aes_a_2->randModuloOdd();
    }

    if (r_type == "COMMON") {
      for (size_t i = 0; i < size; ++i)
        shares_1[i] = aes_common->randModuloOdd();
    }

    if (r_type == "INDEP") {
      for (size_t i = 0; i < size; ++i)
        shares_1[i] = aes_indep->randModuloOdd();
    }

    subtractModuloOdd<T, mpc_t>(x, shares_1, shares_2, size);
  }
  // from tools.hpp end

  // from tools.cpp beg
  void populateBitsVector(vector<small_mpc_t>& vec, string r_type, size_t size) {
    assert(
      (r_type == "COMMON" or r_type == "INDEP") &&
      "invalid randomness type for populateBitsVector");

    if (r_type == "COMMON") {
      for (size_t i = 0; i < size; ++i)
        vec[i] = aes_common->getBit();
    }

    if (r_type == "INDEP") {
      for (size_t i = 0; i < size; ++i)
        vec[i] = aes_indep->getBit();
    }
  }

  void gen_side_shareOfBits(vector<small_mpc_t>& bit_shares, size_t size, string r_type) {
    assert(
      (r_type == "a_1" or r_type == "a_2") && "invalid randomness type for gen_side_shareOfBits");
    small_mpc_t temp;

    if (r_type == "a_1") {
      for (size_t i = 0; i < size; ++i) {
        for (size_t k = 0; k < BIT_SIZE; ++k) {
          temp = aes_a_1->randModPrime();
          bit_shares[i * BIT_SIZE + k] = temp;
        }
      }
    }

    if (r_type == "a_2") {
      for (size_t i = 0; i < size; ++i) {
        for (size_t k = 0; k < BIT_SIZE; ++k) {
          temp = aes_a_2->randModPrime();
          bit_shares[i * BIT_SIZE + k] = temp;
        }
      }
    }
  }

  // Returns shares of MSB...LSB of first number and so on.
  void sharesOfBits(
    vector<small_mpc_t>& bit_shares_x_1, vector<small_mpc_t>& bit_shares_x_2,
    const vector<mpc_t>& x, size_t size, string r_type) {
    assert(
      (r_type == "COMMON" or r_type == "INDEP" or r_type == "a_1") &&
      "invalid randomness type for sharesOfBits");
    small_mpc_t temp;

    if (r_type == "a_1") {
      for (size_t i = 0; i < size; ++i) {
        for (size_t k = 0; k < BIT_SIZE; ++k) {
          temp = aes_a_1->randModPrime();
          bit_shares_x_1[i * BIT_SIZE + k] = temp;
          bit_shares_x_2[i * BIT_SIZE + k] =
            subtractModPrime((x[i] >> (BIT_SIZE - 1 - k) & 1), temp);
        }
      }
    }

    if (r_type == "COMMON") {
      for (size_t i = 0; i < size; ++i) {
        for (size_t k = 0; k < BIT_SIZE; ++k) {
          temp = aes_common->randModPrime();
          bit_shares_x_1[i * BIT_SIZE + k] = temp;
          bit_shares_x_2[i * BIT_SIZE + k] =
            subtractModPrime((x[i] >> (BIT_SIZE - 1 - k) & 1), temp);
        }
      }
    }

    if (r_type == "INDEP") {
      for (size_t i = 0; i < size; ++i) {
        for (size_t k = 0; k < BIT_SIZE; ++k) {
          temp = aes_indep->randModPrime();
          bit_shares_x_1[i * BIT_SIZE + k] = temp;
          bit_shares_x_2[i * BIT_SIZE + k] =
            subtractModPrime((x[i] >> (BIT_SIZE - 1 - k) & 1), temp);
        }
      }
    }
  }

  // Returns boolean shares of LSB of r.
  void sharesOfLSB(
    vector<small_mpc_t>& share_1, vector<small_mpc_t>& share_2, const vector<mpc_t>& r, size_t size,
    string r_type) {
    assert((r_type == "COMMON" or r_type == "INDEP") && "invalid randomness type for sharesOfLSB");

    if (r_type == "COMMON") {
      for (size_t i = 0; i < size; ++i) {
        share_1[i] = aes_common->getBit();
        share_2[i] = share_1[i] ^ (r[i] % 2);
      }
    }

    if (r_type == "INDEP") {
      for (size_t i = 0; i < size; ++i) {
        share_1[i] = aes_indep->getBit();
        share_2[i] = share_1[i] ^ (r[i] % 2);
      }
    }
  }

  // Returns \Z_L shares of LSB of r.
  void sharesOfLSB(
    vector<mpc_t>& share_1, vector<mpc_t>& share_2, const vector<mpc_t>& r, size_t size,
    string r_type) {
    assert((r_type == "COMMON" or r_type == "INDEP") && "invalid randomness type for sharesOfLSB");

    if (r_type == "COMMON") {
      for (size_t i = 0; i < size; ++i) {
        share_1[i] = aes_common->get64Bits();
        share_2[i] = FloatToMpcType(r[i] % 2, GetMpcContext()->FLOAT_PRECISION) - share_1[i];
      }
    }

    if (r_type == "INDEP") {
      for (size_t i = 0; i < size; ++i) {
        share_1[i] = aes_indep->get64Bits();
        share_2[i] = FloatToMpcType(r[i] % 2, GetMpcContext()->FLOAT_PRECISION) - share_1[i];
      }
    }
  }

  // Returns \Z_L shares of LSB of r.
  void sharesOfLSB2(
    vector<mpc_t>& share_1, vector<mpc_t>& share_2, const vector<mpc_t>& r, size_t size,
    string r_type) {
    assert((r_type == "COMMON" or r_type == "INDEP" or r_type == "a_1") && 
    "invalid randomness type for sharesOfLSB");

    if (r_type == "COMMON") {
      for (size_t i = 0; i < size; ++i) {
        share_1[i] = aes_common->get64Bits();
        share_2[i] = FloatToMpcType(r[i] % 2, GetMpcContext()->FLOAT_PRECISION) - share_1[i];
      }
    }

    if (r_type == "INDEP") {
      for (size_t i = 0; i < size; ++i) {
        share_1[i] = aes_indep->get64Bits();
        share_2[i] = FloatToMpcType(r[i] % 2, GetMpcContext()->FLOAT_PRECISION) - share_1[i];
      }
    }

    if (r_type == "a_1") {
      for (size_t i = 0; i < size; ++i) {
        share_1[i] = aes_a_1->get64Bits();
        share_2[i] = FloatToMpcType(r[i] % 2, GetMpcContext()->FLOAT_PRECISION) - share_1[i];
      }
    }
  }

  // Returns boolean shares of a bit vector vec.
  void sharesOfBitVector(
    vector<small_mpc_t>& share_1, vector<small_mpc_t>& share_2, const vector<small_mpc_t>& vec,
    size_t size, string r_type) {
    assert((r_type == "COMMON" or r_type == "INDEP") && "invalid randomness type for sharesOfLSB");

    if (r_type == "COMMON") {
      for (size_t i = 0; i < size; ++i) {
        share_1[i] = aes_common->getBit();
        share_2[i] = share_1[i] ^ vec[i];
      }
    }

    if (r_type == "INDEP") {
      for (size_t i = 0; i < size; ++i) {
        share_1[i] = aes_indep->getBit();
        share_2[i] = share_1[i] ^ vec[i];
      }
    }
  }

  // Returns \Z_L shares of a bit vector vec.
  void sharesOfBitVector(
    vector<mpc_t>& share_1, vector<mpc_t>& share_2, const vector<small_mpc_t>& vec, size_t size,
    string r_type) {
    assert(
      (r_type == "COMMON" or r_type == "INDEP" or r_type == "a_2") &&
      "invalid randomness type for sharesOfLSB");

    if (r_type == "a_2") {
      for (size_t i = 0; i < size; ++i) {
        share_2[i] = aes_a_2->get64Bits();
        share_1[i] = FloatToMpcType(vec[i], GetMpcContext()->FLOAT_PRECISION) - share_2[i];
      }
    }

    if (r_type == "COMMON") {
      for (size_t i = 0; i < size; ++i) {
        share_1[i] = aes_common->get64Bits();
        share_2[i] = FloatToMpcType(vec[i], GetMpcContext()->FLOAT_PRECISION) - share_1[i];
      }
    }

    if (r_type == "INDEP") {
      for (size_t i = 0; i < size; ++i) {
        share_1[i] = aes_indep->get64Bits();
        share_2[i] = FloatToMpcType(vec[i], GetMpcContext()->FLOAT_PRECISION) - share_1[i];
      }
    }
  }

  void gen_side_shareOfBitVector(vector<mpc_t>& share_side, size_t size, string r_type) {
    assert(r_type == "a_2" && "invalid randomness type for gen_side_shareOfBitVector");

    if (r_type == "a_2") {
      for (size_t i = 0; i < size; ++i) {
        share_side[i] = aes_a_2->get64Bits();
      }
    }
  }

  // Split shares of a vector of mpc_t into shares (randomness is independent)
  void splitIntoShares(const vector<mpc_t>& a, vector<mpc_t>& a1, vector<mpc_t>& a2, size_t size) {
    populateRandomVector<mpc_t>(a1, size, "INDEP", "POSITIVE");
    subtractVectors<mpc_t>(a, a1, a2, size);
  }
  // from tools.cpp end

  template <typename T>
  void populateRandomVector(vector<T>& vec, size_t size, string r_type, string neg_type) {
    // assert((r_type == "COMMON" or r_type == "INDEP") && "invalid randomness type for
    // populateRandomVector");
    assert(
      (neg_type == "NEGATIVE" or neg_type == "POSITIVE") &&
      "invalid negativeness type for populateRandomVector");
    // assert(sizeof(T) == sizeof(mpc_t) && "Probably only need 64-bit numbers");
    // assert(r_type == "COMMON" && "Only common randomness mode required currently");

    mpc_t sign = 1;
    if (r_type == "COMMON") {
      if (neg_type == "NEGATIVE") {
        if (partyNum == PARTY_B || partyNum == PARTY_D)
          sign = MINUS_ONE;

        if (sizeof(T) == sizeof(mpc_t)) {
          for (size_t i = 0; i < size; ++i)
            vec[i] = sign * aes_common->get64Bits();
        } else {
          for (size_t i = 0; i < size; ++i)
            vec[i] = sign * aes_common->get8Bits();
        }
      }

      if (neg_type == "POSITIVE") {
        if (sizeof(T) == sizeof(mpc_t)) {
          for (size_t i = 0; i < size; ++i)
            vec[i] = aes_common->get64Bits();
        } else {
          for (size_t i = 0; i < size; ++i)
            vec[i] = aes_common->get8Bits();
        }
      }
    }

    if (r_type == "INDEP") {
      if (neg_type == "NEGATIVE") {
        if (partyNum == PARTY_B || partyNum == PARTY_D)
          sign = MINUS_ONE;

        if (sizeof(T) == sizeof(mpc_t)) {
          for (size_t i = 0; i < size; ++i)
            vec[i] = sign * aes_indep->get64Bits();
        } else {
          for (size_t i = 0; i < size; ++i)
            vec[i] = sign * aes_indep->get8Bits();
        }
      }

      if (neg_type == "POSITIVE") {
        if (sizeof(T) == sizeof(mpc_t)) {
          for (size_t i = 0; i < size; ++i)
            vec[i] = aes_indep->get64Bits();
        } else {
          for (size_t i = 0; i < size; ++i)
            vec[i] = aes_indep->get8Bits();
        }
      }
    }

    if (r_type == "a_1") {
      assert((partyNum == PARTY_A || partyNum == PARTY_C) && "Only A and C can call for a_1");
      assert(neg_type == "POSITIVE" && "neg_type should be POSITIVE");
      // assert(sizeof(T) == sizeof(mpc_t) && "sizeof(T) == sizeof(mpc_t)");
      // for (size_t i = 0; i < size; ++i)
      //  vec[i] = aes_a_1->get64Bits();
      if (sizeof(T) == sizeof(mpc_t)) {
        for (size_t i = 0; i < size; ++i)
          vec[i] = aes_a_1->get64Bits();
      } else {
        for (size_t i = 0; i < size; ++i)
          vec[i] = aes_a_1->get8Bits();
      }
    }

    if (r_type == "b_1") {
      assert((partyNum == PARTY_A || partyNum == PARTY_C) && "Only A and C can call for b_1");
      assert(neg_type == "POSITIVE" && "neg_type should be POSITIVE");
      // assert(sizeof(T) == sizeof(mpc_t) && "sizeof(T) == sizeof(mpc_t)");
      // for (size_t i = 0; i < size; ++i)
      //   vec[i] = aes_b_1->get64Bits();
      if (sizeof(T) == sizeof(mpc_t)) {
        for (size_t i = 0; i < size; ++i)
          vec[i] = aes_b_1->get64Bits();
      } else {
        for (size_t i = 0; i < size; ++i)
          vec[i] = aes_b_1->get8Bits();
      }      
    }

    if (r_type == "c_1") {
      assert((partyNum == PARTY_A || partyNum == PARTY_C) && "Only A and C can call for c_1");
      assert(neg_type == "POSITIVE" && "neg_type should be POSITIVE");
      // assert(sizeof(T) == sizeof(mpc_t) && "sizeof(T) == sizeof(mpc_t)");
      // for (size_t i = 0; i < size; ++i)
      //   vec[i] = aes_c_1->get64Bits();
      if (sizeof(T) == sizeof(mpc_t)) {
        for (size_t i = 0; i < size; ++i)
          vec[i] = aes_c_1->get64Bits();
      } else {
        for (size_t i = 0; i < size; ++i)
          vec[i] = aes_c_1->get8Bits();
      }       
    }

    if (r_type == "a_2") {
      assert((partyNum == PARTY_B || partyNum == PARTY_C) && "Only B and C can call for a_2");
      assert(neg_type == "POSITIVE" && "neg_type should be POSITIVE");
      // assert(sizeof(T) == sizeof(mpc_t) && "sizeof(T) == sizeof(mpc_t)");
      // for (size_t i = 0; i < size; ++i)
      //   vec[i] = aes_a_2->get64Bits();
      if (sizeof(T) == sizeof(mpc_t)) {
        for (size_t i = 0; i < size; ++i)
          vec[i] = aes_a_2->get64Bits();
      } else {
        for (size_t i = 0; i < size; ++i)
          vec[i] = aes_a_2->get8Bits();
      }            
    }

    if (r_type == "b_2") {
      assert((partyNum == PARTY_B || partyNum == PARTY_C) && "Only B and C can call for b_2");
      assert(neg_type == "POSITIVE" && "neg_type should be POSITIVE");
      // assert(sizeof(T) == sizeof(mpc_t) && "sizeof(T) == sizeof(mpc_t)");
      // for (size_t i = 0; i < size; ++i)
      //   vec[i] = aes_b_2->get64Bits();
      if (sizeof(T) == sizeof(mpc_t)) {
        for (size_t i = 0; i < size; ++i)
          vec[i] = aes_b_2->get64Bits();
      } else {
        for (size_t i = 0; i < size; ++i)
          vec[i] = aes_b_2->get8Bits();
      }       
    }
  }

  template <typename T>
  void populateRandomVector2(vector<T>& vec, size_t size, string r_type, string neg_type) {
    assert((neg_type == "NEGATIVE" or neg_type == "POSITIVE") && "invalid negativeness type");
    assert(sizeof(T) == sizeof(mpc_t) && "sizeof(T) == sizeof(mpc_t)");

    std::shared_ptr<AESObject> obj = nullptr;
    if (r_type == "COMMON") {
      if (partyNum == PARTY_C) {
        vec.resize(size);
        return;
      }
      assert((partyNum == PARTY_A || partyNum == PARTY_B) && "Only A and B can call for common");
      obj = aes_common;
    } else if (r_type == "a_1") {
      if (partyNum == PARTY_B) {
        vec.resize(size);
        return;
      }
      assert((partyNum == PARTY_A || partyNum == PARTY_C) && "Only A and C can call for a_1");
      obj = aes_a_1;
    } else if (r_type == "a_2") {
      if (partyNum == PARTY_A) {
        vec.resize(size);
        return;
      }
      assert((partyNum == PARTY_B || partyNum == PARTY_C) && "Only B and C can call for a_2");
      obj = aes_a_2;
    } else {
      assert(false && "invalid r_type (only support COMMON,a_1,a_2)");
    }

    mpc_t sign = 1;
    if (neg_type == "NEGATIVE") {
      if (partyNum == PARTY_B || partyNum == PARTY_D)
        sign = MINUS_ONE;

      if (sizeof(T) == sizeof(mpc_t)) {
        for (size_t i = 0; i < size; ++i)
          vec[i] = sign * obj->get64Bits();
      } else {
        for (size_t i = 0; i < size; ++i)
          vec[i] = sign * obj->get8Bits();
      }
    }

    if (neg_type == "POSITIVE") {
      if (sizeof(T) == sizeof(mpc_t)) {
        for (size_t i = 0; i < size; ++i)
          vec[i] = obj->get64Bits();
      } else {
        for (size_t i = 0; i < size; ++i)
          vec[i] = obj->get8Bits();
      }
    }
  }

  template <typename T>
  void populateRandomVector3(vector<T>& vec, size_t size) {
    if (sizeof(T) == sizeof(mpc_t)) {
      for (size_t i = 0; i < size; ++i)
        vec[i] = aes_private->get64Bits();
    } else {
      for (size_t i = 0; i < size; ++i)
        vec[i] = aes_private->get8Bits();
    }
  }

  /*
  ** this function only used for 4PC
  ** will remove this in the future
  */
  template <typename T>
  void getVectorfromPrimary(vector<T>& vec, size_t size, string r_mode, string n_mode) {
    assert(
      ((r_mode == "RANDOMIZE") || (r_mode == "AS-IS")) &&
      "Random mode issue in getVectorfromPrimary");
    assert(
      ((n_mode == "NATURAL") || (n_mode == "UNNATURAL")) &&
      "Natural mode issue in getVectorfromPrimary");

    if (r_mode == "RANDOMIZE") {
      // Original vec also gets modified here.
      vector<mpc_t> temp(size);
      if (PRIMARY) {
        populateRandomVector<mpc_t>(temp, size, "COMMON", "NEGATIVE");
        addVectors<mpc_t>(vec, temp, vec, size);
      }
    }

    if (n_mode == "NATURAL") {
      if (PRIMARY)
        sendVector<T>(vec, partner(partyNum), size);
      if (!PRIMARY)
        receiveVector<T>(vec, partner(partyNum), size);
    } else {
      if (partyNum == PARTY_A)
        sendVector<T>(vec, PARTY_D, size);
      if (partyNum == PARTY_B)
        sendVector<T>(vec, PARTY_C, size);

      if (partyNum == PARTY_C)
        receiveVector<T>(vec, PARTY_B, size);
      if (partyNum == PARTY_D)
        receiveVector<T>(vec, PARTY_A, size);
    }
  }

  template <typename T>
  void sendVector(const vector<T>& vec, size_t player, size_t size) {
    sendBuf(player, (const char*)vec.data(), size * sizeof(T), 0);
  }

  template <typename T>
  void receiveVector(vector<T>& vec, size_t player, size_t size) {
    receiveBuf(player, (char*)vec.data(), size * sizeof(T), 0);
  }

  template <typename T>
  void sendVector2(const vector<T>& vec, const string& node_id, size_t size) {
    sendBuf(node_id, (const char*)vec.data(), size * sizeof(T), 0);
  }

  template <typename T>
  void receiveVector2(vector<T>& vec, const string& node_id, size_t size) {
    receiveBuf(node_id, (char*)vec.data(), size * sizeof(T), 0);
  }

  template <typename T>
  void sendTwoVectors(
    const vector<T>& vec1, const vector<T>& vec2, size_t player, size_t size1, size_t size2) {
    vector<T> temp(size1 + size2);
    for (size_t i = 0; i < size1; ++i)
      temp[i] = vec1[i];
    for (size_t i = 0; i < size2; ++i)
      temp[size1 + i] = vec2[i];

    sendVector<T>(temp, player, size1 + size2);
  }

  template <typename T>
  void receiveTwoVectors(
    vector<T>& vec1, vector<T>& vec2, size_t player, size_t size1, size_t size2) {
    vector<T> temp(size1 + size2);
    receiveVector<T>(temp, player, size1 + size2);

    for (size_t i = 0; i < size1; ++i)
      vec1[i] = temp[i];

    for (size_t i = 0; i < size2; ++i)
      vec2[i] = temp[size1 + i];
  }

  template <typename T>
  void sendThreeVectors(
    const vector<T>& vec1, const vector<T>& vec2, const vector<T>& vec3, size_t player,
    size_t size1, size_t size2, size_t size3) {
    vector<T> temp(size1 + size2 + size3);
    for (size_t i = 0; i < size1; ++i)
      temp[i] = vec1[i];

    for (size_t i = 0; i < size2; ++i)
      temp[size1 + i] = vec2[i];

    for (size_t i = 0; i < size3; ++i)
      temp[size1 + size2 + i] = vec3[i];

    sendVector<T>(temp, player, size1 + size2 + size3);
  }

  template <typename T>
  void receiveThreeVectors(
    vector<T>& vec1, vector<T>& vec2, vector<T>& vec3, size_t player, size_t size1, size_t size2,
    size_t size3) {
    vector<T> temp(size1 + size2 + size3);
    receiveVector<T>(temp, player, size1 + size2 + size3);

    for (size_t i = 0; i < size1; ++i)
      vec1[i] = temp[i];

    for (size_t i = 0; i < size2; ++i)
      vec2[i] = temp[size1 + i];

    for (size_t i = 0; i < size3; ++i)
      vec3[i] = temp[size1 + size2 + i];
  }

  template <typename T>
  void sendFourVectors(
    const vector<T>& vec1, const vector<T>& vec2, const vector<T>& vec3, const vector<T>& vec4,
    size_t player, size_t size1, size_t size2, size_t size3, size_t size4) {
    vector<T> temp(size1 + size2 + size3 + size4);

    for (size_t i = 0; i < size1; ++i)
      temp[i] = vec1[i];

    for (size_t i = 0; i < size2; ++i)
      temp[size1 + i] = vec2[i];

    for (size_t i = 0; i < size3; ++i)
      temp[size1 + size2 + i] = vec3[i];

    for (size_t i = 0; i < size4; ++i)
      temp[size1 + size2 + size3 + i] = vec4[i];

    sendVector<T>(temp, player, size1 + size2 + size3 + size4);
  }

  template <typename T>
  void receiveFourVectors(
    vector<T>& vec1, vector<T>& vec2, vector<T>& vec3, vector<T>& vec4, size_t player, size_t size1,
    size_t size2, size_t size3, size_t size4) {
    vector<T> temp(size1 + size2 + size3 + size4);
    receiveVector<T>(temp, player, size1 + size2 + size3 + size4);

    for (size_t i = 0; i < size1; ++i)
      vec1[i] = temp[i];

    for (size_t i = 0; i < size2; ++i)
      vec2[i] = temp[size1 + i];

    for (size_t i = 0; i < size3; ++i)
      vec3[i] = temp[size1 + size2 + i];

    for (size_t i = 0; i < size4; ++i)
      vec4[i] = temp[size1 + size2 + size3 + i];
  }

  template <typename T>
  void sendSixVectors(
    const vector<T>& vec1, const vector<T>& vec2, const vector<T>& vec3, const vector<T>& vec4,
    const vector<T>& vec5, const vector<T>& vec6, size_t player, size_t size1, size_t size2,
    size_t size3, size_t size4, size_t size5, size_t size6) {
    vector<T> temp(size1 + size2 + size3 + size4 + size5 + size6);
    size_t offset = 0;

    for (size_t i = 0; i < size1; ++i)
      temp[i + offset] = vec1[i];

    offset += size1;
    for (size_t i = 0; i < size2; ++i)
      temp[i + offset] = vec2[i];

    offset += size2;
    for (size_t i = 0; i < size3; ++i)
      temp[i + offset] = vec3[i];

    offset += size3;
    for (size_t i = 0; i < size4; ++i)
      temp[i + offset] = vec4[i];

    offset += size4;
    for (size_t i = 0; i < size5; ++i)
      temp[i + offset] = vec5[i];

    offset += size5;
    for (size_t i = 0; i < size6; ++i)
      temp[i + offset] = vec6[i];

    sendVector<T>(temp, player, size1 + size2 + size3 + size4 + size5 + size6);
  }

  template <typename T>
  void receiveSixVectors(
    vector<T>& vec1, vector<T>& vec2, vector<T>& vec3, vector<T>& vec4, vector<T>& vec5,
    vector<T>& vec6, size_t player, size_t size1, size_t size2, size_t size3, size_t size4,
    size_t size5, size_t size6) {
    vector<T> temp(size1 + size2 + size3 + size4 + size5 + size6);
    size_t offset = 0;

    receiveVector<T>(temp, player, size1 + size2 + size3 + size4 + size5 + size6);

    for (size_t i = 0; i < size1; ++i)
      vec1[i] = temp[i + offset];

    offset += size1;
    for (size_t i = 0; i < size2; ++i)
      vec2[i] = temp[i + offset];

    offset += size2;
    for (size_t i = 0; i < size3; ++i)
      vec3[i] = temp[i + offset];

    offset += size3;
    for (size_t i = 0; i < size4; ++i)
      vec4[i] = temp[i + offset];

    offset += size4;
    for (size_t i = 0; i < size5; ++i)
      vec5[i] = temp[i + offset];

    offset += size5;
    for (size_t i = 0; i < size6; ++i)
      vec6[i] = temp[i + offset];
  }

  // Added in 20200928 by Junjie Shi, and used by Equal only for now.
  // Attention!! only the LAST bit is USED!!
  void sendBitVector(const vector<small_mpc_t>& vec, size_t player, size_t size) {
    int len = (size + 7) / 8;
    unsigned char* ptr = new unsigned char[len];
    memset(ptr, 0, len);
    int i = 0;
    for (int j = 0; j < len; j++) {
      for (int k = 0; k < 8; k++) {
        if (i < size) {
          ptr[j] = (((vec[i++] & 0x1) << k) & 0xFF) | ptr[j];
        }
      }
    }
    int ret = io->send(player, (const char*)ptr, len, msg_id());
    delete[] ptr;
 }

void receiveTwoBitVector(vector<small_mpc_t>& vec_a,
                                vector<small_mpc_t>& vec_b,
                                size_t player, size_t size_a, size_t size_b) {
    vector<small_mpc_t> temp(size_a + size_b);
    receiveBitVector(temp, player, size_a + size_b);

    for (size_t i = 0; i < size_a; ++i)
      vec_a[i] = temp[i];

    for (size_t i = 0; i < size_b; ++i)
      vec_b[i] = temp[size_a + i];
 }
 

 void sendTwoBitVector(const vector<small_mpc_t>& vec_a,
                                const vector<small_mpc_t>& vec_b,
                                size_t player, size_t size_a, size_t size_b) {
    vector<small_mpc_t> temp(size_a + size_b);
    for (size_t i = 0; i < size_a; ++i)
      temp[i] = vec_a[i];
    for (size_t i = 0; i < size_b; ++i)
      temp[size_a + i] = vec_b[i];
    
    sendBitVector(temp, player, size_a + size_b);
 }

 void receiveBitVector(vector<small_mpc_t>& vec, size_t player, size_t size) {
    int len = (size + 7) / 8;
    unsigned char* ptr = new unsigned char[len];
    memset(ptr, 0, len);
    int ret = io->recv(player, (char*)ptr, len, msg_id());

    int i = 0;
    for (int j = 0; j < len; j++) {
      for (int k = 0; k < 8; k++) {
        if (i < size) {
          vec[i++] = (ptr[j] >> k) & 0x01;
        }
      }
    }
    delete[] ptr;
 }

void sendBuf(int player, const char* buf, int length, int conn /* = 0*/) {
  message_sent_.fetch_add(1);
  bytes_sent_.fetch_add(length);

#if OPEN_MPCOP_DEBUG_AND_CHECK
  {
    string key(msg_id().str());
    unique_lock<mutex> lck(g_key_stat_mtx);
    if (g_key_stat.find(key) != g_key_stat.end()) {
      g_key_stat[key]->message_sent.fetch_add(1);
      g_key_stat[key]->bytes_sent.fetch_add(length);
    }
  }
#endif

#if USE_NETIO_WITH_MESSAGEID
  io->send(player, buf, length, msg_id());
#else
  io->send(player, buf, length, 0);
#endif
}

void receiveBuf(int player, char* buf, int length, int conn /* = 0*/) {
  message_received_.fetch_add(1);
  bytes_received_.fetch_add(length);

#if OPEN_MPCOP_DEBUG_AND_CHECK
  {
    string key(msg_id().str());
    unique_lock<mutex> lck(g_key_stat_mtx);
    if (g_key_stat.find(key) != g_key_stat.end()) {
      g_key_stat[key]->message_received.fetch_add(1);
      g_key_stat[key]->bytes_received.fetch_add(length);
    }
  }
#endif

#if USE_NETIO_WITH_MESSAGEID
  io->recv(player, buf, length, msg_id());
#else
  io->recv(player, buf, length, 0);
#endif
}

void sendBuf(const string& node_id, const char* buf, int length, int conn /* = 0*/) {
  message_sent_.fetch_add(1);
  bytes_sent_.fetch_add(length);

#if OPEN_MPCOP_DEBUG_AND_CHECK
  {
    string key(msg_id().str());
    unique_lock<mutex> lck(g_key_stat_mtx);
    if (g_key_stat.find(key) != g_key_stat.end()) {
      g_key_stat[key]->message_sent.fetch_add(1);
      g_key_stat[key]->bytes_sent.fetch_add(length);
    }
  }
#endif

#if USE_NETIO_WITH_MESSAGEID
  io->send(node_id, buf, length, msg_id());
#else
  io->send(node_id, buf, length, 0);
#endif
}

void receiveBuf(const string& node_id, char* buf, int length, int conn /* = 0*/) {
  message_received_.fetch_add(1);
  bytes_received_.fetch_add(length);

#if OPEN_MPCOP_DEBUG_AND_CHECK
  {
    string key(msg_id().str());
    unique_lock<mutex> lck(g_key_stat_mtx);
    if (g_key_stat.find(key) != g_key_stat.end()) {
      g_key_stat[key]->message_received.fetch_add(1);
      g_key_stat[key]->bytes_received.fetch_add(length);
    }
  }
#endif

#if USE_NETIO_WITH_MESSAGEID
  io->recv(node_id, buf, length, msg_id());
#else
  io->recv(node_id, buf, length, 0);
#endif
}

void synchronize(int length) {
#if USE_NETIO_WITH_MESSAGEID
  cerr << "error! please use void synchronize(const msg_id_t& msg_id)" << endl;
  throw;
#endif
  io->sync();
}
void synchronize(const msg_id_t& msg_id) { io->sync_with(msg_id); }

// Share Truncation, truncate shares of a by power (in place) (power is logarithmic)
void Truncate(
  vector<mpc_t>& a,
  size_t power,
  size_t size,
  size_t party_1,
  size_t party_2,
  int partyNum) {
  if (!PRIMARY)
    return;
  assert((partyNum == party_1 || partyNum == party_2) && "Truncate called by spurious parties");

  if (partyNum == party_1) {
    for (size_t i = 0; i < size; ++i)
      a[i] = static_cast<mpc_t>(static_cast<signed_mpc_t>(a[i]) >> power);
  }

  if (partyNum == party_2) {
    for (size_t i = 0; i < size; ++i)
      a[i] = -static_cast<mpc_t>(static_cast<signed_mpc_t>(-a[i]) >> power);
  }
}

void Truncate(mpc_t& a, size_t power, size_t party_1, size_t party_2, int partyNum) {
  assert((partyNum == party_1 || partyNum == party_2) && "Truncate called by spurious parties");

  if (partyNum == party_1)
    a = static_cast<mpc_t>(static_cast<signed_mpc_t>(a) >> power);

  if (partyNum == party_2)
    a = -static_cast<mpc_t>(static_cast<signed_mpc_t>(-a) >> power);
}

void Truncate_many(
  vector<mpc_t>& a,
  vector<size_t> power,
  size_t size,
  size_t party_1,
  size_t party_2,
  int partyNum) {
  if (!PRIMARY)
    return;
  assert((partyNum == party_1 || partyNum == party_2) && "Truncate called by spurious parties");

  if (partyNum == party_1) {
    for (size_t i = 0; i < size; ++i)
      a[i] = static_cast<mpc_t>(static_cast<signed_mpc_t>(a[i]) >> power[i]);
  }

  if (partyNum == party_2) {
    for (size_t i = 0; i < size; ++i)
      a[i] = -static_cast<mpc_t>(static_cast<signed_mpc_t>(-a[i]) >> power[i]);
  }
}

// XOR shares with a public bit into output.
void XORModuloOdd(
  vector<small_mpc_t>& bit,
  vector<mpc_t>& shares,
  vector<mpc_t>& output,
  size_t size,
  int partyNum) {
  if (!PRIMARY)
    return;
  if (partyNum == PARTY_A) {
    for (size_t i = 0; i < size; ++i) {
      if (bit[i] == 1)
        output[i] = subtractModuloOdd<small_mpc_t, mpc_t>(1, shares[i]);
      else
        output[i] = shares[i];
    }
  }

  if (partyNum == PARTY_B) {
    for (size_t i = 0; i < size; ++i) {
      if (bit[i] == 1)
        output[i] = subtractModuloOdd<small_mpc_t, mpc_t>(0, shares[i]);
      else
        output[i] = shares[i];
    }
  }
}
