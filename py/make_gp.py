#! -*- coding: utf-8 -*-

import sys

# len_algos = 8
cmd_temp = '"%s" using 2:%s ps 1.8 pt %s title "{/=15 %s}"'
rep_list = [['REPAIR0', 'REPAIR'],
             ['OLCA0', 'OLCA'],
             ['FOLCA0', 'FOLCA'],
             ['LZDF0', 'LZDF']]
def rep(txt, rep_list):
  new_txt = txt[:]
  for rp in rep_list:
    new_txt = new_txt.replace(rp[0], rp[1])
  return new_txt

point_styles = range(1, 6) + [12] + range(8, 12) + range(13, 22)

# set terminal postscript eps color enhanced "Courier" 25
# set key box opaque
body='''
set terminal postscript eps color enhanced "Courier" 25
set datafile separator ","
set output "%s"
set key box opaque
set key spacing 0.7
set tics font "Helvetica,20"
set xlabel "{/=30 Compression Ratio}"
set ylabel "{/=20 %s}"'''
# set xlabel "{/=30 Compression Ratio (compressed size / original size)}"

def getGP(table, data_fname, out_fname, ylabel):
  n = len(table)
  num_data = sum([1 for line in table if len(line[-1]) > 0])
  len_algos = n / num_data 
  cmds = []
  assert(len(point_styles) >= len_algos)
  for i in range(len_algos):
    cmd = rep(cmd_temp % (data_fname, str(i + 3), point_styles[i], table[i*num_data][0]), rep_list)
    cmds.append(cmd)
  res = [body % (out_fname, ylabel)]
  res.append('plot ' + ','.join(cmds))
  res.append('')
  return '\n'.join(res)

def getGP2(table, fname, out_fname, ylabel, select):
  '''
  table format:[fname, size, cmp_ratio1, cmp_time1, cmp_speed1, cmp_ratio2, cmp_time2, cmp_speed2, ...]
  '''
  rep_list = [['fftrie', 'LZD'],
               ['vfpre10', 'VFPre(10)'],
               ['vfpre16', 'VFPre(16)'],
               ['vfcount10', 'VFCount(10)'],
               ['vfcount16', 'VFCount(16)'],
               ['lz', 'LZ78'],
               ['folca', 'FOLCA'],
               ['olca', 'OLCA'],
               ['repair', 'REPAIR'],
               ['bzipbest', 'bzip -9'],
               ['gzipbest', 'gzip -9']
              ]
  cmds = []
  cmd_temp = '"%s" using %s:%s ps 1.8 pt %s title "{/=15 %s}"'
  header = table[0]
  for i in range(2, len(header), 3):
    algo = header[i]
    cmd = rep(cmd_temp % (fname, str(i+1), str(i+3), point_styles[i/3], algo), rep_list)
    cmds.append(cmd)
  if select != 'select': 
    res = [body % (out_fname, ylabel)]
    res.append('plot ' + ','.join(cmds))
    res.append('')
  else:
    res = [body % (out_fname.replace('.eps', '') + '.select.eps', ylabel)]
    res.append('plot ' + ','.join(cmds[:-3]))
    res.append('')
  return '\n'.join(res)

if __name__=='__main__':
  fname = sys.argv[1]
  out_fname = sys.argv[2]
  if len(sys.argv) > 3:
    ylabel = sys.argv[3]
  else:
    ylabel = 'Run Time (sec)'
  select = sys.argv[4]
  table = [x.split(',') for x in open(fname, 'r').read().replace('\r', '\n').split('\n')]
  print getGP2(table, fname, out_fname, ylabel, select)
