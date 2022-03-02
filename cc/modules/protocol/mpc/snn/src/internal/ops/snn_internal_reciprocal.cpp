#include "cc/modules/protocol/mpc/snn/include/snn_internal.h"
#include "cc/modules/common/include/utils/str_type_convert.h"
#include <thread>

using std::thread;

namespace rosetta {
namespace snn {

int SnnInternal::Reciprocal(const vector<mpc_t>& a, vector<mpc_t>& c){

    log_debug << "Reciprocal...";

    Reciprocal_Goldschmidt(a,  c);

    log_debug << "Reciprocal ok.";
    return 0;


  }

int SnnInternal::Reciprocal(const vector<string>& a, vector<mpc_t>& c){

size_t size = a.size();
int float_precision = GetMpcContext()->FLOAT_PRECISION;
 vector<mpc_t> Denominator(size, 0);
 if (partyNum == PARTY_A) {
    vector<double> da(size, 0.0);
    rosetta::convert::from_double_str(a, da);
    convert_double_to_mpctype(da, Denominator, GetMpcContext()->FLOAT_PRECISION);
  }
return Reciprocal( Denominator, c);

  }

int SnnInternal::Reciprocal_Goldschmidt(const vector<mpc_t>& shared_denominator_vec,  vector<mpc_t>& shared_quotient_vec){

  log_debug << "Reciprocal ...";
 size_t vec_size = shared_denominator_vec.size();

if(THREE_PC){

vector<mpc_t> shared_denom_sign(vec_size, 0);
ComputeMSB(shared_denominator_vec, shared_denom_sign);

vector<mpc_t> shared_sign_pos(vec_size, 0);
    if (partyNum == PARTY_A) {
        shared_sign_pos = vector<mpc_t>(vec_size, FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION));
      }
      vector<mpc_t> shared_sign_neg(vec_size, 0);
     if (partyNum == PARTY_A) {
        shared_sign_neg = vector<mpc_t>(vec_size, FloatToMpcType(-1, GetMpcContext()->FLOAT_PRECISION));
      }
vector<mpc_t>denominator_vec(vec_size, 0);
vector<mpc_t>quo_sign(vec_size,0);
Select1Of2(shared_sign_neg, shared_sign_pos, shared_denom_sign, quo_sign);
DotProduct(shared_denominator_vec,quo_sign,denominator_vec);

///we assume that the number of bits is  16, we can set lamda as fangsuoyinzi.
    vector<mpc_t> lamda(vec_size, 0);
    if(partyNum == PARTY_A) {
      lamda = vector<mpc_t>(vec_size, FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION));
    }
    vector<mpc_t> fixed_lamda(vec_size, 0);
    if(partyNum == PARTY_A) {
      lamda = vector<mpc_t>(vec_size, FloatToMpcType(0.0675, GetMpcContext()->FLOAT_PRECISION));
    }
    vector<mpc_t> choose_lamda(vec_size,0);
    vector<mpc_t> lamda_update(vec_size,0);
    
    vector<mpc_t>   shared_two(vec_size, 0);
    if(partyNum == PARTY_A) {
     lamda = vector<mpc_t>(vec_size, FloatToMpcType(2, GetMpcContext()->FLOAT_PRECISION));
    }
    vector<mpc_t>   shared_one(vec_size, 0);
    if(partyNum == PARTY_A) {
     shared_one = vector<mpc_t>(vec_size, FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION));
    }
    vector<mpc_t> compare_two_result(vec_size,0);
    vector<mpc_t> den_temp(vec_size,0);
    den_temp = denominator_vec;
    vector<mpc_t> den_temp_update(vec_size,0);



    for(int i = 1 ; i <= 4 ;i++ ){
      subtractVectors<mpc_t>(den_temp,shared_two,compare_two_result,vec_size);
      ComputeMSB(compare_two_result,choose_lamda);

      DotProduct(den_temp,fixed_lamda,den_temp_update);
      Select1Of2(den_temp,den_temp_update,choose_lamda,den_temp);

      DotProduct(lamda,fixed_lamda,lamda_update);
      Select1Of2(lamda,lamda_update,choose_lamda,lamda);
    }
    DotProduct(denominator_vec,lamda,denominator_vec);

vector<mpc_t> numer(vec_size,0);//the numer of reciprocal is 1
if(partyNum == PARTY_A) {
  numer = vector<mpc_t>(vec_size, FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION));
}
vector<mpc_t> y(vec_size,0);//d  = 1 + y
vector<mpc_t> factor_m(vec_size,0);//m = 1 -y

///initial
subtractVectors<mpc_t>(denominator_vec,shared_one,y, vec_size);
subtractVectors<mpc_t>(shared_one,y,factor_m, vec_size);

 for(int i = 1 ; i <= 8 ;i++ ){
   DotProduct(factor_m,numer,numer);
   DotProduct(factor_m,denominator_vec,denominator_vec);
   subtractVectors<mpc_t>(shared_two,denominator_vec,factor_m, vec_size);
 }
 vector<mpc_t> result(vec_size,0);
DotProduct(numer,lamda,result);
DotProduct(result,quo_sign,shared_quotient_vec);


}//three pc
  log_debug << "Reciprocal ok.";
  return 0;
}

}//snn
}//rosetta
