#!/usr/bin/env python3

#!/usr/bin/env python3
import latticex.rosetta as rtt  # difference from tensorflow
rtt.activate("SecureNN")
mpc_player_id = rtt.py_protocol_handler.get_party_id()

print('DONE!')
