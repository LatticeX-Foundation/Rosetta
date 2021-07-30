import tensorflow as tf
import latticex.rosetta as rst



x = tf.Variable([1,2])
y = tf.Variable([1,2])
z = x + y

try:
    print(x[0])
    print(z[0])
    print("Pass")
except:
    print("Fail")
    

writer = tf.summary.FileWriter("log/getitem", tf.get_default_graph())
writer.close()