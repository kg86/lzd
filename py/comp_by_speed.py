#! -*- coding: utf-8 -*-
import sys
import pprint

pp = pprint.PrettyPrinter(indent=2)

def filter(table_ratio, algo):
    new_table = []
    for line in zip(*table_ratio):
        if line[0] == algo:
            continue
        new_table.append(line)
    return zip(*new_table)

def getCompTableRatioTime(table_ratio, table_time, firstcol = 'fname'):
    runtime = {}
    for i, fname in enumerate(zip(*table_time)[0][1:]):
      for j, algo in enumerate(table_time[0][1:]):
        runtime[(algo, fname)] = table_time[i + 1][j + 1]

    algos = table_ratio[0][2:]
    fnames = set()
    ratio = {}
    for i in range(2, len(table_ratio[0])): # table_ratio[0]:algorithm
      algo_name = table_ratio[0][i]
      for j in range(1, len(table_ratio)):
        fname = table_ratio[j][0]
        fnames.add(fname)
        ratio[(algo_name, fname)] = table_ratio[j][i]
    lines = [['fname', 'ratio'] + algos]

    # [fname, ratio] + algorithm runtimes
    for i in range(len(algos)):
      for fname in sorted(list(fnames)):
        speeds = [str(runtime[(algos[i], fname)]) if i==j else '' for j in range(len(algos))]
        fcol = 'None'
        if firstcol == 'fname':
          fcol = fname
        elif firstcol == 'algo':
          fcol = algos[i]
        lines.append([fcol, ratio[(algos[i], fname)]] + speeds)
    return lines

if __name__ == '__main__':
    ratio_fname = sys.argv[1]
    comp_fname = sys.argv[2]
    table_ratio = [x.split(',') for x in open(ratio_fname, 'r').read().strip().split('\n')]
    table_time = [x.split(',') for x in open(comp_fname, 'r').read().strip().split('\n')]
    table_rt = getCompTableRatioTime(table_ratio, table_time, firstcol='algo')

    print '\n'.join([','.join(line) for line in table_rt[1:]])
