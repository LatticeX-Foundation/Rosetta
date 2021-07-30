import unittest
from decorator_utest_suite_ import MpcBinaryUnitTester
from decorator_utest_cases_ import *


class PowUtest(MpcBinaryUnitTester):
    def __init__(self, testName):
        MpcBinaryUnitTester.__init__(self, testName)
        op_name = 'SecurePow'
        self.setOpName(op_name)
        print("test {}".format(op_name))
    
    # def test_const_sample0(self): 
    #     # 5^0 == 1
    #     in1 = [2, 0]
    #     in2 = [3, 0]
    #     mpc_outputs = self.run_players(in1, in2, const_pos=1)
    #     local_outputs = self.run_local(in1, in2)
    
    #     print("inputs: in1: {}, in2: {}".format(in1, in2))
    #     print("expect: {}".format(local_outputs))
    #     print("mpc: {}".format(mpc_outputs))
    #     self.assertEqual(mpc_outputs.tolist(), local_outputs.tolist())

    def test_const_sample1(self):
        # 3^1, 1^0 == 3
        in1 = [3, 1]
        in2 = in1
        mpc_outputs = self.run_players(in1, in2, const_pos=1)
        local_outputs = self.run_local(in1, in2)
        print("inputs: in1: {}, in2: {}".format(in1, in2))
        print("expect: {}".format(local_outputs))
        print("mpc: {}".format(mpc_outputs))
        self.assertEqual(mpc_outputs, local_outputs)

    def test_const_sample2(self):
        # 3^2 == 9
        in1 = [3, 2]
        in2 = in1
        mpc_outputs = self.run_players(in1, in2, const_pos=1)
        local_outputs = self.run_local(in1, in2)
        print("inputs: in1: {}, in2: {}".format(in1, in2))
        print("expect: {}".format(local_outputs))
        print("mpc: {}".format(mpc_outputs))
        self.assertEqual(mpc_outputs, local_outputs)
        
    def test_const_sample3(self):
        # 3^3 == 9
        in1 = [3, 3]
        in2 = in1
        mpc_outputs = self.run_players(in1, in2, const_pos=1)
        local_outputs = self.run_local(in1, in2)
        print("inputs: in1: {}, in2: {}".format(in1, in2))
        print("expect: {}".format(local_outputs))
        print("mpc: {}".format(mpc_outputs))
        self.assertEqual(mpc_outputs, local_outputs)
    
if __name__ == '__main__':
    unittest.main()
    # suite = unittest.TestLoader().loadTestsFromTestCase(EqualUtest)
    # unittest.TextTestRunner(verbosity=3).run(suite)
