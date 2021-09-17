import time
from ops import *
from utils import *


class ResNet(object):
    def __init__(self, sess, args):
        self.model_name = 'ResNet'
        self.sess = sess
        self.dataset_name = args.dataset

        if self.dataset_name == 'cifar10':
            self.train_x, self.train_y, self.test_x, self.test_y = load_cifar10(
                args)
            self.img_size = 32
            self.c_dim = 3
            self.label_dim = 10

        if self.dataset_name == 'cifar100':
            self.train_x, self.train_y, self.test_x, self.test_y = load_cifar100()
            self.img_size = 32
            self.c_dim = 3
            self.label_dim = 100

        if self.dataset_name == 'mnist':
            self.train_x, self.train_y, self.test_x, self.test_y = load_mnist(
                args)
            self.img_size = 28
            self.c_dim = 1
            self.label_dim = 10

        if self.dataset_name == 'fashion-mnist':
            self.train_x, self.train_y, self.test_x, self.test_y = load_fashion()
            self.img_size = 28
            self.c_dim = 1
            self.label_dim = 10

        if self.dataset_name == 'tiny':
            self.train_x, self.train_y, self.test_x, self.test_y = load_tiny()
            self.img_size = 64
            self.c_dim = 3
            self.label_dim = 200

        self.checkpoint_dir = args.checkpoint_dir
        self.log_dir = args.log_dir

        self.res_n = args.res_n

        self.epoch = args.epoch
        self.batch_size = args.batch_size
        self.iteration = len(self.train_x) // self.batch_size

        self.init_lr = args.lr

    ##################################################################################
    # Generator
    ##################################################################################

    def network(self, x, is_training=True, reuse=False):
        with tf.variable_scope("network", reuse=reuse):

            if self.res_n < 50:
                residual_block = resblock
            else:
                residual_block = bottle_resblock2

            residual_list = get_residual_layer(self.res_n)

            ########################################################################################################

            ch = 64  # paper is 64
            x = conv(x, channels=ch, kernel=3, stride=1, scope='conv')  # 32
            # x = conv(x, channels=ch, kernel=7, stride=2, scope='conv')
            x = batch_norm(x, is_training, scope='batch_norm0')
            x = relu(x)
            x = tf.nn.max_pool(
                x, ksize=(1, 3, 3, 1), strides=(1, 2, 2, 1), padding='VALID')

            ########################################################################################################

            x = residual_block(
                x, channels=ch, is_training=is_training, downsample=True, scope='resblock0_0')

            for i in range(1, residual_list[0]):
                x = residual_block(x, channels=ch, is_training=is_training,
                                   downsample=False, scope='resblock0_' + str(i))

            ########################################################################################################

            x = residual_block(
                x, channels=ch*2, is_training=is_training, downsample=True, scope='resblock1_0', stridess=2)

            for i in range(1, residual_list[1]):
                x = residual_block(x, channels=ch*2, is_training=is_training,
                                   downsample=False, scope='resblock1_' + str(i))

            ########################################################################################################

            x = residual_block(
                x, channels=ch*4, is_training=is_training, downsample=True, scope='resblock2_0', stridess=2)

            for i in range(1, residual_list[2]):
                x = residual_block(x, channels=ch*4, is_training=is_training,
                                   downsample=False, scope='resblock2_' + str(i))

            ########################################################################################################

            x = residual_block(
                x, channels=ch*8, is_training=is_training, downsample=True, scope='resblock_3_0', stridess=2)

            for i in range(1, residual_list[3]):
                x = residual_block(x, channels=ch*8, is_training=is_training,
                                   downsample=False, scope='resblock_3_' + str(i))

            ########################################################################################################

            x = batch_norm(x, is_training, scope='batch_norm1')
            x = relu(x)

            x = global_avg_pooling(x)
            #x = avg_pooling(x)
            x = fully_conneted(x, units=self.label_dim, scope='logit')

            return x

    ##################################################################################
    # Model
    ##################################################################################

    def build_model(self):
        """ Graph Input """
        self.train_inptus = tf.placeholder(tf.float32, [
                                           self.batch_size, self.img_size, self.img_size, self.c_dim], name='train_inputs')
        self.train_labels = tf.placeholder(
            tf.float32, [self.batch_size, self.label_dim], name='train_labels')

        self.test_inptus = tf.placeholder(tf.float32, [len(
            self.test_x), self.img_size, self.img_size, self.c_dim], name='test_inputs')
        self.test_labels = tf.placeholder(
            tf.float32, [len(self.test_y), self.label_dim], name='test_labels')

        self.lr = tf.placeholder(tf.float32, name='learning_rate')

        """ Model """
        self.train_logits = self.network(self.train_inptus)
        self.test_logits = self.network(
            self.test_inptus, is_training=False, reuse=tf.AUTO_REUSE)

        self.train_loss, self.train_accuracy = classification_loss(
            logit=self.train_logits, label=self.train_labels)
        self.test_loss, self.test_accuracy = classification_loss(
            logit=self.test_logits, label=self.test_labels)

        reg_loss = tf.losses.get_regularization_loss()
        self.train_loss += reg_loss
        self.test_loss += reg_loss

        """ Training """
        self.optim = tf.train.MomentumOptimizer(
            self.lr, momentum=0.9).minimize(self.train_loss)
        #self.optim = tf.train.AdamOptimizer(self.lr).minimize(self.train_loss)

        """" Summary """
        self.summary_train_loss = tf.summary.scalar(
            "train_loss", self.train_loss)
        self.summary_train_accuracy = tf.summary.scalar(
            "train_accuracy", self.train_accuracy)

        self.summary_test_loss = tf.summary.scalar("test_loss", self.test_loss)
        self.summary_test_accuracy = tf.summary.scalar(
            "test_accuracy", self.test_accuracy)

        self.train_summary = tf.summary.merge(
            [self.summary_train_loss, self.summary_train_accuracy])
        self.test_summary = tf.summary.merge(
            [self.summary_test_loss, self.summary_test_accuracy])

    ##################################################################################
    # Train
    ##################################################################################

    def train(self):
        # initialize all variables
        tf.global_variables_initializer().run()

        # saver to save model
        self.saver = tf.train.Saver(name='v2', max_to_keep=3)

        # summary writer
        self.writer = tf.summary.FileWriter(
            self.log_dir + '/' + self.model_dir, self.sess.graph)

        # restore check-point if it exits
        could_load, checkpoint_counter = self.load(self.checkpoint_dir)
        if could_load:
            epoch_lr = self.init_lr
            start_epoch = (int)(checkpoint_counter / self.iteration)
            start_batch_id = checkpoint_counter - start_epoch * self.iteration
            counter = checkpoint_counter

            if start_epoch >= int(self.epoch * 0.75):
                epoch_lr = epoch_lr * 0.01
            elif start_epoch >= int(self.epoch * 0.5) and start_epoch < int(self.epoch * 0.75):
                epoch_lr = epoch_lr * 0.1
            print(" [*] succeed in loading model checkpoint!")
        else:
            epoch_lr = self.init_lr
            start_epoch = 0
            start_batch_id = 0
            counter = 1
            print(" [!] Fail to load model checkpoint. If this is your first time to train, you can ignore this.")

        # loop for epoch
        start_time = time.time()
        test_loss, test_accuracy = 0.0, 0.0
        for epoch in range(start_epoch, self.epoch):
            if epoch == int(self.epoch * 0.5) or epoch == int(self.epoch * 0.75):
                epoch_lr = epoch_lr * 0.1

            # get batch data
            for idx in range(start_batch_id, self.iteration):
                batch_x = self.train_x[idx *
                                       self.batch_size:(idx+1)*self.batch_size]
                batch_y = self.train_y[idx *
                                       self.batch_size:(idx+1)*self.batch_size]

                batch_x = data_augmentation(
                    batch_x, self.img_size, self.dataset_name)

                train_feed_dict = {
                    self.train_inptus: batch_x,
                    self.train_labels: batch_y,
                    self.lr: epoch_lr
                }

                test_feed_dict = {
                    self.test_inptus: self.test_x,
                    self.test_labels: self.test_y
                }

                # update network
                _, train_loss, train_accuracy = self.sess.run(
                    [self.optim, self.train_loss, self.train_accuracy], feed_dict=train_feed_dict)
                # _, summary_str, train_loss, train_accuracy = self.sess.run(
                #     [self.optim, self.train_summary, self.train_loss, self.train_accuracy], feed_dict=train_feed_dict)
                # self.writer.add_summary(summary_str, counter)

                # test
                if counter % 10 == 0:
                    test_loss, test_accuracy = self.sess.run(
                        [self.test_loss, self.test_accuracy], feed_dict=test_feed_dict)
                # summary_str, test_loss, test_accuracy = self.sess.run(
                #     [self.test_summary, self.test_loss, self.test_accuracy], feed_dict=test_feed_dict)
                # self.writer.add_summary(summary_str, counter)

                # display training status
                counter += 1
                print("Epoch: [%2d] [%5d/%5d] time: %4.4f, train_accuracy: %.4f, test_accuracy: %.4f, learning_rate : %.5f, train_loss : %.4f, test_loss : %.4f"
                      % (epoch, idx, self.iteration, time.time() - start_time, train_accuracy, test_accuracy, epoch_lr, train_loss, test_loss), flush=True)

                if counter % 10 == 0:
                    self.save(self.checkpoint_dir, counter)

            # After an epoch, start_batch_id is set to zero
            # non-zero value is only for the first epoch after loading pre-trained model
            start_batch_id = 0

            # save model
            self.save(self.checkpoint_dir, counter)

        # save model for final step
        self.save(self.checkpoint_dir, counter)

    @property
    def model_dir(self):
        # return "{}{}_{}_{}_{}".format(self.model_name, self.res_n, self.dataset_name, self.batch_size, self.init_lr)
        return "{}{}_{}".format(self.model_name, self.res_n, self.dataset_name)

    def save(self, checkpoint_dir, step):
        checkpoint_dir = os.path.join(checkpoint_dir, self.model_dir)

        if not os.path.exists(checkpoint_dir):
            os.makedirs(checkpoint_dir)

        self.saver.save(self.sess, os.path.join(
            checkpoint_dir, self.model_name+'.model'), global_step=step)

    def load(self, checkpoint_dir):
        print(" [*] Reading checkpoints...")
        checkpoint_dir = os.path.join(checkpoint_dir, self.model_dir)

        ckpt = tf.train.get_checkpoint_state(checkpoint_dir)
        if ckpt and ckpt.model_checkpoint_path:
            ckpt_name = os.path.basename(ckpt.model_checkpoint_path)
            self.saver.restore(self.sess, os.path.join(
                checkpoint_dir, ckpt_name))
            counter = int(ckpt_name.split('-')[-1])
            print(" [*] Success to read {}".format(ckpt_name))
            return True, counter
        else:
            print(" [*] Failed to find a checkpoint")
            return False, 0

    def test(self):
        tf.global_variables_initializer().run()

        self.saver = tf.train.Saver(name='v2', max_to_keep=3)
        could_load, checkpoint_counter = self.load(self.checkpoint_dir)

        if could_load:
            print(" [*] Load SUCCESS")
        else:
            print(" [!] Load failed...")

        test_feed_dict = {
            self.test_inptus: self.test_x,
            self.test_labels: self.test_y
        }

        test_accuracy = self.sess.run(
            self.test_accuracy, feed_dict=test_feed_dict)
        print("test_accuracy: {}".format(test_accuracy))
