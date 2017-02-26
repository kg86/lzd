
SRC_DIR = "src/"
OUT_DIR = "out/"

import os, sys, glob
import types

include = '/usr/include'
lib = '/usr/lib'
if sys.platform == 'darwin':
    include = '/opt/local/include'
    lib = '/opt/local/lib'

cxx = ARGUMENTS.get('cxx', '/usr/bin/clang++')
cc = ARGUMENTS.get('cc', '/usr/bin/clang')
lib = ARGUMENTS.get('lib', lib) # /opt/local/lib for mac os
include = ARGUMENTS.get('include', include) # /opt/local/include for mac os
env = Environment(ENV = {'PATH' : os.environ['PATH']},
                  CC = cc, CXX = cxx,
                  CXXFLAGS="-Wall -Ofast -DNDEBUG",
                  CCFLAGS="-Wall -Ofast -DNDEBUG",
                  LINKFLAGS="-Ofast")
envDebug = Environment(ENV = {'PATH' : os.environ['PATH']},
    CC = cc, CXX = cxx,
    CCFLAGS="-g -Wall -O0 -DDEBUG",
    CXXFLAGS="-g -Wall -O0 -DDEBUG"
    )


def makeprog(env, onlyfor_sources, progname, srcs, exts):
    objects_onlyfor = makeObj(env, onlyfor_sources, exts[0])
    env.Program(OUT_DIR + progname + exts[1], objects_onlyfor + srcs)

def makeObj(env, srcs, suf):
    if type(srcs) != types.ListType:
        srcs = [srcs]
    ret = []
    for src in srcs:
        name, ext = os.path.splitext(os.path.basename(src))
        ret.append(env.Object(OUT_DIR + name+suf, src))
    return ret

sources_common = [SRC_DIR + src for src in ['common.cpp',
                                            'LRUList.cpp',
                                            'string_tree.cpp',
                                            'slp2enc.cpp',
                                            'lz78.cpp',
                                            'lzd.cpp',
                                            'string_tree_ns.cpp',
                                            ]]
sources_lcacomp = [SRC_DIR +'lcacomp/' + src for src in ['bits.c',
                                            'enc2txt.c',
                                            'cfg2enc.c',
                                            'txt2cfg_online']]
sources_common = sources_common + sources_lcacomp
objects_common = makeObj(env, sources_common, '.o')
debug_obj_common = makeObj(envDebug, sources_common, '.debug.o')

progs = [
    ['lzdMain.cpp', 'lzd'],
    ['lzdDecompressMain.cpp', 'lzdDecompress']
    ]

for fin, fout in progs:
    makeprog(env, SRC_DIR + fin, fout, objects_common, ['.o',''])
    makeprog(envDebug, SRC_DIR + fin, fout, debug_obj_common, ['.debug.o','.debug'])

# tests: uses google-test
####################################################
#
def runUnitTest(env,target,source):
    import subprocess
    app = str(source[0].abspath)
    if not subprocess.call(app,cwd="./tests"):
        open(str(target[0]),'w').write("PASSED\n")

gtest = os.path.join(lib, 'libgtest.a')
gtest_main = os.path.join(lib, 'libgtest_main.a')
print gtest, gtest_main
if os.path.exists(gtest) and os.path.exists(gtest_main):
    testProg = OUT_DIR + 'runTests'
    envTEST = env.Clone(CPPPATH = ['./', include],
                        LIBPATH=[lib, './'],
                        LIBS=['pthread']
                        )
    envTESTDebug = envDebug.Clone(CPPPATH = ['./', include],
                        LIBPATH=[lib, './'],
                        LIBS=['pthread'],
                        CCFLAGS="-g -Wall -O0  -DDEBUG")
    objects_onlyfor = envTEST.Object(glob.glob('tests/*Tests.cpp'))
    program = envTEST.Program(testProg, objects_common + objects_onlyfor + [gtest, gtest_main])

    objects_onlyfor_debug = makeObj(envTESTDebug, glob.glob('tests/*Tests.cpp'), '.debug.o')
    envTESTDebug.Program(testProg + '.debug', debug_obj_common + objects_onlyfor_debug + [gtest, gtest_main])

    Command("tests.passed", testProg, runUnitTest)

    test_alias = Alias(testProg, [program], program[0].abspath)
    AlwaysBuild(test_alias)
else:
    print "Google test not found in /opt/local/lib/. Testing skipped."
