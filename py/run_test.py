#! -*- coding: utf-8 -*-

import subprocess
import time
import os
import sys
from optparse import OptionParser

class cmdClass:
  def __init__(self,
               _comp_cmd, _cmd_op,
               _decomp_cmd, _decomp_op, _suf, _dir, _algo, _cl = 0):
    self.comp_cmd = _comp_cmd + ' ' + _cmd_op
    self.decomp_cmd = _decomp_cmd + ' ' + _decomp_op
    self.comp_cmd_debug = _comp_cmd + '.debug ' + _cmd_op
    self.decomp_cmd_debug = _decomp_cmd + '.debug ' + _decomp_op
    self.suf = '.al=' + _suf
    self.dir = _dir
    self.algo = _algo
    self.cl = _cl

  def outname(self, infile):
    return infile + self.suf


cmd_bzips = [cmdClass('bzip2', '--' + op + ' %s -c > %s',
                     'bzip2', '-d %s -c > %s',
                     'bzip' + op,
                     'bzip' + op,
                     'bzip' + op)
  for op in ['fast', 'best']]

cmd_gzips = [cmdClass('gzip', '--' + op + ' %s -c > %s',
                     'gzip', '-d %s -c > %s',
                     'gzip' + op,
                     'gzip' + op,
                     'gzip' + op)
  for op in ['fast', 'best']]

def zeroEmpty(x):
  if x == 0 or x == '0':
    return ''
  else:
    return x

cmd_fftrie = cmdClass('./out/lzd', ' -a lzd -f %s -o %s',
                    './out/lzdDecompress', ' -a lzd -f %s -o %s',
                    'fftrie',
                    'fftrie',
                    'fftrie')
cmd_mwtrie = cmdClass('./out/lzd', ' -a lzmw -f %s -o %s',
                    './out/lzdDecompress', ' -a lzmw -f %s -o %s',
                    'mwtrie',
                    'mwtrie',
                    'mwtrie')
cmd_vfs = [cmdClass('./out/lzd', ' -a ' + algo +' -l ' + cl + ' -f %s -o %s',
                    './out/lzdDecompress', ' -a ' + algo + ' -f %s -o %s',
                    algo + '.cl=' + zeroEmpty(cl),
                    algo+cl,
                    algo,
                    'cl='+cl)
    for (algo, cl) in [('vfpre', '10'), ('vfpre', '12'), ('vfpre', '16'),
                       ('vfcount', '10'), ('vfcount', '12'), ('vfcount', '16'),
                       ('vfclean', '10'), ('vfclean', '12'), ('vfclean', '16')
                       ]]

cmd_lz78 = cmdClass('./out/lzd', ' -a lz78 -f %s -o %s',
                    './out/lzdDecompress', ' -a lz78 -f %s -o %s',
                    'lz78',
                    'lz78',
                    'lz78')

test_case_begin = r'[start]'
test_case_end = r'[end]'
def test_file_algo_option(fname , algo_name, option):
  return 'file=[%s] algo=[%s] codelength=[%s]' % (fname, algo_name, option)

func_list = [cmd_fftrie, cmd_mwtrie, cmd_lz78] + cmd_bzips + cmd_gzips  + cmd_vfs

def comp_test(date, test_set, do_purge, num_repeat, verbose=False):
  '''
  run compression
  '''
  comp_dir = 'exp/%s' % date
  os.mkdir('exp')
  os.mkdir(comp_dir)
  fprefix = comp_dir + '/comp_'
  out_fname = 'exp/%s/run_test.comp.txt' % date
  sys.stderr.write('res: %s\n' % out_fname)
  res_file = open(out_fname, 'w')
  sys.stderr.write('output to %s\n' % comp_dir)
  for avg in range(num_repeat):
    for in_file in test_set:
      basename = os.path.basename(in_file)
      sys.stderr.write('%d/%d %s\n' % (avg, num_repeat, in_file))
      for func in func_list:
        line = []
        out_file = fprefix + func.outname(basename)
        cmd = func.comp_cmd % (in_file, out_file)
        if verbose:
          print cmd
        if do_purge:
          line.append('run purge ' + subprocess.check_output('purge'))
        line.append(test_case_begin)
        line.append('cmd: ' + cmd)
        line.append(test_file_algo_option(basename, func.suf.replace('.al=',''), func.cl))
        start_time = time.time()
        print cmd
        line.append(subprocess.check_output(cmd, shell=True))
        line.append('Time for Total:%f' % (time.time() - start_time))
        line.append(test_case_end)
        res_file.write('\n'.join(line))
  return

def decomp_test(date, test_set, do_purge, num_repeat, verbose=False):
  '''
  run decompression
  '''
  decomp_dir = 'exp/' + date
  fprefix = decomp_dir + '/decomp_'
  out_fname = decomp_dir + '/run_test.decomp.txt'
  sys.stderr.write('res: %s\n' % out_fname)
  res_file = open(out_fname, 'w')
  for avg in range(num_repeat):
    for basename in test_set:
      basename = os.path.basename(basename)
      sys.stderr.write('%d/%d %s\n' % (avg, num_repeat, basename))
      for func in func_list:
        line = []
        in_file = os.path.join(decomp_dir, 'comp_' + func.outname(basename)) # compressed file name
        out_file = fprefix + func.outname(basename)

        if not os.path.exists(in_file):
          sys.stderr.write('%s does not exist\n' % in_file)
          continue
        if func.algo == 'ADS':
          cmd = func.decomp_cmd % (in_file, out_file, in_file + '.dic')
        else:
          cmd = func.decomp_cmd % (in_file, out_file)
        if verbose:
          print cmd
        if do_purge:
          line.append('run purge ' + subprocess.check_output('purge'))
        line.append(test_case_begin)
        line.append('cmd: '+ cmd)
        line.append(test_file_algo_option(basename, func.suf.replace('.al=',''), func.cl))
        start_time = time.time()
        line.append(subprocess.check_output(cmd, shell=True))
        line.append('Time for Total:%f' % (time.time() - start_time))
        line.append(test_case_end)
        res_file.write('\n'.join(line))
  return res_file

def stat_test(date, test_set, verbose = False):
  func_list = [cmd_lz78, cmd_fftrie] + cmd_vfs
  for func in func_list:
    print 'hoge'
  pass

usage = "usage: %prog [options]"

if __name__=='__main__':
  parser = OptionParser()
  parser.set_usage(usage)
  parser.add_option("-f", "--fname", dest="fname", metavar="file list")
  parser.add_option("-d", "--date", dest="date", metavar="date")
  parser.add_option("-p", "--purge", dest="do_purge", action='store_true')
  parser.add_option("-v", "--verbose", dest="verbose", action='store_true')
  parser.add_option("-a", "--act", dest="act", metavar='comp/decomp/compdecomp')
  parser.add_option("-c", "--comp_dir", dest="comp_dir", metavar='DIR')
  parser.add_option("-r", "--num_repeat", dest="num_repeat", type='int',
                  help="number of repeat to run tests", metavar='num')
  (opt, args) = parser.parse_args()
  test_set = []

  # option for data set
  print 'do_purge:', opt.do_purge
  test_set = open(opt.fname, 'r').read().strip().split('\n')

  if opt.act == 'comp':
    comp_test(opt.date, test_set, opt.do_purge, opt.num_repeat, opt.verbose)
  elif opt.act == 'decomp':
    decomp_test(opt.date, test_set, opt.do_purge, opt.num_repeat, opt.verbose)
  elif opt.act == 'compdecomp':
    comp_test(opt.date, test_set, opt.do_purge, opt.num_repeat, opt.verbose)
    decomp_test(opt.date, test_set, opt.do_purge, opt.num_repeat, opt.verbose)
  elif opt.act == 'stat':
    stat_test(opt.date, test_set, opt.verbose)
