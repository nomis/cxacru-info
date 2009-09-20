#!/usr/bin/env python
# encoding: utf-8
#
#	cxacru-info - outputs cxacru status information from sysfs
#
#	Copyright ©2009 Simon Arlott
#
#	This program is free software; you can redistribute it and/or
#	modify it under the terms of the GNU General Public License v2
#	as published by the Free Software Foundation.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#	Or, point your browser to http://www.gnu.org/copyleft/gpl.html
#
#
#	http://simon.arlott.org/sw/cxacru-info/

import sys
import re

SYS_PATH = "/sys/class/atm"
DEV_NAME = "cxacru"
DEV_PATH = SYS_PATH + "/" + DEV_NAME + "%s/device"
ATTR_PATH = DEV_PATH + "/%s"
ATM_DEVICES = "/proc/net/atm/devices"

ATM_LINE_PAT = re.compile('^ *(?P<itf>\d+) +(?P<type>\w+) +(?P<addr>\w)+(?P<aals>[^\t]+)\t\[(?P<refcnt>\d+)\]')
ATM_AALS_PAT = re.compile(' +(?P<num>\d+) +\( +(?P<tx_cnt>\d+) +(?P<tx_err>\d+) +(?P<rx_cnt>\d+) +(?P<rx_err>\d+) +(?P<rx_drop>\d+) +\)')

DS, US, XS = range(3)
DESC, TYPE, UNITS = range(3)
EXIT_SUCCESS, EXIT_FAILURE, EXIT_USAGE = range(3)

attributes = {
	# name:			desc,			type,	units
	"rate":			("Line rate",		XS,	" kbps"),
	"attenuation":		("Attenuation",		XS,	" dB"),
	"snr_margin":		("Noise margin",	XS,	" dB"),
	"transmitter_power":	("Power",		US,	" dBm/Hz"),
	"crc_errors":		("CRC errors",		XS,	""),
	"fec_errors":		("FEC errors",		XS,	""),
	"hec_errors":		("HEC errors",		XS,	""),
	"line_status":		("Line status",		DS,	None),
	"link_status":		("Link status",		DS,	None),
	"modulation":		("Modulation",		DS,	None),
	"mac_address":		("MAC address",		DS,	None)
}

def read_attr(name):
	try:
		with open(ATTR_PATH % (device["itf"], name), "r") as f:
			return f.read().rstrip("\n")
	except IOError, msg:
		print >> sys.stderr, "%s: Error reading %s%s stats" % (sys.argv[0], DEV_NAME, device["itf"])
		print >> sys.stderr, "%s: IOError: %s" % (sys.argv[0], msg)
		exit(EXIT_FAILURE)

def find_dev(num):
	try:
		with open(ATM_DEVICES) as f:
			f.readline()
			for line in f:
				line = ATM_LINE_PAT.match(line)
				if not line:
					contine
				line = line.groupdict()

				if line["type"] == DEV_NAME and (num == None or int(line["itf"]) == num):
					return line
	except IOError, msg:
		print >> sys.stderr, "%s: Error reading ATM devices" % (sys.argv[0])
		print >> sys.stderr, "%s: IOError: %s" % (sys.argv[0], msg)
		exit(EXIT_FAILURE)
	return None

def get_aal(num):
	for aal in ATM_AALS_PAT.finditer(device["aals"]):
		aal = aal.groupdict()
		if int(aal["num"]) == num:
			return aal
	return None

def print_line(desc, downstream, upstream):
	if downstream != "" and upstream == "":
		print "%-15s%s" % (desc, downstream)
	elif downstream != "":
		print "%-15s%-15s%s" % (desc, downstream, upstream)

def format_attr(value, units):
	if units != None:
		return "%10s%s" % (value, units)
	else:
		return "    %s" % (value)

def print_attr(names):
	print
	for name in names:
		attr = attributes[name]

		if attr[TYPE] == XS:
			print_line(attr[DESC],
				format_attr(read_attr("downstream_" + name), attr[UNITS]),
				format_attr(read_attr("upstream_" + name), attr[UNITS]))
		elif attr[TYPE] == DS:
			print_line(attr[DESC],
				format_attr(read_attr(name), attr[UNITS]), "")
		elif attr[TYPE] == US:
			print_line(attr[DESC],
				"", format_attr(read_attr(name), attr[UNITS]))

if "-h" in sys.argv[1:] or "--help" in sys.argv[1:] or len(sys.argv) > 2:
	print "Usage: %s [device num]" % (sys.argv[0])
	sys.exit(EXIT_USAGE)

if len(sys.argv) >= 2:
	if sys.argv[1] in ["-v", "--version"]:
		print "cxacru-info v0.9"
		sys.exit(EXIT_SUCCESS)
	else:
		try:
			num = int(sys.argv[1])
			if num < 0:
				raise ValueError
			device = find_dev(num)
		except ValueError:
			print >> sys.stderr, "%s: ATM device number \"%s\" is invalid" % (sys.argv[0], sys.argv[1])
			sys.exit(EXIT_FAILURE)
		if device == None:
			print >> sys.stderr, "%s: ATM device %s%s not found" % (sys.argv[0], DEV_NAME, num)
			sys.exit(EXIT_FAILURE)
else:
	device = find_dev(None)
	if device == None:
		print >> sys.stderr, "%s: no %s ATM devices found" % (sys.argv[0], DEV_NAME)
		sys.exit(EXIT_FAILURE)

aal5 = get_aal(5)
print_line("", format_attr("Downstream", None), format_attr("Upstream", None))
print_attr(["rate", "attenuation", "snr_margin", "transmitter_power"])
if aal5:
	print
	print_line("AAL5 Frames", "%10s" % (aal5["rx_cnt"]), "%10s" % (aal5["tx_cnt"]))
	print_line("     Errors", "%10s" % (aal5["rx_err"]), "%10s" % (aal5["tx_err"]))
	print_line("     Dropped", "%10s" % (aal5["rx_drop"]), "")
print_attr(["crc_errors", "fec_errors", "hec_errors"])
print_attr(["line_status", "link_status", "modulation"])
print_attr(["mac_address"])
sys.exit(EXIT_SUCCESS)
