 #coding=utf-8
import unittest

# disable dynamic pass
import os
os.environ['ROSETTA_DPASS'] ='OFF'
os.environ['TEST_PROTOCOL'] ='SecureNN'

# pattern or test_dir could change
test_dir = './'
discover = unittest.defaultTestLoader.discover(test_dir, pattern='*_utest.py')

if __name__ == '__main__':
    runner = unittest.TextTestRunner()
    runner.run(discover)
    print('----- ending of all tests -----')
