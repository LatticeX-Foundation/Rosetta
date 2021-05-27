#include "cc/modules/protocol/mpc/snn/src/impl/op_impl.h"

#include <vector>
using namespace rosetta;
using namespace std;

int sqrt_nr_iters = 3;


//Computes the inverse square root of the input using the Newton-Raphson method.
namespace rosetta {
namespace snn {



int Rsqrt::funcRsqrt(const vector<mpc_t>& a ,vector<mpc_t>& b,size_t size)
{
    if(THREE_PC)
    {  
        vector<mpc_t> rsqrt_nr_initial = a ;

        const vector<mpc_t> one_fourth(size,FloatToMpcType(0.25));
        const vector<mpc_t> one_fifth(size,FloatToMpcType(0.2));
        const vector<mpc_t> one_pound_one(size,FloatToMpcType(1.1));
        const vector<mpc_t> milli(size, 0.0004883);
        const vector<mpc_t> three(size,FloatToMpcType(3));
        vector<mpc_t> inter_Number(size,0);

        GetMpcOpInner(DotProduct)->Run(a, one_fourth, rsqrt_nr_initial, size); 
        
        if(partyNum == PARTY_A)
            addVectors<mpc_t>(rsqrt_nr_initial, one_fifth, rsqrt_nr_initial, size);//x/2+0.2
        
        GetMpcOpInner(Negative)->Run(rsqrt_nr_initial,rsqrt_nr_initial,size);
        GetMpcOpInner(Exp)->Run(rsqrt_nr_initial,rsqrt_nr_initial,size);//exp(-(x/2+0.2))
        GetMpcOpInner(DotProduct)->Run(rsqrt_nr_initial, one_pound_one, rsqrt_nr_initial, size); 
        
        if(partyNum == PARTY_A)
            addVectors<mpc_t>(rsqrt_nr_initial, one_fifth, rsqrt_nr_initial, size);//add 0.2
        
        GetMpcOpInner(DotProduct)->Run(a, milli, inter_Number, size); //compute a/1024
        subtractVectors<mpc_t>(rsqrt_nr_initial,inter_Number, rsqrt_nr_initial, size);//rsqrt_nr_initial -=a/1024

        vector<mpc_t> temp_number0(size , 0),temp_number1(size , 0),temp_number2(size , 0);
        for (int i = 0; i < sqrt_nr_iters; i++)
        {
            GetMpcOpInner(DotProduct)->Run(rsqrt_nr_initial, rsqrt_nr_initial,temp_number1, size);
            GetMpcOpInner(DotProduct)->Run(temp_number1, a, temp_number0, size);//temp_number0 = a * rsqrt_nr_initial^2
            GetMpcOpInner(Negative)->Run(temp_number0,temp_number0,size); 
            if(partyNum == PARTY_B)
                addVectors<mpc_t>(temp_number0, three, temp_number0, size);
            GetMpcOpInner(DotProduct)->Run(temp_number0, one_fourth, temp_number0, size);
            GetMpcOpInner(DotProduct)->Run(temp_number0,rsqrt_nr_initial,temp_number2, size);    
            rsqrt_nr_initial = temp_number2 ;
        }     
        b = rsqrt_nr_initial;

    }
    return 0;
}

//sqrt
int Sqrt::funcSqrt(const vector<mpc_t>& a ,vector<mpc_t>& b,size_t size)
{
    if(THREE_PC)
    {
        vector<mpc_t> c(size , 0);
        GetMpcOpInner(Rsqrt)->Run(a,b, size);
        GetMpcOpInner(DotProduct)->Run(a,b,c , size);
        b = c ;
    }
    return 0;
}

}
}