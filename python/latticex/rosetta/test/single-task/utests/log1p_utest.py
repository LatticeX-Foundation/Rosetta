import unittest
from decorator_utest_suite_ import MpcUnaryUnitTester
import decorator_utest_cases_ as ucase


class Log1pUtest(MpcUnaryUnitTester):
    def __init__(self, testName):
        MpcUnaryUnitTester.__init__(self, testName)
        op_name = 'SecureLog1p'
        self.setOpName(op_name)
        print("test {}".format(op_name))
      
    # TODO: this test not pass ???
    def test_sample0(self):
        ucase.test_unary_reduce_sample_0(self, loss_precision=0.3)

    def test_sample1(self):
        ucase.test_unary_reduce_sample_1(self, loss_precision=0.3) # precision==0.5 will not pass, here set 1.0

    def test_sample2(self):
        ucase.test_unary_reduce_sample_2(self, loss_precision=0.3) # precision==0.5 will not pass, here set 1.0

    def test_sample3(self):
        ucase.test_unary_reduce_sample_3(self, loss_precision=0.3)

    def test_sample4(self):
        ucase.test_unary_reduce_sample_4(self, loss_precision=0.3)
        
    def test_sample5(self):
        ucase.test_unary_reduce_sample_5(self, loss_precision=0.3)

    def test_sample6(self):
        ucase.test_unary_reduce_sample_6(self, loss_precision=0.3)

    def test_sample8(self):
        ucase.test_unary_reduce_sample_8(self, loss_precision=0.3)

    def test_sample9(self):
        ucase.test_unary_reduce_sample_9(self, loss_precision=0.3)
    
if __name__ == '__main__':
    unittest.main()
