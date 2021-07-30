#coding=utf-8
import unittest
import os

# pattern or test_dir could change
test_dir = './utests/'
test_pattern = '*_utest.py'
discover = unittest.defaultTestLoader.discover(start_dir=test_dir, pattern=test_pattern)

if __name__ == '__main__':
    runner = unittest.TextTestRunner()
    runner.run(discover)
