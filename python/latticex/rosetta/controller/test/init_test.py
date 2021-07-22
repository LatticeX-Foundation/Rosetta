import time
import json
import argparse

import numpy as np
np.set_printoptions(suppress=True)

import tensorflow as tf

import logging

#logging.basicConfig(level=logging.DEBUG)
logging.basicConfig(level=logging.DEBUG)

# our package
import latticex.rosetta as rtt
# log-level: default is `info`,3
rtt.set_backend_loglevel(4)

print("begin protocol testing!")

print("*" * 16 + "Test Case 1: get_supported_protocols" + "*" * 16)
print("supported protocol list: ", rtt.get_supported_protocols())

print("*" * 16 + "Test Case 2: default_run [Helix]" + "*" * 16)
#rtt.default_run()

#time.sleep(10)
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

sum_res = tf.add(xa, xb)

# init (activate with JSON string)

config_json_str = """
{
    "PARTY_ID": 0,
    "PSI": {
        "HOST": "127.0.0.1",
        "PORT": 12345,
        "RECV_PARTY": 2
    },
    "MPC": {
        "FLOAT_PRECISION": 16,
        "P0": {
            "NAME": "PartyA(P0)",
            "HOST": "127.0.0.1",
            "PORT": 11121
        },
        "P1": {
            "NAME": "PartyB(P1)",
            "HOST": "127.0.0.1",
            "PORT": 12144
        },
        "P2": {
            "NAME": "PartyC(P2)",
            "HOST": "127.0.0.1",
            "PORT": 13169
        },
        "SAVER_MODE": 7,
        "SERVER_CERT": "certs/server-nopass.cert",
        "SERVER_PRIKEY": "certs/server-prikey",
        "SERVER_PRIKEY_PASSWORD": "123456"
    }
}"""
json_config = json.loads(config_json_str)
_parser = argparse.ArgumentParser(description="LatticeX Rosetta")
_parser.add_argument('--party_id', type=int, help="Party ID",
                     required=False, default=-1, choices=[0, 1, 2])
_args, _unparsed = _parser.parse_known_args()
_party_id = _args.party_id
json_config["PARTY_ID"] = _party_id

json_config_str = json.dumps(json_config, indent=4)
rtt.activate("Helix", json_config_str)

init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("cipher add result: ", sess.run(sum_res))
sum_plain = rtt.SecureReveal(sum_res)
print("plain result: ", sess.run(sum_plain))

print("*" * 16 + "Test Case 3: activate[ with wrong name]" + "*" * 16)
rtt.activate("SecureNNX")

print("*" * 16 + "Test Case 4: activate SecureNN" + "*" * 16)
curr_config_str = rtt.get_protocol_config()
print("last config str:", curr_config_str)
new_config = json.loads(curr_config_str)
new_config["MPC"]["P0"]["PORT"] = 11133
new_config["MPC"]["P1"]["PORT"] = 11135
new_config["MPC"]["P2"]["PORT"] = 11137
new_config_str = json.dumps(new_config, indent=4)
# use new local port to activate
time.sleep(10)
rtt.deactivate()
print("DEBUG stub 3")
rtt.activate("SecureNN", new_config_str)

sess2 = tf.compat.v1.Session()
sess2.run(init)
sum_res = tf.add(xa, xb)
print("cipher add result: ", sess2.run(sum_res))

sum_plain = rtt.SecureReveal(sum_res)
print("SNN palin result: ", sess2.run(sum_plain))
curr_config_str = rtt.get_protocol_config()
print("new config str:", curr_config_str)
time.sleep(10)
print("*" * 16 + "Test Case 5: deactivate" + "*" * 16)
rtt.deactivate()
