import time
import latticex.rosetta as rtt
from ResNet import ResNet
import argparse
from utils import *
import sys
sys.setrecursionlimit(3000)

"""parsing and configuration"""


def parse_args():
    desc = "Tensorflow implementation of ResNet"
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument('--phase', type=str,
                        default='train', help='train or test ?')
    parser.add_argument('--dataset', type=str, default='tiny',
                        help='cifar10, cifar100, mnist, fashion-mnist, tiny')

    parser.add_argument('--epoch', type=int, default=1,
                        help='The number of epochs to run')
    parser.add_argument('--batch_size', type=int, default=256,
                        help='The size of batch per gpu')
    parser.add_argument('--res_n', type=int, default=18,
                        help='18, 34, 50, 101, 152')

    parser.add_argument('--lr', type=float, default=0.1, help='learning rate')

    parser.add_argument('--checkpoint_dir', type=str, default='checkpoint',
                        help='Directory name to save the checkpoints')
    parser.add_argument('--log_dir', type=str, default='logs',
                        help='Directory name to save training logs')

    parser.add_argument('--train_size', type=int,
                        default=1, help='train data size')
    parser.add_argument('--test_size', type=int,
                        default=1, help='test data size')

    parser.add_argument('--input_public', action='store_true',
                        help="input is public?", default=False)
    parser.add_argument('--model_public', action='store_true',
                        help="load plain model as public?", default=False)

    parser.add_argument('--loops', type=int,
                        default=1, help='how many test loop')

    args, unparsed = parser.parse_known_args()
    return check_args(args, unparsed)


"""checking arguments"""


def check_args(args, unparsed):
    print(args)
    print(args.train_size)
    print(args.test_size)
    print(args.dataset)
    print(args.phase)
    print(args.lr)
    print(args.res_n)
    print(args.epoch)
    print(args.batch_size)

    # --checkpoint_dir
    check_folder(args.checkpoint_dir)

    # --result_dir
    check_folder(args.log_dir)

    # --epoch
    try:
        assert args.epoch >= 1
    except:
        print('number of epochs must be larger than or equal to one')

    # --batch_size
    try:
        assert args.batch_size >= 1
    except:
        print('batch size must be larger than or equal to one')
    return args


"""main"""


def main():
    # parse arguments
    args = parse_args()
    if args is None:
        exit()

    if args.model_public:
        rtt.set_restore_model(False)
    else:
        rtt.set_restore_model(False, plain_model='P0')

    # open session
    # with tf.Session(config=tf.ConfigProto(allow_soft_placement=True)) as sess:
    with tf.Session() as sess:
        start_time = time.time()
        cnn = ResNet(sess, args)

        # build graph
        cnn.build_model()
        print("pystats build_model elapse:{0} s".format(
            time.time() - start_time))

        # show network architecture
        show_all_variables()

        # if args.phase == 'train':
        #     # launch the graph in a session
        #     cnn.train()

        #     print(" [*] Training finished! \n")

        #     cnn.test()
        #     print(" [*] Test finished!")

        if args.phase == 'test':
            start_time = time.time()

            cnn.test(args)
            print("pystats predict elapse:{0} s".format(
                time.time() - start_time))

            print(" [*] Test finished!")


if __name__ == '__main__':
    start_time_all = time.time()

    start_time = time.time()
    protocol = "Mystique"
    rtt.activate(protocol)

    mpc_player_id = rtt.py_protocol_handler.get_party_id()
    rtt.set_backend_loglevel(3)
    print("pystats activate elapse:{0} s".format(time.time() - start_time))

    main()

    start_time = time.time()
    print(rtt.get_perf_stats(True))
    rtt.deactivate()
    print("pystats deactivate elapse:{0} s".format(time.time() - start_time))

    print("pystats total elapse:{0} s".format(time.time() - start_time_all))
