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
        vector<mpc_t> rsqrt_nr_initial ;
        
        //const vector<mpc_t> b(1,2);
        //GetMpcOpInner(FloorDivision)->Run(a, b, rsqrt_nr_initial, 1);
        const vector<mpc_t> one_half(size,FloatToMpcType(0.5));
        const vector<mpc_t> one_fifth(size,FloatToMpcType(0.2));
        const vector<mpc_t> zero(size,FloatToMpcType(0));
        const vector<mpc_t> two_pounds_two(size,FloatToMpcType(2.2));
        const vector<mpc_t> number1(size,FloatToMpcType(1/1024));
        const vector<mpc_t> three(size,FloatToMpcType(3));

         //(mpc_t)0.5;
        //const mpc_t one = FloatToMpcType(1);
        
        GetMpcOpInner(DotProduct)->Run(a, one_half, rsqrt_nr_initial, size); //x/2
        addVectors<mpc_t>(rsqrt_nr_initial, one_fifth, rsqrt_nr_initial, size);//x/2+0.2
        subtractVectors<mpc_t>(zero, rsqrt_nr_initial, rsqrt_nr_initial, size);//-(x/2+0.2)
        vector<mpc_t> Inter_number1 ;
        GetMpcOpInner(Exp)->Run(rsqrt_nr_initial,rsqrt_nr_initial,size);//exp(-(x/2+0.2))
        GetMpcOpInner(DotProduct)->Run(rsqrt_nr_initial, two_pounds_two, rsqrt_nr_initial, size); //点乘2.2
        addVectors<mpc_t>(rsqrt_nr_initial, one_fifth, rsqrt_nr_initial, size);//再加0.2
        vector<mpc_t> Inter_number2;
        GetMpcOpInner(DotProduct)->Run(rsqrt_nr_initial, number1, Inter_number2, size); //计算rsqrt_nr_initial/1024
        subtractVectors<mpc_t>(rsqrt_nr_initial,Inter_number2, rsqrt_nr_initial, size);//rsqrt_nr_initial -=rsqrt_nr_initial/1024
        

         
         //for _ in range(config.sqrt_nr_iters):
        //y = y.mul_(3 - self * y.square()).div_(2)
        //return y
        for (int i = 0; i < sqrt_nr_iters; i++)
        {
            vector<mpc_t> squareInitial(size , 0);
            GetMpcOpInner(DotProduct)->Run(rsqrt_nr_initial, rsqrt_nr_initial, squareInitial, size);//平方
            vector<mpc_t> temp_number0,temp_number1,temp_number2;
            GetMpcOpInner(DotProduct)->Run(squareInitial, a, temp_number0, size);
            subtractVectors<mpc_t>(three,temp_number0, temp_number1, size);
            GetMpcOpInner(DotProduct)->Run(temp_number1,rsqrt_nr_initial , temp_number2, size);
            GetMpcOpInner(DotProduct)->Run(temp_number2,one_half,rsqrt_nr_initial , size);    
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
        GetMpcOpInner(Rsqrt)->Run(a,b , size);
        GetMpcOpInner(DotProduct)->Run(a,b, b, size);
    }
    return 0;
}

}}