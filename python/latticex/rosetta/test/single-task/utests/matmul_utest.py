import unittest
from decorator_utest_suite_ import MpcBinaryUnitTester
from decorator_utest_cases_ import *


class MatMulUtest(MpcBinaryUnitTester):
    def __init__(self, testName):
        MpcBinaryUnitTester.__init__(self, testName)
        op_name = 'SecureMatMul'
        self.setOpName(op_name)
        print("test {}".format(op_name))
      
    # '''
    # test all sample data
    # '''

    def test_sample4(self):
        test_binary_sample_4(self)
        
    def test_my_case1(self):
        in0 = [[[1, 2], [3, 4]], [[2, 4], [6, 8]]]
        in1 = [[[0, 0], [0, 0]], [[0, 0], [0, 0]]]
        outputs = self.run_players(in0, in1, self.getOpName())
        expect_outputs = self.run_local(in0, in1, self.getOpName())# 0: lh_isconst=True, 1: rh_isconst=True, -1: default and both False
        
        print("expect-outputs: {}, type: {}".format(expect_outputs, type(expect_outputs)))
        print("mpc-outputs: {}, type: {}".format(outputs, type(outputs)))
        if np.allclose(outputs, expect_outputs, rtol=0, atol=0.1) == False: #
            self.assertEqual(outputs, expect_outputs)
        print("--------- test {}: sample in0: {}, in1: {} done".format(self.getOpName(), in0, in1))
    
if __name__ == '__main__':
    unittest.main()
    # suite = unittest.TestLoader().loadTestsFromTestCase(EqualUtest)
    # unittest.TextTestRunner(verbosity=3).run(suite)
