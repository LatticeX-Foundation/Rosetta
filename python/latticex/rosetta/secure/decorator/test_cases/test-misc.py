#!/usr/bin/python
#coding: utf-8


from multiprocessing import Process
import os
import time
import sys

origin_stdout = None #sys.__stdout__
def redirect_to_file(name, rst=None):
    global original_stdout
    original_stdout = sys.stdout
    print(" before redirect stdout")
    sys.stdout = open('./log/{}.txt'.format(name), 'a')
    if rst:
        pass
        #rst.secure_player.redirect_stdout('./{}.txt'.format(name))
    #sys.stdout = open('./{}.txt'.format(name), 'a')
    print("redirect stdout to: {} done".format('./log/{}.txt'.format(name)))
    #sys.stdout = original

def restore_stdout():
    global origin_stdout
    sys.stdout = sys.__stdout__ #origin_stdout
    print("---- restore to stdout ---")
    
    
def info(title):
	print(title)
	print('module name:', __name__)
	print('parent process:', os.getppid())
	print('process id:', os.getpid())
	time.sleep(1)

def proc_func(name):
    if name == "alice":
        sys.argv.append("--party_id=0")
    elif name == "bob":
        sys.argv.append("--party_id=1")
    else:
        sys.argv.append("--party_id=2")

    # test import rosetta
    print("0------ import and redirect log ---------")
    import latticex.rosetta as cb
    redirect_to_file(name, cb)
    
    print("{}: ------ import and redirect log ok.---------".format(name))
    print("sys.argv: ", sys.argv)
    info('{}: proc_func ok.'.format(name))
    restore_stdout()
    print("restore stdout, exit sub proc!")

def test_sub_processes(count):
    info('main process')

    subprocs = []
    for i in range(count):
        p = Process(target=proc_func, args=('p'.format(i),))
        p.start()
        subprocs.append(p)

    for i in range(count):
        subprocs[i].join()
        print("sub-process: p{} joined".format(i))

    print("test_sub_processes end.")

if __name__ == '__main__':
    test_sub_processes(3)

    print("ending.")
