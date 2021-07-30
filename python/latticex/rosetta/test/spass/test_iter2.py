import latticex.rosetta as rtt
import tensorflow as tf
import time

local_data_file = "./dsets/simple-nohead.csv"
with_header = True
with_index = False
batch_size = 2

rtt.activate("SecureNN")

def decode(line):
    fields = tf.string_split([line], ',').values
    if with_index: # Skip index
        fields = fields[1:]

    fields = tf.string_to_number(fields, tf.float32)
    # private_input(field)
    return fields


dataset = tf.data.TextLineDataset(local_data_file)
print("dataset: ", dataset)

# if with_header: # Skip header
#   dataset = dataset.skip(1)
# dataset = dataset\
#     .map(decode, num_parallel_calls=tf.data.experimental.AUTOTUNE)\
#     .repeat(1)\
#     .prefetch(tf.data.experimental.AUTOTUNE)\
#     .batch(batch_size)

dataset = dataset\
    .map(decode)\
    .batch(batch_size)

# iterator = dataset.make_one_shot_iterator()
iterator = dataset.make_initializable_iterator()
get_batch = iterator.get_next()

batch_sum = tf.add(get_batch, 1)
batch = 0
try:
    with tf.compat.v1.Session() as sess:
        sess.run(iterator.initializer)
        start = time.time()
        try:
            while True:
                #print(sess.run(iter.get_next()))
                print("batch: ", batch, "elements: ", sess.run(batch_sum))
                batch += 1
        except tf.errors.OutOfRangeError:
            print("to next_epoch...")
        print("cost: ", time.time() - start)
        print("Pass")
except:
    print("Fail")


rtt.deactivate()

Writer = tf.compat.v1.summary.FileWriter("log/iter2", tf.compat.v1.get_default_graph())
Writer.close()