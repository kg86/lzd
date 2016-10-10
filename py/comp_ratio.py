#! -*- coding: utf-8 -*-

'''
input: directory which stores compressed files.
output: csv which stores compression ratios of input.
'''

import os
import sys

KB = 1024
MB = KB * 1024

def byteRound(_fsize):
  fsize = float(_fsize)
  if (fsize / MB) > 1:
    return '%.2f MB' % (fsize / MB)
  if (fsize / KB) > 1:
    return '%.2f KB' % (fsize / KB)
  return str(_fsize) + ' B'

def getSize(flist):
  '''
  return uncompressed size of given files as a dictionary
  '''
  res = {}
  for f in flist:
    statinfo = os.stat(f)
    res[os.path.basename(f)] = statinfo.st_size
  return res

def main(exp_dir, flist, ratio = True):
  '''
  return compression ratio if ratio is True
  return comressed size if ratio is False
  '''
  orig_size = getSize(flist)
  out_sizes = {}
  al = set()
  # compute compressed size
  for f in [f for f in  os.listdir(exp_dir) if f.find('.al') > 0 and f.startswith('comp_')]:
    orig_fname, alop = f[len('comp_'):].split('.al=')
    out_sizes[(orig_fname, alop)] = os.stat(exp_dir + '/' + f).st_size
    al.add(alop)

  # output file name, algorithm, and those data sizes
  print ','.join(map(str, ['fname', 'orig'] + [x for x in sorted(list(al))]))
  for fname in map(os.path.basename, flist):
    line = [fname, byteRound(orig_size[fname])]
    allZero = True
    for alop in sorted(list(al)):
      if not (fname, alop) in out_sizes:
        line.append(0)
      else:
        allZero = False
        if ratio:
          r = 100.0 * out_sizes[(fname, alop)] / orig_size[fname]
          line.append('%.2f' % r)
        else:
          line.append(out_sizes[(fname, alop)])
    if not allZero:
      print ','.join(map(str, line))
  return

if __name__=='__main__':
  ratio = False
  exp_dir = sys.argv[1]
  flist_name = sys.argv[2]
  flist = open(flist_name, 'r').read().strip().split('\n')
  if len(sys.argv) > 3 and sys.argv[3] == 'True':
    ratio = True
  main(exp_dir, flist, ratio)

