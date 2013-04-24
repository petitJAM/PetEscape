#server_mod.py

import subprocess
import shlex

server_process = None

def start_server( server_args ):
	global server_process
	server_process = subprocess.Popen( args=shlex.split( server_args ) )
	
def kill_server():	
	server_process.kill()
