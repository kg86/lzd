#! -*- coding: utf-8 -*-

'''
measure compression time and speed
'''

import re
import sys
import itertools

testout='''

[start]
file=[E.coli] algo=[LZMW] codelength=[0]
failed to stat file: para
algoType is LZMW
the size of input string is 0
sh: line 1: 21222 Segmentation fault: 11  ./out/lzff -a 3 -l 0 -f para -o exp/out/2014-10-23-16-51-2/E.coli.a=3,l=0.mw
time:0.1434
[end]
[start]
file=[E.coli] algo=[LZVF_Pre] codelength=[10]
failed to stat file: para
algoType is LZVF_Pre
the size of input string is 0
256
OUTPUT TO exp/out/2014-10-23-16-51-2/E.coli.a=5,l=10.vf.pre
estimate count : 0
Time for input:6.58035e-05
Time for factorization:9.10759e-05
Time for converting to SLPs:0
Time for output:0.000167847
time:0.0049
[end]
[start]
file=[E.coli] algo=[LZVF_Pre] codelength=[12]
failed to stat file: para
algoType is LZVF_Pre
the size of input string is 0
256
OUTPUT TO exp/out/2014-10-23-16-51-2/E.coli.a=5,l=12.vf.pre
estimate count : 0
Time for input:6.10352e-05
Time for factorization:0.00022316
Time for converting to SLPs:0
Time for output:0.000149012
time:0.0044
[end]

'''

test_stat = '''[start]
cmd: ./out/lzff -a vfpre -l 10 -f ~/lz/data-canterbury/alice29.txt -o exp/comp/2015-2-8-23-12-47/alice29.txt.vfpre10 -d 1
file=[alice29.txt] algo=[vfpre] codelength=[10]
algoType is vfpre
the size of input string is 152089
15208/152089 seq.size()=3282
30418/152089 seq.size()=6335
45625/152089 seq.size()=9365
60832/152089 seq.size()=12307
76040/152089 seq.size()=15313
91251/152089 seq.size()=18167
106456/152089 seq.size()=21083
121665/152089 seq.size()=23896
136874/152089 seq.size()=26818
152080/152089 seq.size()=29854
num_fnodes=1022 num_nodes=1087
ff.seq.size()=29860
len_factors.size()=29858 len_lfactors.size()=29320 alive_time.size()=29858
len_factors : avg=5.11176 median=4 max=50 min=1
len_lfactors : avg=2.60965 median=2 max=44 min=1
len_rfactors : avg=2.57759 median=2 max=28 min=1
len_deleted_factors : avg=5.13514 median=4 max=50 min=1
alive_time : avg=1003.58 median=765 max=29858 min=1
1022
OUTPUT TO exp/comp/2015-2-8-23-12-47/alice29.txt.vfpre10
fileSize : 152089 seqSize : 29860 codeSize : 10
estimate count : 9331
i=29850 [564, 564]
i=29851 [222, 222]
i=29852 [747, 222]
i=29853 [564, 565]
i=29854 [603, 5]
i=29855 [0, 78]
i=29856 [645, 883]
i=29857 [690, 3]
i=29858 [0, 26]
i=29859 [500, 0]
Time for input:0.00285792
Time for factorization:0.028815
Time for converting to SLPs:0
Time for output:0.00119615
DEBUG MODE ON
29859/29860 end
len_factors.size()=29858 len_lfactors.size()=29320 alive_time.size()=29858
len_factors : avg=5.11176 median=4 max=50 min=1
len_lfactors : avg=2.60965 median=2 max=44 min=1
len_rfactors : avg=2.57759 median=2 max=28 min=1
len_deleted_factors : avg=5.13514 median=4 max=50 min=1
alive_time : avg=1003.58 median=765 max=29858 min=1
the decompressed string equals to the input string.
Time for Total:0.061131
[end][start]
cmd: ./out/lzff -a vfpre -l 16 -f ~/lz/data-canterbury/alice29.txt -o exp/comp/2015-2-8-23-12-47/alice29.txt.vfpre16 -d 1
file=[alice29.txt] algo=[vfpre] codelength=[16]
algoType is vfpre
[end][start]
cmd: ./out/lzff -a vfpre -l 10 -f ~/lz/data-pizzachili/dna.50MB -o exp/comp/2015-1-31-20-33-39/dna.50MB.vfpre10 -d 1
file=[dna.50MB] algo=[vfpre] codelength=[10]
algoType is vfpre
the size of input string is 52428800
5242881/52428800 seq.size()=688288
10485761/52428800 seq.size()=1378216
15728641/52428800 seq.size()=2064143
20971525/52428800 seq.size()=2752591
26214402/52428800 seq.size()=3440023
31457282/52428800 seq.size()=4127870
36700162/52428800 seq.size()=4816924
41943041/52428800 seq.size()=5504925
47185922/52428800 seq.size()=6190116
num_fnodes=1021 num_nodes=1320
ff.seq.size()=6878325
len_factors.size()=6878324 len_lfactors.size()=6877850 alive_time.size()=6878324
len_factors : avg=7.62239 median=7 max=960 min=1
len_lfactors : avg=3.81325 median=3 max=480 min=1
len_rfactors : avg=3.8096 median=3 max=480 min=1
len_deleted_factors : avg=7.6225 median=7 max=960 min=1
alive_time : avg=-227.917 median=833 max=6878324 min=1
1021
OUTPUT TO exp/comp/2015-1-31-20-33-39/dna.50MB.vfpre10
fileSize : 52428800 seqSize : 6878325 codeSize : 10
estimate count : 2149476
i=6878315 [25, 886]
i=6878316 [5, 254]
i=6878317 [27, 375]
i=6878318 [833, 406]
i=6878319 [5, 538]
i=6878320 [187, 187]
i=6878321 [9, 73]
i=6878322 [187, 280]
i=6878323 [944, 375]
i=6878324 [9, 2]
Time for input:0.0479269
Time for factorization:6.52898
Time for converting to SLPs:0
Time for output:0.174229
DEBUG MODE ON
len_factors.size()=6878324 len_lfactors.size()=6877850 alive_time.size()=6878324
len_factors : avg=7.62239 median=7 max=960 min=1
len_lfactors : avg=3.81325 median=3 max=480 min=1
len_rfactors : avg=3.8096 median=3 max=480 min=1
len_deleted_factors : avg=7.6225 median=7 max=960 min=1
alive_time : avg=-227.917 median=833 max=6878324 min=1
the decompressed string equals to the input string.
Time for Total:12.220493
[end][start]
'''

test_txt ='''[end][start]
cmd: ./out/lzff -a vfpre -l 10 -f ~/lz/data-pizzachili/dna.50MB -o exp/comp/2015-1-31-20-33-39/dna.50MB.vfpre10 -d 1
file=[dna.50MB] algo=[vfpre] codelength=[10]
algoType is vfpre
the size of input string is 52428800
5242881/52428800 seq.size()=688288
10485761/52428800 seq.size()=1378216
15728641/52428800 seq.size()=2064143
20971525/52428800 seq.size()=2752591
26214402/52428800 seq.size()=3440023
31457282/52428800 seq.size()=4127870
36700162/52428800 seq.size()=4816924
41943041/52428800 seq.size()=5504925
47185922/52428800 seq.size()=6190116
num_fnodes=1021 num_nodes=1320
ff.seq.size()=6878325
len_factors.size()=6878324 len_lfactors.size()=6877850 alive_time.size()=6878324
len_factors : avg=7.62239 median=7 max=960 min=1
len_lfactors : avg=3.81325 median=3 max=480 min=1
len_rfactors : avg=3.8096 median=3 max=480 min=1
len_deleted_factors : avg=7.6225 median=7 max=960 min=1
alive_time : avg=-227.917 median=833 max=6878324 min=1
1021
OUTPUT TO exp/comp/2015-1-31-20-33-39/dna.50MB.vfpre10
fileSize : 52428800 seqSize : 6878325 codeSize : 10
estimate count : 2149476
i=6878315 [25, 886]
i=6878316 [5, 254]
i=6878317 [27, 375]
i=6878318 [833, 406]
i=6878319 [5, 538]
i=6878320 [187, 187]
i=6878321 [9, 73]
i=6878322 [187, 280]
i=6878323 [944, 375]
i=6878324 [9, 2]
Time for input:0.0479269
Time for factorization:6.52898
Time for converting to SLPs:0
Time for output:0.174229
DEBUG MODE ON
len_factors.size()=6878324 len_lfactors.size()=6877850 alive_time.size()=6878324
len_factors : avg=7.62239 median=7 max=960 min=1
len_lfactors : avg=3.81325 median=3 max=480 min=1
len_rfactors : avg=3.8096 median=3 max=480 min=1
len_deleted_factors : avg=7.6225 median=7 max=960 min=1
alive_time : avg=-227.917 median=833 max=6878324 min=1
the decompressed string equals to the input string.
Time for Total:12.220493
[end][start]
'''

# test_case = '''Time to read file %s in/out is correct'''
# test_case = r'''\./bg/.*?Total:\s[\d.e-]+'''
test_case = r'''\[start\].*?\[end\]'''

exp_each_time = r'Time for (\S+?):\s*([\d.e\-+]+)'
exp_total_time = r'Total from (\S+): ([\d.e\-+]+)'
exp_file_prog_algo = r'file=\[(\S+)\] algo=\[(.*?)\] codelength=\[(\S+)\]'
# exp_file_prog_algo = r'file=\[(\S+)\] prog=\[(\S*?)\] algo=\[(\S+)\]'
exp_num_factor = r'# of lz factors: ([\d.e-]+)'

test_casep=re.compile(test_case, re.S)
p_each_time = re.compile(exp_each_time)
p_total_time = re.compile(exp_total_time)
p_file_prog_algo = re.compile(exp_file_prog_algo)
p_num_factor = re.compile(exp_num_factor)

exp_stat=r'''len_factors : avg=([\d.-]+) median=([\d.-]+) max=([\d.-]+) min=([\d.-]+)
len_lfactors : avg=([\d.-]+) median=([\d.-]+) max=([\d.-]+) min=([\d.-]+)
len_rfactors : avg=([\d.-]+) median=([\d.-]+) max=([\d.-]+) min=([\d.-]+)
len_deleted_factors : avg=([\d.-]+) median=([\d.-]+) max=([\d.-]+) min=([\d.-]+)
alive_time : avg=0 median=([\d.-]+) max=([\d.-]+) min=([\d.-]+)'''
exp_seqsize=r'''seq.size\(\)=(.*)'''
exp_stat=r'''len_factors : avg=([\d.-]+) median=([\d.-]+) max=([\d.-]+) min=([\d.-]+)
len_lfactors : avg=([\d.-]+) median=([\d.-]+) max=([\d.-]+) min=([\d.-]+)
len_rfactors : avg=([\d.-]+) median=([\d.-]+) max=([\d.-]+) min=([\d.-]+)
len_deleted_factors : avg=([\d.-]+) median=([\d.-]+) max=([\d.-]+) min=([\d.-]+)
alive_time : avg=([\d.-]+) median=([\d.-]+) max=([\d.-]+) min=([\d.-]+)'''
# '''

p_seqsize = re.compile(exp_seqsize)
p_stat = re.compile(exp_stat)

stat_header = 'fname,algo,codesize,seqsize,factors_len_avg, factors_len_med,factors_len_max, factors_len_min,lfactors_len_avg,lfactors_len_med,lfactors_len_max,lfactors_len_min,rfactors_len_avg,rfactors_len_med,rfactors_len_max,rfactors_len_min,deleted_factors_len_avg,deleted_factors_len_med,deleted_factors_len_max,deleted_factors_len_min, alive_time_avg, alive_time_med, alive_time_max, alive_time_min'
def stat(text):
  print stat_header
  tcases=test_casep.findall(text)
  print len(tcases)
  for tcase in tcases:
    res = p_file_prog_algo.search(tcase)
    fname = res.group(1)
    algo = res.group(2)
    codelength = res.group(3)
    seqsize = p_seqsize.search(tcase).groups()[0]
    print seqsize
    res_stat = p_stat.search(tcase)
    stat_size = 4 * 4
    if res_stat:
      res_stat = res_stat.groups()
    else:
      res_stat = [0] * stat_size
    print ','.join(map(str, [fname, algo, codelength, seqsize] + list(res_stat)))
  

def all_times(text):
  '''
  return all time list
  '''
  # text = testout
  tcases=test_casep.findall(text)
  times = {}
  fnames = set()
  algos = set()
  alclfname = set()
  time_types = set()
  for tcase in tcases:
    res = p_file_prog_algo.search(tcase)
    fname = res.group(1)
    algo = res.group(2)
    alclfname.add((algo, fname))
    algo = algo
    algos.add(algo)
    fnames.add(fname)
    m = p_each_time.findall(tcase)

    if not times.has_key((fname, algo)):
      times[(fname,algo)] = []
    cur_times = {}
    for i in m:
      time_types.add(i[0])
      cur_times[i[0]] = i[1]
    times[(fname, algo)].append(cur_times)

  fnames = list(fnames)
  algos = list(algos)
  time_types = list(time_types)
  fnames.sort()
  algos.sort()
  time_types.sort()
  return fnames, algos, time_types, times, alclfname

def avg(fnames, algos, time_types, times):
  ret_times = {}
  for (fname, algo) in itertools.product(fnames, algos):
    if not times.has_key((fname, algo)):
      continue
    avg_times = [0 for i in range(len(time_types))]
    for time in times[(fname,algo)]:
      for i, time_type in enumerate(time_types):
        if time.has_key(time_type):
          avg_times[i] += float(time[time_type])
        else:
          avg_times[i] = 0
    for i in range(len(avg_times)):
      avg_times[i] /= len(times[(fname, algo)])
    ret_times[(fname,algo)] = avg_times
  return fnames, algos, ret_times

def cmp_times(fnames, algos, avg_times, time_type_idx):
  print 'fname,' + ','.join(algos)
  inf = '999'
  for fname in fnames:
    line = []
    for algo in algos:
      if not avg_times.has_key((fname,algo)):
        line.append(inf)
      else:
        line.append(avg_times[(fname,algo)][time_type_idx])
    print ','.join([fname, ','.join(map(str,line))])

def test():
  print all_times(testout)

def table2fnamealgo(table):
  '''
  return file list and algorithm list from table
  table consists a header and following rows
  [fname1, algo1, ...] 
  [fname2, algo2, ...]
  '''
  algos = []
  fnames = []
  for fname, algo in zip(*zip(*table[1:])[:2]):
    if fname not in fnames:
      fnames.append(fname)
    if algo not in algos:
      algos.append(algo)
  return fnames, algos

def getCompTable(text, runtime_format='%f'):
  table_avg = getAvgTable(text)
  fnames, algos = table2fnamealgo(table_avg)
  total_idx = 2
  times = {}
  for line in table_avg[1:]:
    fname, algo = line[:2]
    times[(fname, algo)] = runtime_format % float(line[total_idx])
  lines = [map(str, ['fname'] + algos)]
  for fname in fnames:
    line = [fname]
    for algo in algos:
      if not times.has_key((fname, algo)):
        line.append(0)
      else:
        line.append(times[(fname, algo)])
    if len(line) > 0 and sum(map(float, line[1:])) > 0:
      lines.append(line)
    else:
      print 'HOGEHOGE'
  return lines

def usage():
  print sys.argv[0] + ' option in_fname'
  print ' ' * len(sys.argv[0]) + ' 0: run test cases'
  print ' ' * len(sys.argv[0]) + ' all: output all times'
  print ' ' * len(sys.argv[0]) + ' avg: output avg times'

def getTimeTable(text):
  '''
  extract runtimes from log file, and return them as table.
  [fname, algo, time_type1, time_type2, ...]
  '''
  fnames, algos,time_types, times, alclfname = all_times(text)
  lines = []
  lines.append(['fname','algo'] + time_types)
  for (fname, algo) in itertools.product(fnames, algos):
    if not times.has_key((fname,algo)): continue
    for time in times[(fname, algo)]:
      line = [fname, algo]
      for time_type in time_types:
        if time.has_key(time_type):
          line.append(time[time_type])
        else:
          line.append(0)
      lines.append(map(str, line))
  return lines

def getAvgTable(text):
  '''
  [fname, algo, time_type1, time_type2, ...]
  '''
  table_all = getTimeTable(text)
  fnames, algos = table2fnamealgo(table_all)
  avg_times = {}
  num = {}
  for line in table_all[1:]:
    fname = line[0]
    algo = line[1]
    for j in range(2, len(line)):
      if not avg_times.has_key((fname, algo)):
        avg_times[(fname, algo)] = [0 for k in range(2, len(line))]
        num[(fname,algo)] = 0
      num[(fname,algo)] += 1
      for k in range(2, len(line)):
        avg_times[(fname, algo)][k-2] += float(line[k])
  for key in avg_times.keys():
    avg_times[key] = [x / num[key] for x in avg_times[key]]
  header = table_all[0]
  lines = [header] # header
  for fname in fnames:
    line = [fname]
    for algo in algos:
      lines.append([fname, algo] + map(str, avg_times[(fname, algo)]))
  return lines

def tableOut(table):
  print '\n'.join([','.join(line) for line in table])

if __name__=='__main__':
  if sys.argv[1] == '0':
    test();
    sys.exit(1)
  text = open(sys.argv[2], 'r').read()
#   print sys.argv[2], text

  if sys.argv[1] == 'all':
    '''output all time'''
    tableOut(getTimeTable(text))
  elif sys.argv[1] == 'avg':
    '''output average time'''
    tableOut(getAvgTable(text))
  elif sys.argv[1] == 'comp':
    tableOut(getCompTable(text, '%f'))
  elif sys.argv[1] == 'speed':
    table_comp = getCompTable(text)
    from comp_ratio import getSize
    flist = open(sys.argv[3], 'r').read().strip().split('\n')
    sizes = getSize(flist)
    table_time = getCompTable(text, '%f')
    new_table = [table_comp[0]]
    for line in table_comp[1:]:
      fname = line[0]
      new_table.append([fname] + [str(sizes[fname] / float(x)) for x in line[1:]])
    tableOut(new_table)

  elif sys.argv[1] == 'stat':
    stat(text)

