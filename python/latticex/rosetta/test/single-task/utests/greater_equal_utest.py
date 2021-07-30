import unittest
from decorator_utest_suite_ import MpcBinaryUnitTester
from decorator_utest_cases_ import *


class GreaterEqualUtest(MpcBinaryUnitTester):
    def __init__(self, testName):
        MpcBinaryUnitTester.__init__(self, testName)
        op_name = 'SecureGreaterEqual'
        self.setOpName(op_name)
        print("test {}".format(op_name))
  
    def test_sample0(self):
        test_binary_sample_0(self)

    def test_sample1(self):
        test_binary_sample_1(self)

    def test_sample2(self):
        test_binary_sample_2(self)

    def test_sample3(self):
        test_binary_sample_3(self)

    def test_sample4(self):
        test_binary_sample_4(self)
        
    def test_const_sample0(self, lh_is_const=False, rh_is_const=True):
        test_binary_const_sample_4(self, lh_is_const=lh_is_const, rh_is_const=rh_is_const)
    
if __name__ == '__main__':
    unittest.main()
    # suite = unittest.TestLoader().loadTestsFromTestCase(EqualUtest)
    # unittest.TextTestRunner(verbosity=3).run(suite)
