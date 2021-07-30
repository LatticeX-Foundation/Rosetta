import tensorflow as tf
import latticex.rosetta as rst


rst.activate("SecureNN")
xa = tf.Variable(
    [
        [1.892, 2],
        [-2.3, 4.43],
        [.0091, .3]
    ]
)
xb = tf.Variable(
    [
        [2.892, 2],
        [-2.3, 4.43],
        [.0091, -0.3]
    ]
)
print("xa:\n", xa)
print("xb:\n", xb)
#

init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)

try:
    sum_res = tf.add(xa, xb)
    print("cipher add result: ", sess.run(sum_res))
    sum_plain = rst.SecureReveal(sum_res)
    print("palin result: ", sess.run(sum_plain))
    print("Pass")
except Exception as e:
    print(str(e))
    print("Fail")


rst.deactivate()
Writer = tf.summary.FileWriter("log/reveal", tf.get_default_graph())
Writer.close()