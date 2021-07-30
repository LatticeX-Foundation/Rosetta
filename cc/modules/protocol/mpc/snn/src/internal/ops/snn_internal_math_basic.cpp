#include "cc/modules/protocol/mpc/snn/include/snn_internal.h"
#include "cc/modules/common/include/utils/str_type_convert.h"
#include "cc/modules/protocol/mpc/snn/include/snn_triple_generator.h"
#include <thread>

using std::thread;

namespace rosetta
{
namespace snn
{

int SnnInternal::Negate(const vector<mpc_t>& a, vector<mpc_t>& b) {
  tlog_debug << "Negate ...";
  AUDIT("id:{}, P{} Negate, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  if (PRIMARY) {
    for (size_t i = 0; i < a.size(); ++i) {
      b[i] = MINUS_ONE - a[i] + 1; // in \Z_L
    }
  }

  AUDIT("id:{}, P{} Negate, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));
  tlog_debug << "Negate ok.";
  return 0;
}

int SnnInternal::Mul(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  return DotProduct(a, b, c);
}

int SnnInternal::Mul(const vector<string>& const_a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  return DotProduct(const_a, b, c);
}
int SnnInternal::Mul(const vector<mpc_t>& a, const vector<string>& const_b, vector<mpc_t>& c) {
  return DotProduct(a, const_b, c);
}
int SnnInternal::Mul(const vector<double>& const_a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  return DotProduct(const_a, b, c);
}
int SnnInternal::Mul(const vector<mpc_t>& a, const vector<double>& const_b, vector<mpc_t>& c) {
  return DotProduct(const_b, a, c);
}

int SnnInternal::Square(const vector<mpc_t>& a, vector<mpc_t>& c) {
  return DotProduct(a, a, c);
}

int SnnInternal::Exp(const vector<mpc_t>& a, vector<mpc_t>& c) {
  tlog_debug << "Exp ...";
  AUDIT("id:{}, P{} Exp, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));

  size_t a_size = a.size();
  size_t n = 500;
  const vector<double_t> n_reciprocal(a_size, 0.002);
  vector<mpc_t> m(a_size, 0);
  vector<mpc_t> result(a_size, 0);

  DotProduct(n_reciprocal, a, m);
  
  const vector<mpc_t> one(a_size, FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION));
  if (partyNum == PARTY_A) 
      addVectors<mpc_t>(m, one, m, a_size);
  PowV1(m, n, c);

  AUDIT("id:{}, P{} Exp, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "Exp ok."; 
  return 0;
}

int SnnInternal::Rsqrt(const vector<mpc_t>& a, vector<mpc_t>& c) {
  tlog_debug << "Rsqrt ...";

  const int sqrt_nr_iters = 3;
  vector<mpc_t> rsqrt_nr_initial = a;
  size_t size = a.size();
  
  AUDIT("id:{}, P{} Rsqrt, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));

  const vector<double> float_half(size, 0.5);
  const vector<double> float_two_ten(size, 0.2);
  const vector<double> float_two_dot_two(size, 2.2);
  const vector<double> float_milli(size, 0.0009765625);
  const vector<double> float_three(size, 3);

  vector<mpc_t> inter_Number(size);

  // Mul(a, float_half, rsqrt_nr_initial);
  DotProduct(float_half, a, rsqrt_nr_initial);

  Add(rsqrt_nr_initial, float_two_ten, inter_Number); //x/2+0.2

  Negative(inter_Number, rsqrt_nr_initial); // negative

  Exp(rsqrt_nr_initial, inter_Number); //exp(-(x/2+0.2))

  Mul(inter_Number, float_two_dot_two, rsqrt_nr_initial); // mul 2.2

  Add(rsqrt_nr_initial, float_two_ten, inter_Number);//add 0.2

  vector<mpc_t> temp(a.size());
  Mul(a, float_milli, temp); //compute a/1024
  Sub(inter_Number, temp, rsqrt_nr_initial); //rsqrt_nr_initial -=a/1024

  vector<mpc_t> temp_number0(size), temp_number1(size), temp_number2(size);
  for (int i = 0; i < sqrt_nr_iters; i++)
  {
    // y.mul_(3 - self * y.square()).div_(2)
    Mul(rsqrt_nr_initial, rsqrt_nr_initial, temp_number1);
    Mul(temp_number1, a, temp_number0); //temp_number0 = a * rsqrt_nr_initial^2
    Negative(temp_number0, temp_number1);

    Add(temp_number1, float_three, temp_number0);
    
    Mul(temp_number0, rsqrt_nr_initial, temp_number2); 
    Mul(temp_number2, float_half, temp_number0);
    rsqrt_nr_initial = temp_number0;
  }     

  Abs(rsqrt_nr_initial, c);
  // c = rsqrt_nr_initial;
  
  AUDIT("id:{}, P{} Rsqrt, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  
  tlog_debug << "Rsqrt ok.";
  return 0;
}

int SnnInternal::Sqrt(const vector<mpc_t>& a, vector<mpc_t>& c) {
  tlog_debug << "Sqrt ...";

  if (THREE_PC)
  {
    AUDIT("id:{}, P{} Sqrt, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
    size_t a_size = a.size();
    vector<mpc_t> temp(a_size , 0);
    Rsqrt(a, temp);
    DotProduct(a, temp, c);
    AUDIT("id:{}, P{} Sqrt, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
	
  }

  tlog_debug << "Sqrt ok.";
  return 0;
}

int SnnInternal::Add(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  log_debug << "Add const ...";

  vector<mpc_t> plain_a(a.size(), 0);
  if (partyNum == PARTY_A)
    convert_double_to_mpctype(a, plain_a, GetMpcContext()->FLOAT_PRECISION);

  c.resize(a.size());
  addVectors(plain_a, b, c, a.size());

  log_debug << "Add const ok.";
  return 0;
}
int SnnInternal::Add(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c) {
  return Add(b, a, c);
}
int SnnInternal::Add(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  tlog_debug << "Add ...";
  AUDIT("id:{}, P{} Add, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  AUDIT("id:{}, P{} Add, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));

  c.resize(a.size());
  addVectors(a, b, c, a.size());

  AUDIT("id:{}, P{} Add, output Z(=X+Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "Add ok.";
  return 0;
}

int SnnInternal::Add(const vector<string>& const_a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  tlog_debug << "Add lh_is_const ...";
  size_t size = const_a.size();
  c.resize(size);
  int float_precision = GetMpcContext()->FLOAT_PRECISION;

  vector<double> da(size);
  vector<mpc_t> plain_a(size, 0);
  rosetta::convert::from_double_str(const_a, da);
  AUDIT("id:{}, P{} Add lh_is_const, input X(double){}", msg_id().get_hex(), context_->GetMyRole(), Vector<double>(da));
  AUDIT("id:{}, P{} Add lh_is_const, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));

  if (partyNum == PARTY_A)
    convert_double_to_mpctype(da, plain_a, GetMpcContext()->FLOAT_PRECISION);

  if (PRIMARY) {
    for (size_t i = 0; i < size; i++) {
      c[i] = plain_a[i] + b[i];
    }
  }

  AUDIT("id:{}, P{} Add lh_is_const, output Z(=X+Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "Add lh_is_const ok.";
  return 0;
}

int SnnInternal::Add(const vector<mpc_t>& a, const vector<string>& const_b, vector<mpc_t>& c) {
  tlog_debug << "Add rh_is_const ...";
  c.resize(a.size());
  return Add(const_b, a, c);
}

int SnnInternal::Sub(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  tlog_debug << "Sub ...";
  AUDIT("id:{}, P{} Sub, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  AUDIT("id:{}, P{} Sub, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));

  c.resize(a.size());
  subtractVectors(a, b, c, a.size());

  AUDIT("id:{}, P{} Sub, output Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "Sub ok.";
  return 0;
}
int SnnInternal::Sub(const vector<string>& const_a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  tlog_debug << "Sub lh_is_const ...";
  size_t size = const_a.size();
  c.resize(size);

  vector<mpc_t> sa(size, 0);
  Const2Share(const_a, sa);
  AUDIT("id:{}, P{} Sub lh_is_const, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(sa));
  AUDIT("id:{}, P{} Sub lh_is_const, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));

  if (PRIMARY) {
    for (size_t i = 0; i < size; i++) {
      c[i] = sa[i] - b[i];
    }
  }

  AUDIT("id:{}, P{} Sub lh_is_const, output Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "Sub lh_is_const ok.";
  return 0;
}
int SnnInternal::Sub(const vector<mpc_t>& a, const vector<string>& const_b, vector<mpc_t>& c) {
  tlog_debug << "Sub rh_is_const ...";
  size_t size = const_b.size();
  c.resize(size);
  
  vector<mpc_t> plain_b(size, 0);
  Const2Share(const_b, plain_b);

  AUDIT("id:{}, P{} Sub rh_is_const, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  AUDIT("id:{}, P{} Sub rh_is_const, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(plain_b));

  if (PRIMARY) {
    for (size_t i = 0; i < size; i++) {
      c[i] = a[i] - plain_b[i];
    }
  }

  AUDIT("id:{}, P{} Sub rh_is_const, output Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "Sub rh_is_const ok.";
  return 0;
}

// Matrix Multiplication of a*b = c with transpose flags for a,b.
// Output is a share between PARTY_A and PARTY_B.
// a^transpose_a is rows*common_dim and b^transpose_b is common_dim*columns
int SnnInternal::MatMul(
  const vector<mpc_t>& a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c,
  size_t rows,
  size_t common_dim,
  size_t columns,
  size_t transpose_a,
  size_t transpose_b) {
  assert(THREE_PC && "SelectShares called in non-3PC mode");
  tlog_debug << "MatMul ...";
  AUDIT("id:{}, P{} MatMul({},{},{}), input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(a));
  AUDIT("id:{}, P{} MatMul({},{},{}), input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(b));

  size_t size = rows * columns;
  size_t size_left = rows * common_dim;
  size_t size_right = common_dim * columns;
  vector<mpc_t> A(size_left, 0), B(size_right, 0), C(size, 0);

  if (HELPER) {
    vector<mpc_t> A1(size_left, 0), A2(size_left, 0), B1(size_right, 0), B2(size_right, 0),
      C1(size, 0), C2(size, 0);

    populateRandomVector<mpc_t>(A1, size_left, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} MatMul({},{},{}), populateRandomVector A1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(A1));

    populateRandomVector<mpc_t>(A2, size_left, "a_2", "POSITIVE");
    AUDIT("id:{}, P{} MatMul({},{},{}), populateRandomVector A2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(A2));

    populateRandomVector<mpc_t>(B1, size_right, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} MatMul({},{},{}), populateRandomVector B1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(B1));

    populateRandomVector<mpc_t>(B2, size_right, "a_2", "POSITIVE");
    AUDIT("id:{}, P{} MatMul({},{},{}), populateRandomVector B2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(B2));

    populateRandomVector<mpc_t>(C1, size, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} MatMul({},{},{}), populateRandomVector C1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(C1));

    addVectors<mpc_t>(A1, A2, A, size_left);
    AUDIT("id:{}, P{} MatMul({},{},{}), compute A=A1+A2, A(=A1+A2)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(A));

    addVectors<mpc_t>(B1, B2, B, size_right);
    AUDIT("id:{}, P{} MatMul({},{},{}), compute B=B1+B2, B(=B1+B2)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(B));

    EigenMatMul(A, B, C, rows, common_dim, columns, transpose_a, transpose_b);
    AUDIT("id:{}, P{} MatMul({},{},{}), compute C=A*B, C(=A*B)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(C));

    subtractVectors<mpc_t>(C, C1, C2, size);
    AUDIT("id:{}, P{} MatMul({},{},{}), compute C2=C-C1, C2(=C-C1)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(C2));

    sendVector<mpc_t>(C2, PARTY_B, size);
    AUDIT("id:{}, P{} MatMul({},{},{}) SEND to P{}, C2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, PARTY_B, Vector<mpc_t>(C2));
  }

  if (PRIMARY) {
    vector<mpc_t> E(size_left), F(size_right);
    vector<mpc_t> temp_E(size_left), temp_F(size_right);
    vector<mpc_t> temp_c(size);

    if (partyNum == PARTY_A) {
      populateRandomVector<mpc_t>(A, size_left, "a_1", "POSITIVE");
      AUDIT("id:{}, P{} MatMul({},{},{}), populateRandomVector A1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(A));

      populateRandomVector<mpc_t>(B, size_right, "a_1", "POSITIVE");
      AUDIT("id:{}, P{} MatMul({},{},{}), populateRandomVector B1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(B));

      populateRandomVector<mpc_t>(C, size, "a_1", "POSITIVE");
      AUDIT("id:{}, P{} MatMul({},{},{}), populateRandomVector C1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(C));
    }

    if (partyNum == PARTY_B) {
      populateRandomVector<mpc_t>(A, size_left, "a_2", "POSITIVE");
      AUDIT("id:{}, P{} MatMul({},{},{}), populateRandomVector A2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(A));

      populateRandomVector<mpc_t>(B, size_right, "a_2", "POSITIVE");
      AUDIT("id:{}, P{} MatMul({},{},{}), populateRandomVector B2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(B));

      receiveVector<mpc_t>(C, PARTY_C, size);
      AUDIT("id:{}, P{} MatMul({},{},{}) RECV from P{}, C2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, PARTY_C, Vector<mpc_t>(C));
    }

    // receiveThreeVectors<mpc_t>(A, B, C, PARTY_C, size_left, size_right, size);
    subtractVectors<mpc_t>(a, A, E, size_left);
    AUDIT("id:{}, P{} MatMul({},{},{}) compute: E=X-A, E(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(E));

    subtractVectors<mpc_t>(b, B, F, size_right);
    AUDIT("id:{}, P{} MatMul({},{},{}) compute: F=Y-B, F(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(F));

    thread* threads = new thread[2];

    threads[0] = thread(
      &SnnInternal::sendTwoVectors<mpc_t>, this, ref(E), ref(F), adversary(partyNum), size_left,
      size_right);
    AUDIT("id:{}, P{} MatMul({},{},{}) SEND to P{} E(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, adversary(partyNum), Vector<mpc_t>(E));
    AUDIT("id:{}, P{} MatMul({},{},{}) SEND to P{} F(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, adversary(partyNum), Vector<mpc_t>(F));

    threads[1] = thread(
      &SnnInternal::receiveTwoVectors<mpc_t>, this, ref(temp_E), ref(temp_F), adversary(partyNum),
      size_left, size_right);
    AUDIT("id:{}, P{} MatMul({},{},{}) RECV from P{} E(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, adversary(partyNum), Vector<mpc_t>(temp_E));
    AUDIT("id:{}, P{} MatMul({},{},{}) RECV from P{} F(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, adversary(partyNum), Vector<mpc_t>(temp_F));

    for (int i = 0; i < 2; i++)
      threads[i].join();

    delete[] threads;

    addVectors<mpc_t>(E, temp_E, E, size_left);
    AUDIT("id:{}, P{} MatMul({},{},{}) compute E=E0+E1, E(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(E));

    addVectors<mpc_t>(F, temp_F, F, size_right);
    AUDIT("id:{}, P{} MatMul({},{},{}) compute F=F0+F1, F(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(F));

    EigenMatMul(a, F, c, rows, common_dim, columns, transpose_a, transpose_b);
    AUDIT("id:{}, P{} MatMul({},{},{}) compute C0=X*F, C0(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(c));
    
    EigenMatMul(E, b, temp_c, rows, common_dim, columns, transpose_a, transpose_b);
    AUDIT("id:{}, P{} MatMul({},{},{}) compute C1=Y*E, C1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(temp_c));

    addVectors<mpc_t>(c, temp_c, c, size);
    AUDIT("id:{}, P{} MatMul({},{},{}), C(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(c));
    addVectors<mpc_t>(c, C, c, size);
    AUDIT("id:{}, P{} MatMul({},{},{}), C(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(c));

    if (partyNum == PARTY_A) {
      EigenMatMul(E, F, temp_c, rows, common_dim, columns, transpose_a, transpose_b);
      AUDIT("id:{}, P{} MatMul({},{},{}) compute temp_c=E*F, temp_c(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(temp_c));
      subtractVectors<mpc_t>(c, temp_c, c, size);
      AUDIT("id:{}, P{} MatMul({},{},{}) compute c=c-temp_c, c(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(c));
    }

    Truncate(c, GetMpcContext()->FLOAT_PRECISION, size, PARTY_A, PARTY_B, partyNum);
  }
 
  AUDIT("id:{}, P{} MatMul({},{},{}), output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, common_dim, columns, Vector<mpc_t>(c));
  tlog_debug << "MatMul ok.";
  return 0;
}

int SnnInternal::MatMul(
  const vector<string>& as,
  const vector<string>& bs,
  vector<string>& cs,
  size_t rows,
  size_t common_dim,
  size_t columns,
  size_t transpose_a,
  size_t transpose_b) {
  vector<mpc_t> a, b, c;
  rosetta::convert::from_binary_str(as, a);
  rosetta::convert::from_binary_str(bs, b);

  int ret = MatMul(a, b, c, rows, common_dim, columns, transpose_a, transpose_b);
  if (ret != 0) {
    tlog_error << "MatMul failed!";
    return -1;
  }

  convert_mpctype_to_string(c, cs);
  return 0;
}

///////////     select-share    /////////
// 3PC SelectShares
// a,b,c are shared across PARTY_A, PARTY_B
int SnnInternal::SelectShares(
  const vector<mpc_t>& a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c) {
  assert(THREE_PC && "SelectShares called in non-3PC mode");
  tlog_debug << "SelectShares ...";
  AUDIT("id:{}, P{} SelectShares, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  DotProduct(a, b, c);
  AUDIT("id:{}, P{} SelectShares, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));
  tlog_debug << "SelectShares ok.";
  return 0;
}

int SnnInternal::Select1Of2(
  const vector<mpc_t> shared_x,
  const vector<mpc_t> shared_y,
  const vector<mpc_t> shared_bool,
  vector<mpc_t>& shared_result) {
  tlog_debug << "Select1Of2 ...";
  AUDIT("id:{}, P{} SelectShares, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_x));
  AUDIT("id:{}, P{} SelectShares, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_y));
  AUDIT("id:{}, P{} SelectShares, input B(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_bool));

  size_t size = shared_x.size();
  vector<mpc_t> res_prod(size, 0);
  vector<mpc_t> x_minus_y(size, 0);
  subtractVectors(shared_x, shared_y, x_minus_y, size);
  AUDIT("id:{}, P{} SelectShares compute: Z=X-Y, Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(x_minus_y));

  DotProduct(x_minus_y, shared_bool, res_prod);
  AUDIT("id:{}, P{} SelectShares compute: prod=Z*B, prod(=Z*B=(X-Y)*B)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(res_prod));

  addVectors<mpc_t>(shared_y, res_prod, shared_result, size);
  AUDIT("id:{}, P{} SelectShares compute: output=Y*pord, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_result));

  tlog_debug << "Select1Of2 ok.";
  return 0;
}

int SnnInternal::XorBit(
  const vector<mpc_t> shared_x,
  const vector<mpc_t> shared_y,
  vector<mpc_t>& shared_result) {
  tlog_debug << "XorBit ...";
  AUDIT("id:{}, P{} XorBit, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_x));
  AUDIT("id:{}, P{} XorBit, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_y));

  size_t size = shared_x.size();
  vector<mpc_t> shared_sum(size, 0);
  addVectors<mpc_t>(shared_x, shared_y, shared_sum, size);
  AUDIT("id:{}, P{} XorBit compute sum=X+Y, sum(=X+Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_sum));

  vector<mpc_t> shared_prod(size, 0);
  DotProduct(shared_x, shared_y, shared_prod);
  AUDIT("id:{}, P{} XorBit compute prod=X*Y, prod(=X*Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_prod));
  if (PRIMARY) {
    for (int i = 0; i < size; ++i) {
      shared_prod[i] = shared_prod[i] << 1;
    }
  }
  subtractVectors(shared_sum, shared_prod, shared_result, size);

  AUDIT("id:{}, P{} XorBit, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_result));
  tlog_debug << "XorBit ok.";
  return 0;
}

int SnnInternal::DotProduct(
  const vector<string>& const_a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c) {
  tlog_debug << "DotProduct lh_is_const ...";

  size_t size = const_a.size();
  c.resize(size);
  int float_precision = GetMpcContext()->FLOAT_PRECISION;

  vector<double> da(size);
  vector<mpc_t> plain_a(size, 0);
  rosetta::convert::from_double_str(const_a, da);
  convert_double_to_mpctype(da, plain_a, GetMpcContext()->FLOAT_PRECISION);
  AUDIT("id:{}, P{} DotProduct lh_is_const, input X(double){}", msg_id().get_hex(), context_->GetMyRole(), Vector<double>(da));
  AUDIT("id:{}, P{} DotProduct lh_is_const, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));
  
  if (PRIMARY)
  {
    for (size_t i = 0; i < size; i++) {
      c[i] = plain_a[i] * b[i];
    }

    Truncate(c, GetMpcContext()->FLOAT_PRECISION, size, PARTY_A, PARTY_B, partyNum);
  }
  
  AUDIT("id:{}, P{} DotProduct lh_is_const, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "DotProduct lh_is_const ok.";
  return 0;
}

int SnnInternal::DotProduct(
  const vector<double>& const_a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c) {
  size_t size = const_a.size();
  c.resize(size);
  int float_precision = GetMpcContext()->FLOAT_PRECISION;
  AUDIT("id:{}, P{} DotProduct lh_is_const_double, input X(double){}", msg_id().get_hex(), context_->GetMyRole(), Vector<double>(const_a));
  AUDIT("id:{}, P{} DotProduct lh_is_const_double, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));

  vector<mpc_t> plain_a(size);
  convert_double_to_mpctype(const_a, plain_a, GetMpcContext()->FLOAT_PRECISION);

  if (PRIMARY) {
    for (size_t i = 0; i < size; i++) {
      c[i] = plain_a[i] * b[i];
    }

    Truncate(c, GetMpcContext()->FLOAT_PRECISION, size, PARTY_A, PARTY_B, partyNum);
  }

  AUDIT("id:{}, P{} DotProduct lh_is_const_double, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  return 0;
}

int SnnInternal::DotProduct(
  const vector<mpc_t>& a,
  const vector<string>& const_b,
  vector<mpc_t>& c) {
  return DotProduct(const_b, a, c);
}

int SnnInternal::DotProduct(
  const vector<mpc_t>& a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c) {
  assert(THREE_PC && "DotProduct called in non-3PC mode");
  tlog_debug << "DotProduct ...";
  AUDIT("id:{}, P{} DotProduct, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  AUDIT("id:{}, P{} DotProduct, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));

  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
  const int size = a.size();
  c.resize(size);

  vector<mpc_t> A(size, 0), B(size, 0), C(size, 0);

#if ENABLE_OFFLINE_TRIPLES_MODE
  tlog_debug << "SnnIternal Using offline MUL-triples";
  _triple_generator->get_mul_triple(_op_msg_id, size, A, B, C);
  tlog_debug << "SnnInternal get_mul_triple DONE";
  AUDIT("id:{}, P{} DotProduct get_triple_a, A(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(A));
  AUDIT("id:{}, P{} DotProduct get_triple_b, B(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(B));
  AUDIT("id:{}, P{} DotProduct get_triple_c, C(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(C));
#else
    tlog_debug << "Using online MUL-triples";
  if (HELPER) {
    vector<mpc_t> A1(size, 0), A2(size, 0), B1(size, 0), B2(size, 0), C1(size, 0), C2(size, 0);

    populateRandomVector<mpc_t>(A1, size, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} DotProduct populateRandomVector A1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(A1));

    populateRandomVector<mpc_t>(A2, size, "a_2", "POSITIVE");
    AUDIT("id:{}, P{} DotProduct populateRandomVector A2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(A2));

    populateRandomVector<mpc_t>(B1, size, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} DotProduct populateRandomVector B1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(B1));

    populateRandomVector<mpc_t>(B2, size, "a_2", "POSITIVE");
    AUDIT("id:{}, P{} DotProduct populateRandomVector B2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(B2));

    populateRandomVector<mpc_t>(C1, size, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} DotProduct populateRandomVector C1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(C1));

    addVectors<mpc_t>(A1, A2, A, size);
    AUDIT("id:{}, P{} DotProduct compute: A=A1+A2, B(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(A));

    addVectors<mpc_t>(B1, B2, B, size);
    AUDIT("id:{}, P{} DotProduct compute: B=B1+B2, B(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(B));

    for (size_t i = 0; i < size; ++i) {
      C[i] = A[i] * B[i];
    }
    AUDIT("id:{}, P{} DotProduct compute: C=A*B, C(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(C));

    subtractVectors<mpc_t>(C, C1, C2, size);
    AUDIT("id:{}, P{} DotProduct compute: C2=C-C1, C2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(C2));

    tlog_debug << "---> P2 send C2 to P1 ....";
    sendVector<mpc_t>(C2, PARTY_B, size);
    AUDIT("id:{}, P{} DotProduct SEND to P{}, C2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(C2));
  }

  if (PRIMARY) {
    if (partyNum == PARTY_A) {
      populateRandomVector<mpc_t>(A, size, "a_1", "POSITIVE");
      AUDIT("id:{}, P{} DotProduct populateRandomVector A1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(A));

      populateRandomVector<mpc_t>(B, size, "a_1", "POSITIVE");
      AUDIT("id:{}, P{} DotProduct populateRandomVector B1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(B));

      populateRandomVector<mpc_t>(C, size, "a_1", "POSITIVE");
      AUDIT("id:{}, P{} DotProduct populateRandomVector C1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(C));
    }

    if (partyNum == PARTY_B) {
      populateRandomVector<mpc_t>(A, size, "a_2", "POSITIVE");
      AUDIT("id:{}, P{} DotProduct populateRandomVector A2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(A));

      populateRandomVector<mpc_t>(B, size, "a_2", "POSITIVE");
      AUDIT("id:{}, P{} DotProduct populateRandomVector B2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(B));

      tlog_debug << "---> P1 recv C2 from P2 ....";
      receiveVector<mpc_t>(C, PARTY_C, size);
      AUDIT("id:{}, P{} DotProduct RECV from P{}, C2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_C, Vector<mpc_t>(C));
      //cout << "partyNum " << partyNum << ",  C[0]:" << C[0] << endl;
    }
  }
#endif
    if (PRIMARY) {
    // receiveThreeVectors<mpc_t>(A, B, C, PARTY_C, size, size, size);
    vector<mpc_t> E(size), F(size), temp_E(size), temp_F(size);
    mpc_t temp;

    subtractVectors<mpc_t>(a, A, E, size);
    AUDIT("id:{}, P{} DotProduct compute: E=X-A, E(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(E));

    subtractVectors<mpc_t>(b, B, F, size);
    AUDIT("id:{}, P{} DotProduct compute: F=Y-B, F(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(F));
    tlog_debug << "dot-product to recovery E, F ...";
#if 1
    thread* threads = new thread[2];
    threads[0] = thread(
      &SnnInternal::sendTwoVectors<mpc_t>, this, ref(E), ref(F), adversary(partyNum), size, size);
    AUDIT("id:{}, P{} DotProduct SEND to P{}, E(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<mpc_t>(E));
    AUDIT("id:{}, P{} DotProduct SEND to P{}, F(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<mpc_t>(F));

    threads[1] = thread(
      &SnnInternal::receiveTwoVectors<mpc_t>, this, ref(temp_E), ref(temp_F), adversary(partyNum),
      size, size);
    AUDIT("id:{}, P{} DotProduct RECV from P{}, temp_E(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<mpc_t>(temp_E));
    AUDIT("id:{}, P{} DotProduct RECV from P{}, temp_F(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<mpc_t>(temp_F));

    for (int i = 0; i < 2; i++)
      threads[i].join();

    delete[] threads;
#else
    if (partyNum == PARTY_A) {
      sendTwoVectors<mpc_t>(ref(E), ref(F), adversary(partyNum), size, size);
      receiveTwoVectors<mpc_t>(ref(temp_E), ref(temp_F), adversary(partyNum), size, size);
    } else {
      sendTwoVectors<mpc_t>(ref(E), ref(F), adversary(partyNum), size, size);
      receiveTwoVectors<mpc_t>(ref(temp_E), ref(temp_F), adversary(partyNum), size, size);
      // sendTwoVectors<mpc_t>(ref(E), ref(F), adversary(partyNum), size, size);
    }
#endif
    tlog_debug << "dot-product recovery E, F   OK.";
    
    addVectors<mpc_t>(E, temp_E, E, size);
    AUDIT("id:{}, P{} DotProduct compute: E=E+temp_E, E(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(E));

    addVectors<mpc_t>(F, temp_F, F, size);
    AUDIT("id:{}, P{} DotProduct compute: F=F+temp_F, F(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(F));

    for (size_t i = 0; i < size; ++i) {
      c[i] = a[i] * F[i];
      mpc_t temp = E[i] * b[i];
      c[i] = c[i] + temp;

      if (partyNum == PARTY_A) {
        temp = E[i] * F[i];
        c[i] = c[i] - temp;
      }
    }
    addVectors<mpc_t>(c, C, c, size);
    AUDIT("id:{}, P{} DotProduct c(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
#if FIX_SHARE_TRUNCATION_PROBABILISTIC_ERROR
    // Fix the serious truncation error by restricting any shares <X_0, X_1>
    // to <X-r, r> s.t r \in [2^{-62}, 2^{62}].
    //  In this way, the serious error that MSB(X) = b and MSB<X-r> == MSB<r> == (1-b)
    // can be avoided!
    // check section5.1.1 in ABY3 paper
    vector<mpc_t> before_c = c;
    vector<mpc_t> tmp_rand_share(size);
    if (partyNum == PARTY_B) {
      populateRandomVector<mpc_t>(tmp_rand_share, size, "INDEP", "POSITIVE");
      AUDIT("id:{}, P{} DotProduct populateRandomVector tmp_rand_share(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(tmp_rand_share));

      for (int fix_iter = 0; fix_iter < size; ++fix_iter) {
        // make sure r \in [2^{-62}, 2^{62}
        tmp_rand_share[fix_iter] = (mpc_t)((signed_mpc_t)tmp_rand_share[fix_iter] >> 1);
      }
      subtractVectors(c, tmp_rand_share, c, size);
      AUDIT("id:{}, P{} DotProduct compute: c=c-tmp_rand_share, c(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));

      sendVector<mpc_t>(c, PARTY_A, size);
      AUDIT("id:{}, P{} DotProduct SEND to P{}, c(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<mpc_t>(c));
      c = tmp_rand_share;
    } else if (partyNum == PARTY_A) {
      receiveVector<mpc_t>(tmp_rand_share, PARTY_B, size);
      AUDIT("id:{}, P{} DotProduct RECV from P{}, tmp_rand_share(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<mpc_t>(c));
      addVectors<mpc_t>(c, tmp_rand_share, c, size);
      AUDIT("id:{}, P{} DotProduct compute: c=c+tmp_rand_share, c(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
    }
#endif

    Truncate(c, float_precision, size, PARTY_A, PARTY_B, partyNum);

// just for debuging
#if MPC_DEBUG
    for (int i = 0; i < size; ++i) {
      cout << i << "-th context:" << endl;
      cout << " local [signed] a:" << (signed_mpc_t)a[i];
      cout << ", bit-presentation: ";
      for (int j = sizeof(mpc_t) * 8 - 1; j >= 0; --j) {
        cout << ((a[i] >> j) & 0x01);
      }
      cout << endl;

      cout << " local b:" << (signed_mpc_t)b[i];
      cout << ", bit-presentation: ";
      for (int j = sizeof(mpc_t) * 8 - 1; j >= 0; --j) {
        cout << ((b[i] >> j) & 0x01);
      }
      cout << endl;

      cout << " local masking triple random:" << endl;
      cout << " A: " << (signed_mpc_t)A[i];
      cout << ", bit-presentation: ";
      for (int j = sizeof(mpc_t) * 8 - 1; j >= 0; --j) {
        cout << ((A[i] >> j) & 0x01);
      }
      cout << endl;

      cout << " B: " << (signed_mpc_t)B[i];
      cout << ", bit-presentation: ";
      for (int j = sizeof(mpc_t) * 8 - 1; j >= 0; --j) {
        cout << ((B[i] >> j) & 0x01);
      }
      cout << endl;
      cout << " C: " << (signed_mpc_t)C[i];
      cout << ", bit-presentation: ";
      for (int j = sizeof(mpc_t) * 8 - 1; j >= 0; --j) {
        cout << ((C[i] >> j) & 0x01);
      }
      cout << endl;

      cout << "Mul result::::" << endl;
      cout << " Local share BEFORE truncation:" << (signed_mpc_t)before_c[i];
      cout << ", bit-presentation: ";
      for (int j = sizeof(mpc_t) * 8 - 1; j >= 0; --j) {
        cout << ((before_c[i] >> j) & 0x01);
      }
      cout << endl;
      cout << " Local share AFTER truncation:" << (signed_mpc_t)c[i];
      cout << ", bit-presentation: ";
      for (int j = sizeof(mpc_t) * 8 - 1; j >= 0; --j) {
        cout << ((c[i] >> j) & 0x01);
      }
      cout << endl;
    }

  } else {
    for (int i = 0; i < size; ++i) {
      cout << i << "-th context:" << endl;
      cout << " local masking triple random:" << endl;
      cout << " A: " << (signed_mpc_t)A[i];
      cout << ", bit-presentation: ";
      for (int j = sizeof(mpc_t) * 8 - 1; j >= 0; --j) {
        cout << ((A[i] >> j) & 0x01);
      }
      cout << endl;

      cout << " B: " << (signed_mpc_t)B[i];
      cout << ", bit-presentation: ";
      for (int j = sizeof(mpc_t) * 8 - 1; j >= 0; --j) {
        cout << ((B[i] >> j) & 0x01);
      }
      cout << endl;
      cout << " C: " << (signed_mpc_t)C[i];
      cout << ", bit-presentation: ";
      for (int j = sizeof(mpc_t) * 8 - 1; j >= 0; --j) {
        cout << ((C[i] >> j) & 0x01);
      }
      cout << endl;
    }
#endif
  }

  AUDIT("id:{}, P{} DotProduct output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "DotProduct ok.";
  return 0;
}

int SnnInternal::BitMul(
  const vector<small_mpc_t>& a,
  const vector<small_mpc_t>& b,
  vector<small_mpc_t>& c) {
  tlog_debug << "BitMul ...";
  AUDIT("id:{}, P{} BitMul, input X(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(a));
  AUDIT("id:{}, P{} BitMul, input Y(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(b));

  size_t size = a.size();
  if (THREE_PC) {
    vector<small_mpc_t> A(size, 0), B(size, 0), C(size, 0);

    if (HELPER) {
      vector<small_mpc_t> A1(size, 0), A2(size, 0), B1(size, 0), B2(size, 0), C1(size, 0),
        C2(size, 0);
      // We only use the last bit!
      populateRandomVector<small_mpc_t>(A1, size, "a_1", "POSITIVE");
      AUDIT("id:{}, P{} BitMul populateRandomVector A1(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(A1));

      populateRandomVector<small_mpc_t>(A2, size, "a_2", "POSITIVE");
      AUDIT("id:{}, P{} BitMul populateRandomVector A2(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(A2));

      populateRandomVector<small_mpc_t>(B1, size, "b_1", "POSITIVE");
      AUDIT("id:{}, P{} BitMul populateRandomVector B1(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(B1));

      populateRandomVector<small_mpc_t>(B2, size, "b_2", "POSITIVE");
      AUDIT("id:{}, P{} BitMul populateRandomVector B2(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(B2));
      populateRandomVector<small_mpc_t>(C1, size, "c_1", "POSITIVE");
      AUDIT("id:{}, P{} BitMul populateRandomVector C1(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(C1));

      for (int i = 0; i < size; ++i) {
        A[i] = A1[i] ^ A2[i] & 0x01;
        B[i] = B1[i] ^ B2[i] & 0x01;
        C[i] = A[i] & B[i] & 0x01;
        C2[i] = C[i] ^ C1[i] & 0x01;
      }
      sendBitVector(C2, PARTY_B, size);
      AUDIT("id:{}, P{} SEND to P{}, C2(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<small_mpc_t>(C2));
    }

    if (PRIMARY) {
      if (partyNum == PARTY_A) {
        populateRandomVector<small_mpc_t>(A, size, "a_1", "POSITIVE");
        AUDIT("id:{}, P{} BitMul populateRandomVector A1(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(A));

        populateRandomVector<small_mpc_t>(B, size, "b_1", "POSITIVE");
        AUDIT("id:{}, P{} BitMul populateRandomVector B1(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(B));

        populateRandomVector<small_mpc_t>(C, size, "c_1", "POSITIVE");
        AUDIT("id:{}, P{} BitMul populateRandomVector C1(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(C));
      }

      if (partyNum == PARTY_B) {
        populateRandomVector<small_mpc_t>(A, size, "a_2", "POSITIVE");
        AUDIT("id:{}, P{} BitMul populateRandomVector A2(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(A));

        populateRandomVector<small_mpc_t>(B, size, "b_2", "POSITIVE");
        AUDIT("id:{}, P{} BitMul populateRandomVector B2(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(B));

        receiveBitVector(C, PARTY_C, size);
        AUDIT("id:{}, P{} BitMul RECV from P{}, C2(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_C,Vector<small_mpc_t>(C));
        //cout << "partyNum " << partyNum << ",  C[0]:" << C[0] << endl;
      }

      // receiveThreeVectors<mpc_t>(A, B, C, PARTY_C, size, size, size);
      vector<small_mpc_t> E(size), F(size), temp_E(size), temp_F(size);

      for (int i = 0; i < size; ++i) {
        E[i] = a[i] ^ A[i] & 0x01;
        F[i] = b[i] ^ B[i] & 0x01;
      }

      thread* threads = new thread[2];
      threads[0] = thread(
        &SnnInternal::sendTwoBitVector, this, ref(E), ref(F), adversary(partyNum), size, size);
      AUDIT("id:{}, P{} BitMul SEND to P{}, E(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<small_mpc_t>(E));
      AUDIT("id:{}, P{} BitMul SEND to P{}, F(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<small_mpc_t>(F));

      threads[1] = thread(
        &SnnInternal::receiveTwoBitVector, this, ref(temp_E), ref(temp_F), adversary(partyNum), size,
        size);
      AUDIT("id:{}, P{} BitMul RECV from P{}, E(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<small_mpc_t>(E));
      AUDIT("id:{}, P{} BitMul RECV from P{}, F(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<small_mpc_t>(F));

      for (int i = 0; i < 2; i++)
        threads[i].join();

      delete[] threads;

      for (int i = 0; i < size; ++i) {
        E[i] = E[i] ^ temp_E[i] & 0x01;
        F[i] = F[i] ^ temp_F[i] & 0x01;
      }

      for (size_t i = 0; i < size; ++i) {
        c[i] = a[i] & F[i] & 0x01;
        small_mpc_t temp = E[i] & b[i] & 0x01;
        c[i] = c[i] ^ temp & 0x01;

        if (partyNum == PARTY_A) {
          temp = E[i] & F[i] & 0x01;
          c[i] = c[i] ^ temp & 0x01;
        }
        c[i] = c[i] ^ C[i] & 0x01;
      }
    }
  }

  AUDIT("id:{}, P{} BitMul, output(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(c));
  tlog_debug << "BitMul ok.";
  return 0;
}

int SnnInternal::LinearMPC(
  const vector<mpc_t>& x,
  double a,
  double b,
  vector<mpc_t>& out) {
  tlog_debug << "LinearMPC ...";
  AUDIT("id:{}, P{} LinerMPC_b, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(x));
  AUDIT("id:{}, P{} LinerMPC_b, input a(dobule): {}", msg_id().get_hex(), context_->GetMyRole(), a);
  AUDIT("id:{}, P{} LinerMPC_b, input b(double): {}", msg_id().get_hex(), context_->GetMyRole(), b);

  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
  mpc_t sa = FloatToMpcType(a, float_precision);
  mpc_t sb = 0;
  if (partyNum == PARTY_A) {
    sb = FloatToMpcType(b, float_precision);
  }

  size_t size = x.size();
  out.resize(size);
  for (size_t i = 0; i < size; i++) {
    out[i] = sa * x[i];
    Truncate(out[i], float_precision, PARTY_A, PARTY_B, partyNum);
    out[i] += sb;
  }

  AUDIT("id:{}, P{} LinerMPC_b, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(out));
  tlog_debug << "LinearMPC ok.";
  return 0;

}

int SnnInternal::LinearMPC(
  const vector<mpc_t>& x,
  mpc_t a,
  mpc_t b,
  vector<mpc_t>& out) {
  tlog_debug << "LinearMPC ...";
  AUDIT("id:{}, P{} LinerMPC, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(x));
  AUDIT("id:{}, P{} LinerMPC, input a(mpc_t): {}", msg_id().get_hex(), context_->GetMyRole(), a);
  AUDIT("id:{}, P{} LinerMPC, input b(mpc_t): {}", msg_id().get_hex(), context_->GetMyRole(), b);

  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
  mpc_t sb = 0;
  if (partyNum == PARTY_A) {
    sb = b;
  }

  size_t size = x.size();
  out.resize(size);
  for (size_t i = 0; i < size; i++) {
    out[i] = a * x[i];
    Truncate(out[i], float_precision, PARTY_A, PARTY_B, partyNum);
    out[i] = out[i] + sb;
  }

  AUDIT("id:{}, P{} LinerMPC, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(out));
  tlog_debug << "LinearMPC ok.";
  return 0;
}

}//snn
}//rosetta
