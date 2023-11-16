import sys
import os
import getopt
import subprocess
import numpy as np
import yahoo

config={}
MAX_TICKER_ROWS=1764
def get_config_file(path) -> str:
	print(path+"/../../../../../config.ini")
	f=''
	if os.access("/etc/stockminer/config.ini",os.R_OK) is True:
		f="/etc/stockminer/config.ini"
	elif os.access("config.ini", os.R_OK) is True:
		f="config"
	elif os.access(path+"/../../../../../config.ini",os.R_OK) is True:
		f=path+"/../../../../../config.ini"
	else:
		print("config file missing")
	with open(f,'r') as fp:
		return fp.read()

def get_options(argv, opstr_short, opstr_long) -> dict:
	argdict = {};
	opsh    = [*opstr_short.replace(':', '')]
	count   = 0
	try:
		opts, args = getopt.getopt(argv, opstr_short, opstr_long)
	except getopt.GetoptError as err:
		print(err)
		sys.exit(-1)
	for op, arg in opts:
		if op.startswith('--'):
			# long options
			op = op.replace('-','') + '='
			argdict[op.replace('=','')] = arg;
		else:
			# short options
			op = op.replace('-','')
			for short in opsh:
				if op == short:
					argdict[opstr_long[count].replace('-','').replace('=','')] = arg;
					break
				count = count + 1
	return argdict

def load_tickers(path) -> list:
	ticker_list       = []
	nr_python_threads = 4
	with open(path,'r') as fp:
		tickers=fp.read()
	if "ticker_delim" in config:
		delim = config['ticker_delim']
	else:
		delim = 0
	if "ticker_field" in config:
		field = int(config['ticker_field'])
	else:
		field = 0
	lines = tickers.splitlines()
	for line in lines:
		if delim:
			ticker = line.split(delim)[field]
		else:
			ticker = line;
		ticker_list.append(ticker)
	if "nr_python_threads" in config:
		nr_python_threads  = int(config['nr_python_threads'])
	config['ticker_list']  = ticker_list
	config['ticker_lists'] = np.array_split(ticker_list, nr_python_threads)

def init() -> dict:
	path = os.path.dirname(os.path.realpath(__file__))
	conf = get_config_file(path)
	for line in conf.splitlines():
		if (len(line) == 0):
			continue
		if line[0]=='#' or line[0]=='[':
			continue
		args=line.split("=")
		config[args[0]] = args[1]
	load_tickers(config['ticker_path'])
	config['__file__'] = os.path.dirname(os.path.realpath(__file__))
	return config

def stockminer_colgen():
	procs  = []
	pyfile = config['__file__'] + "/colgen.py"
	for x in range(int(config['nr_python_threads'])):
		procs.append(subprocess.Popen(["python3", pyfile, str(x)], close_fds=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE))

if __name__ == "__main__":
	if "__file__" not in locals():
		__file__ = "/stockmkiner/src/python/stockminer/src/"
	init()
	if (sys.argv[1] == "yahoo"):
		yahoo.stockminer_yahoo(sys.argv[2:],config)
	elif (sys.argv[1] == "colgen"):
		stockminer_colgen()
