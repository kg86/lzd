#! -*- coding: utf-8 -*-

import os
import sys
from comp_by_speed import getCompTableRatioTime
from make_gp import getGP
import subprocess

def getCompTableRatioTimePerFile(table, out_pre):
  def getAlgo(line):
    for i, x in enumerate(line[2:]):
      if len(x) > 0:
        return algo_header[i + 2]
    raise
  # split table for files
  table_file = {}
  algo_header = table[0]
  for line in table[1:]:
    # line = [fname, ratio, time1, time2, ..., timen]
    fname = line[0]
    algo = getAlgo(line)
    if not table_file.has_key(fname):
      table_file[fname] = []
    table_file[fname].append([algo] + line[1:])
  return table_file

if __name__=='__main__':
  ratio_fname = sys.argv[1]
  comp_fname = sys.argv[2]
  comp_dir, comp_base = os.path.split(comp_fname)
  if len(sys.argv) > 3:
    ylabel = sys.argv[3]
  else:
    ylabel = 'Run Time (sec)'
  table_ratio = [x.split(',') for x in open(ratio_fname, 'r').read().strip().split('\n')]
  table_time = [x.split(',') for x in open(comp_fname, 'r').read().strip().split('\n')]
  table_rt = getCompTableRatioTime(table_ratio, table_time, firstcol='fname')
  out_pre = 'data-' + comp_base + '-'
  for fname, table in getCompTableRatioTimePerFile(table_rt, out_pre).items():
    data_fname = os.path.join(comp_dir, 'data.%s.%s.csv' % (comp_base, fname))
    out_data = open(data_fname, 'w')
    out_data.write('\n'.join([','.join(line) for line in table]))
    out_data.close()

    out_fname = os.path.join(comp_dir, '%s.%s' % (comp_base, fname))
    gp = getGP(table, data_fname, out_fname + '.eps', ylabel)
    gp_outside = getGP(table, data_fname, out_fname + '.outcap.eps', ylabel).replace('set key spacing 0.7', 'set key spacing 0.7\nset key outside')
    gp_noncap = getGP(table, data_fname, out_fname + '.noncap.eps', ylabel).replace('set key box opaque\nset key spacing 0.7', 'unset key')

    out_gp = open(data_fname + '.gp', 'w')
    out_gp2 = open(data_fname + '2.gp', 'w')
    out_gp3 = open(data_fname + '3.gp', 'w')
    out_gp.write(gp)
    out_gp2.write(gp_outside)
    out_gp3.write(gp_noncap)
    out_gp.close()
    out_gp2.close()
    out_gp3.close()
    subprocess.check_call('gnuplot '+ data_fname + '.gp', shell=True)
    subprocess.check_call('gnuplot '+ data_fname + '2.gp', shell=True)
    subprocess.check_call('gnuplot '+ data_fname + '3.gp', shell=True)
