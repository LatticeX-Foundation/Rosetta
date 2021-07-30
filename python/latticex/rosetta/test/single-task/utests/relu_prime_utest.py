import unittest
from decorator_utest_suite_ import MpcUnaryUnitTester
import decorator_utest_cases_ as ucase
import numpy as np

class ReluPrimeUtest(MpcUnaryUnitTester):
    def __init__(self, testName):
        MpcUnaryUnitTester.__init__(self, testName)
        op_name = 'SecureReluPrime'
        self.setOpName(op_name)
        print("test {}".format(op_name))

    def test_sample0(self):
        ucase.test_unary_reduce_sample_0(self)

    def test_sample1(self):
        ucase.test_unary_reduce_sample_1(self)


    def test_sample2(self):
        ucase.test_unary_reduce_sample_2(self)

    def test_sample3(self):
        ucase.test_unary_reduce_sample_3(self)

    def test_sample4(self):
        ucase.test_unary_reduce_sample_4(self)

    def test_sample5(self):
        ucase.test_unary_reduce_sample_5(self)

    def test_sample6(self):
        ucase.test_unary_reduce_sample_6(self)

    def test_sample7(self):
        ucase.test_unary_reduce_sample_7(self)

    def test_sample8(self):
        ucase.test_unary_reduce_sample_8(self)

    def test_sample9(self):
        ucase.test_unary_reduce_sample_9(self)

    def test_sample10(self):
        in0 = [[1.0, -1.0, 0, 2]]
        in1 = in0
        zero_in1 = np.zeros([1,4])
        outputs = self.run_players(in0, in1, self.getOpName())
        expect_outputs = self.run_local(in0, zero_in1, self.getOpName())# 0: lh_isconst=True, 1: rh_isconst=True, -1: default and both False

        print("expect-outputs: {}, type: {}".format(expect_outputs, type(expect_outputs)))
        print("mpc-outputs: {}, type: {}".format(outputs, type(outputs)))
        
        if np.allclose(outputs, expect_outputs, rtol=0, atol=2) == False: #
            self.assertEqual(outputs.tolist(), expect_outputs.tolist())
        print("--------- test {}: sample in0: {}, in1: {} done".format(self.getOpName(), in0, in1))

if __name__ == '__main__':
    unittest.main()
    # suite = unittest.TestLoader().loadTestsFromTestCase(EqualUtest)
    # unittest.TextTestRunner(verbosity=3).run(suite)
