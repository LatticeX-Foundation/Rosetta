import unittest
from decorator_utest_suite_ import MpcUnaryUnitTester
import decorator_utest_cases_ as ucase


class MaxUtest(MpcUnaryUnitTester):
    def __init__(self, testName):
        MpcUnaryUnitTester.__init__(self, testName)
        op_name = 'SecureMax'
        self.setOpName(op_name)
        print("test {}".format(op_name))
      
    
    
    def test_sample0(self):
        ucase.test_unary_reduce_sample_0(self, is_reduce=True)

    def test_sample1(self):
        ucase.test_unary_reduce_sample_1(self, is_reduce=True)

    def test_sample2(self):
        ucase.test_unary_reduce_sample_2(self, is_reduce=True)

    def test_sample3(self):
        ucase.test_unary_reduce_sample_3(self, is_reduce=True)

    def test_sample4(self):
        ucase.test_unary_reduce_sample_4(self, is_reduce=True)
        
    def test_sample5(self):
        ucase.test_unary_reduce_sample_5(self, is_reduce=True)

    def test_sample6(self):
        ucase.test_unary_reduce_sample_6(self, is_reduce=True)

    def test_sample7(self):
        ucase.test_unary_reduce_sample_7(self, is_reduce=True)

    def test_sample8(self):
        ucase.test_unary_reduce_sample_8(self, is_reduce=True)

    def test_sample9(self):
        ucase.test_unary_reduce_sample_9(self, is_reduce=True)
    
if __name__ == '__main__':
    unittest.main()
    # suite = unittest.TestLoader().loadTestsFromTestCase(EqualUtest)
    # unittest.TextTestRunner(verbosity=3).run(suite)
