import errno
import inspect
import os
#import pprint
import subprocess
import sys
import time


class MeasureTime:
	def __init__(self, msg, indent = 0):
		self.msg = msg
		self.indent = indent

	def __enter__(self):
		IndPrint.P(self.msg)
		IndPrint.Ind(2)
		self.start_time = time.time()
		return self

	def __exit__(self, type, value, traceback):
		dur = time.time() - self.start_time
		IndPrint.P("%.0f ms" % (dur * 1000.0))
		IndPrint.Ind(-2)

	def GetMs(self):
		return (time.time() - self.start_time) * 1000.0


def mkdirs(path):
	try:
		os.makedirs(path)
	except OSError as exc: # Python >2.5
		if exc.errno == errno.EEXIST and os.path.isdir(path):
			pass
		else:
			raise


def RmFile(fn):
	try:
		os.remove(fn)
	except:
		pass


def Indent(s0, ind):
	spaces = ""
	for i in range(ind):
		spaces += " "

	o = ""
	tokens = s0.split("\n")
	first = True
	for t in tokens:
		if len(t) == 0:
			continue
		if first == True:
			first = False
		else:
			o += "\n"
		o += spaces
		o += t
	return o


class IndPrint:
	ind = 0

	@staticmethod
	def Ind(ind):
		IndPrint.ind += ind

	@staticmethod
	def P(msg, ind = 0):
		print Indent(msg, IndPrint.ind + ind)


def Trace(msg = ""):
	prev_cf = inspect.currentframe().f_back

	# List members
	#pprint.pprint(dir(prev_cf.f_code.__class__))

	IndPrint.P("[%-10s %-15s %3d] %s" \
			% (os.path.basename(prev_cf.f_code.co_filename),
					prev_cf.f_code.co_name,
					prev_cf.f_lineno,
					msg))


def KillProcs(proc_names):
	# Uses pkill to match processes like "python ./webserver". killall doesn't
	# seem to kill them.

	for pn in proc_names:
		cmd = "pkill --signal KILL -f %s" % pn
		#print cmd
		p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		# Not waiting can be dangerous. It can kill other unwanted processed that
		# are created later on.
		stdouterr = p.communicate()[0].strip()
		if "Operation not permitted" in stdouterr:
			raise RuntimeError("Error: cmd=[%s] [%s]" % (cmd, stdouterr))

	# Wait till they are gone
	while True:
		all_killed = True
		for pn0 in proc_names:
			pn = pn0[:-1] + "[" + pn0[-1:] + "]"
			cmd = "pgrep -f %s" % pn
			#Util.Trace(cmd)
			p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
			stdouterr = p.communicate()[0]
			rc = p.returncode
			if rc == 0:
				all_killed = False
				break
			elif rc == 1:
				# No process matched
				pass
			else:
				raise RuntimeError("Error: cmd=[%s] rc=%d" % (cmd, rc))
		if all_killed:
			break
