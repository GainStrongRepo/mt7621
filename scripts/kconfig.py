#!/usr/bin/env python
# 
# Copyright (C) 2016 Hua Shao <nossiac@163.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

import re
import os
import sys

PREFIX = "CONFIG_"

def load_config(file):
	configs = {}
	with open(file, "r") as fp:
		for line in fp:
			m = re.match(r"CONFIG_(.+?)=(.+)", line)
			if m:
				configs[m.group(1)] = m.group(2)
				continue
			m = re.match(r"# CONFIG_(.+?) is not set", line)
			if m:
				configs[m.group(1)] = None #"n"
	return configs

def config_and(cfg1, cfg2):
	""" common subset of 2 sets """
	common = {}
	for k in cfg1.keys():
		if k in cfg2 and cfg1[k] == cfg2[k]:
			common[k] = cfg1[k]
	return common

def config_merge(cfg1, cfg2, overwrite=0):
	"""
	take cfg1 as base, merge cfg2 into cfg1
		overwrite mode 0: y=m=n (default)
		overwrite mode 1: y=m>n
		overwrite mode 2: y>m>n
		overwrite mode 3: n>y=m
	"""
	for k in cfg2.keys():
		if k in cfg1:
			if overwrite == 0 and cfg2[k]:
				cfg1[k] = cfg2[k]
			elif overwrite == 1:
				if cfg2[k] or not cfg1[k]:
					cfg1[k] = cfg2[k]
			elif overwrite == 2:
				if not cfg1[k]:
					cfg1[k] = cfg2[k]
				elif cfg2[k] == "y":
					cfg1[k] = cfg2[k]
				elif cfg1[k] == "y":
					continue
				else: # not a module at all
					cfg1[k] = cfg2[k]
			elif overwrite == 3:
				if not cfg2[k] or cfg1[k]:
					cfg1[k] = cfg2[k]
		else:
			cfg1[k] = cfg2[k]

	return cfg1

def config_diff(cfg1, cfg2, overwrite = None):
	""" xor result of 2 sets """
	diff = {}
	for k in cfg2.keys():
		try:
			if k not in cfg1:
				if overwrite and not cfg2[k]:
					continue
				diff[k] = cfg2[k]
			elif cfg1[k] != cfg2[k]:
				diff[k] = cfg2[k]
		except Exception as e:
			sys.exit(-1)

	return diff


def config_sub(cfg1, cfg2):
	""" cfg1 subs common part of cfg2 """
	for k in cfg2.keys():
		if k in cfg1:
			del cfg[k]
	return cfg1


def dump_config(cfg = {}):
	for k in sorted(cfg.keys()):
		if not cfg[k] or cfg[k] == "n":
			print("# {0} is not set".format(PREFIX+k))
		else:
			print("{0}={1}".format(PREFIX+k,cfg[k]))



import getopt

def usage():
	print("Basic usage:\n"
		"	kconfig.py <-m|--merge>  <1.config> <2.config> # append 2 into 1\n"
		"	kconfig.py <-c|--common> <1.config> <2.config> # return common part\n"
		"	kconfig.py <-d|--diff>   <1.config> <2.config> # return different part\n"
		"	kconfig.py <-f|--filter> <1.config> <2.config> # remove 2 from 1\n"
		"Advance flags:\n"
		"	<-o|--overwrite> [mode] specify overwrite mode\n"
		"		mode 0: y=m=n (default)\n"
		"		mode 1: y=m>n\n"
		"		mode 2: y>m>n\n"
		"		mode 3: n>y=m\n"
		)
	sys.exit(-1)

def main():
	try:
		opts, args = getopt.getopt(sys.argv[1:], "mcdfo:", 
			["merge", "common", "diff", "filter", "overwrite"])
	except getopt.GetoptError as err:
		# print help information and exit:
		print(err)  # will print something like "option -a not recognized"
		usage()
		sys.exit(2)

	overwrite = 0
	action=""
	for k, v in opts:
		if k in ("-o", "--overwrite"):
			overwrite = int(v) or 0
		elif k in ("-m", "--merge"):
			action = "merge" if not action else usage()
		elif k in ("-c", "--common"):
			action = "common" if not action else usage()
		elif k in ("-d", "--diff"):
			action = "diff" if not action else usage()
		elif k in ("-f", "--filter"):
			action = "filter" if not action else usage()
		else:
			usage()


	cfgs = {}
	for f in args:
		if not os.path.exists(f):
			print(f+" does not exist!")
			sys.exit(-1)
		tmp = load_config(f)
		if action == "common":
			if not cfgs:
				cfgs = config_and(cfgs, tmp)
			else:
				cfgs = config_and(cfgs, tmp)
		elif action == "filter":
			cfgs = config_sub(cfgs, tmp)
		elif action == "merge":
			cfgs = config_merge(cfgs, tmp, overwrite)
		elif action == "diff":
			cfgs = config_diff(cfgs, tmp, overwrite)
		else:
			assert False, "shouldn't be here!"

	dump_config(cfgs)

if __name__ == "__main__":
	main()
