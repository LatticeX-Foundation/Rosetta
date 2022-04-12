from ResNet import ResNet
import argparse
from utils import *
import os
os.environ['CUDA_VISIBLE_DEVICES'] = '2'
# NOT USE GPU
#os.environ["CUDA_DEVICE_ORDER"] = "PCI_BUS_ID"
#os.environ["CUDA_VISIBLE_DEVICES"] = "-1"

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
                        default=-1, help='train data size')
    parser.add_argument('--test_size', type=int,
                        default=-1, help='test data size')

    return check_args(parser.parse_args())


"""checking arguments"""


def check_args(args):
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

    # open session
    # with tf.Session(config=tf.ConfigProto(allow_soft_placement=True)) as sess:
    with tf.Session() as sess:
        cnn = ResNet(sess, args)

        # build graph
        cnn.build_model()

        # show network architecture
        show_all_variables()
        # variable_names = [v.name for v in tf.trainable_variables()]
        # values = sess.run(variable_names)
        # for k, v in zip(variable_names, values):
        #     print("=================================================")
        #     print("Variable: ", k)
        #     print("Shape: ", v.shape)
        #     print(v)

        if args.phase == 'train':
            # launch the graph in a session
            cnn.train()

            print(" [*] Training finished! \n")

            cnn.test()
            print(" [*] Test finished!")

        if args.phase == 'test':
            cnn.test()
            print(" [*] Test finished!")


if __name__ == '__main__':
    main()
