
SRC_DIR = "src/"
OUT_DIR = "out/"

import os, sys, glob
import types

cxx = ARGUMENTS.get('cxx', '/usr/bin/clang++')
cc = ARGUMENTS.get('cc', '/usr/bin/clang')
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
                                            'lzff.cpp',
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
    ['lzFFMain.cpp', 'lzFF'],
    ['lzFFdecompressMain.cpp', 'lzFFdecompress']
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

if os.path.exists('/opt/local/lib/libgtest.a') and os.path.exists('/opt/local/lib/libgtest_main.a'):
    testProg = OUT_DIR + 'runTests'
    envTEST = env.Clone(CPPPATH = ['./', '/opt/local/include'],
                        LIBPATH=['/opt/local/lib', './'],
                        LIBS=['gtest', 'gtest_main', 
                              ])
    envTESTDebug = envDebug.Clone(CPPPATH = ['./', '/opt/local/include'],
                        LIBPATH=['/opt/local/lib', './'],
                        LIBS=['gtest', 'gtest_main'],
                        CCFLAGS="-g -Wall -O0  -DDEBUG")
    objects_onlyfor = envTEST.Object(glob.glob('tests/*Tests.cpp'))
    program = envTEST.Program(testProg, objects_common + objects_onlyfor)

    envTESTDebug.Program(testProg + '.debug', debug_obj_common + makeObj(envTESTDebug, glob.glob('tests/*Tests.cpp'), '.debug.o'))

    Command("tests.passed", testProg, runUnitTest)

    test_alias = Alias(testProg, [program], program[0].abspath)
    AlwaysBuild(test_alias)
else:
    print "Google test not found in /opt/local/lib/. Testing skipped."
