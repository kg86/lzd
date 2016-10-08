#include <fstream>
#include <iostream>
#include <cstdlib>
#include <gtest/gtest.h>
#include <unistd.h>
#include <sys/wait.h>

#include "../src/lzd.hpp"
#include "../src/common.hpp"
#include "../src/string_tree.hpp"
#include "../src/string_tree_pre.hpp"
#include "../src/string_tree_occ.hpp"
#include "../src/string_tree_clean.hpp"
#include "../src/slp2enc.hpp"
#include "../src/lz78.hpp"

class CompDecompTest : public ::testing::Test{
  protected:
  std::string file_dir;
  std::string dir_bill;
  std::string out_dir;
  std::vector<std::string> files;
  std::vector<std::string> strs;
  std::vector<std::string> out_fnames;
  virtual void SetUp(){
    file_dir = "../data/canterbury";
    // out_dir = "../data_out";
    out_dir = "../data/out";
    int num_bill = 16;
    std::string tmp_files [] = {
      "alice29.txt",
      "asyoulik.txt",
      "cp.html",
      "fields.c",
      "grammar.lsp",
      "kennedy.xls",
      "lcet10.txt",
      "plrabn12.txt",
      "ptt5",
      "xargs.1"
    };
    unsigned int num_files= 10;
    files = std::vector<std::string>(tmp_files, tmp_files + num_files);
    unsigned int n = (unsigned int) files.size();

    strs = std::vector<std::string>(n, "");
    out_fnames = std::vector<std::string>(n, "");
    for (unsigned int i = 0; i < n; i++){
      std::string fname;
      if (i < num_files) {
        fname = file_dir + "/" + files[i];
      }else if (i < num_files + num_bill){
        fname = dir_bill + "/" + files[i];
      }
      out_fnames[i] = out_dir + "/" + files[i];
      UTIL::stringFromFile(fname, strs[i]);
      files[i] = fname;
    }
  }
};

TEST_F(CompDecompTest, LZVF_PRE){
  // UTIL::DEBUG_LEVEL = 1;
  unsigned int n = files.size();
  unsigned int codeSizes[] = {8, 12};
  for (unsigned int j = 0; j < 2; j++){
    unsigned int codeSize = codeSizes[j];
    for (unsigned int i = 0; i < n; i++){
      std::stringstream ss;
      ss << out_fnames[i] << ".vf.pre." << codeSize;
      std::string out_fname = ss.str();
      std::string decomp_str;

      STreePre::STreePreCompress(files[i], out_fname, codeSize);

      // UTIL::DEBUG_LEVEL = 2;
      // std::cout << "******* [decomp] ******* str.size()=";
      std::ifstream out_infs(out_fname.c_str(), std::ios::in|std::ios::binary);
      std::ostringstream out_str;
      STreePre::STreePreDecompress(out_infs, out_str);
      out_str.flush();
      const std::string str = out_str.str();
      EXPECT_EQ(strs[i].size(), str.size());
      EXPECT_TRUE(strs[i] == str);

    }
  }
}

TEST_F(CompDecompTest, LZVF_COUNT){
  unsigned int i = 0;
  unsigned int n = files.size();
  unsigned int codeSizes[] = {8, 12};
  for (unsigned int j = 0; j < 2; j++){
    unsigned int codeSize = codeSizes[j];
    for (i = 0; i < n; i++){
      std::stringstream ss;
      ss << out_fnames[i] << ".vf.count." << codeSize;
      std::string out_fname = ss.str();
      std::string decomp_str;
      // std::ostringstream ss;
      // std::cout << "vfocc file[" << in_fname << "] out_fname=["
      //           << out_fname << "] codeSize" << codeSize << std::endl;

      STreeOcc::STreeOccCompress(files[i], out_fname, codeSize);

      // std::cout << "******* [decomp] ******* str.size()=";
      std::ifstream out_infs(out_fname.c_str(), std::ios::in|std::ios::binary);

      std::ostringstream out_str;
      STreeOcc::STreeOccDecompress(out_infs, out_str);

      const std::string str = out_str.str();
      EXPECT_EQ(strs[i].size(), str.size());
      EXPECT_TRUE(strs[i] == str);
    }
  }
}

TEST_F(CompDecompTest, LZVF_CLEAN){
  unsigned int i = 0;
  unsigned int n = files.size();
  unsigned int codeSizes[] = {8, 12};
  for (unsigned int j = 0; j < 2; j++){
    unsigned int codeSize = codeSizes[j];
    for (i = 0; i < n; i++){
      std::stringstream ss;
      ss << out_fnames[i] << ".vf.clean." << codeSize;
      std::string out_fname = ss.str();
      std::string decomp_str;
      // std::cout << "vfocc file[" << in_fname << "] out_fname=["
      //           << out_fname << "] codeSize" << codeSize << std::endl;

      STreeClean::STreeCleanCompress(files[i], out_fname, codeSize);

      std::ifstream out_infs(out_fname.c_str(), std::ios::in|std::ios::binary);

      std::ostringstream out_str;
      STreeClean::STreeCleanDecompress(out_infs, out_str);

      const std::string str = out_str.str();
      // std::cout << str.size() << std::endl;;
      EXPECT_EQ(strs[i].size(), str.size());
      EXPECT_TRUE(strs[i] == str);
    }
  }
}

TEST_F(CompDecompTest, LZVF_PRE_NO_STREAM){
  // UTIL::DEBUG_LEVEL = 1;
  unsigned int n = files.size();
  unsigned int codeSizes[] = {8, 12};
  for (unsigned int j = 0; j < 2; j++){
    unsigned int codeSize = codeSizes[j];
    for (unsigned int i = 0; i < n; i++){
      std::stringstream ss;
      ss << out_fnames[i] << ".vf.pre_ns." << codeSize;
      std::string out_fname = ss.str();
      std::string decomp_str;

      // std::cout << out_fname << std::endl;
      // comp
      const std::string & in_str = strs[i];
      LZFF::LZD ffcomp;
      LZFF::ffpre_compress(in_str, codeSize, ffcomp);
      ffcomp.out(out_fname);

      // decomp
      LZFF::LZD ffdecomp(out_fname);
      LZFF::ffpre_decompress(ffdecomp, decomp_str);
      EXPECT_EQ(strs[i].size(), decomp_str.size());
      EXPECT_TRUE(strs[i] == decomp_str);
    }
  }
}

TEST_F(CompDecompTest, LZVF_COUNT_NO_STREAM){
  // UTIL::DEBUG_LEVEL = 1;
  unsigned int n = files.size();
  unsigned int codeSizes[] = {8, 12};
  for (unsigned int j = 0; j < 2; j++){
    unsigned int codeSize = codeSizes[j];
    for (unsigned int i = 0; i < n; i++){
      std::stringstream ss;
      ss << out_fnames[i] << ".vf.count_ns." << codeSize;
      std::string out_fname = ss.str();
      std::string decomp_str;

      // std::cout << out_fname << std::endl;
      // comp
      const std::string & in_str = strs[i];
      LZFF::LZD ffcomp;
      LZFF::ffocc_compress(in_str, codeSize, ffcomp);
      ffcomp.out(out_fname);

      // decomp
      LZFF::LZD ffdecomp(out_fname);
      LZFF::ffocc_decompress(ffdecomp, decomp_str);
      EXPECT_EQ(strs[i].size(), decomp_str.size());
      EXPECT_TRUE(strs[i] == decomp_str);
    }
  }
}

TEST_F(CompDecompTest, LZD){
  // UTIL::DEBUG_LEVEL = 1;
  unsigned int n = files.size();
  unsigned int codeSizes[] = {8, 12};
  for (unsigned int j = 0; j < 2; j++){
    unsigned int codeSize = codeSizes[j];
    for (unsigned int i = 0; i < n; i++){
      std::stringstream ss;
      ss << out_fnames[i] << ".double_ns." << codeSize;
      std::string out_fname = ss.str();
      std::string decomp_fname = out_fname + ".decomp";
      std::string decomp_str;

      // std::cout << "comp file=" << out_fname << std::endl
      //           << "decomp file=" << decomp_fname << std::endl;
      // comp

      LZFF::LZD ff;
      std::string tmp_str = strs[i];
      LZFF::ff_compress(tmp_str, ff);
      // std::cout << "convert to SLPs" << std::endl;
      std::vector<std::pair<unsigned int, unsigned int> > vars;
      std::string outSLP;
      LZFF::seq2vars(ff.seq, vars);
      slp2enc(vars, (unsigned int) strs[i].size(), out_fname);

      // decomp
      std::string cmd_decomp = "../out/lzdDecompress -a lzd -f " + out_fname+ " -o " + decomp_fname;
      UTIL::runCmd(cmd_decomp);
      UTIL::stringFromFile(decomp_fname, decomp_str);

      EXPECT_EQ(strs[i].size(), decomp_str.size());
      EXPECT_TRUE(strs[i] == decomp_str);
    }
  }
}

TEST_F(CompDecompTest, LZMW){
  unsigned int n = files.size();
  unsigned int codeSizes[] = {8, 12};
  for (unsigned int j = 0; j < 2; j++){
    unsigned int codeSize = codeSizes[j];
    for (unsigned int i = 0; i < n; i++){
      std::stringstream ss;
      ss << out_fnames[i] << ".mw_ns." << codeSize;
      std::string out_fname = ss.str();
      std::string decomp_fname = out_fname + ".decomp";
      std::string decomp_str;

      LZFF::mw_compress(files[i], out_fname);

      std::string cmd_decomp = "../out/lzdDecompress -a lzmw -f " + out_fname+ " -o " + decomp_fname;
      UTIL::runCmd(cmd_decomp);
      UTIL::stringFromFile(decomp_fname, decomp_str);

      EXPECT_EQ(strs[i].size(), decomp_str.size());
      EXPECT_TRUE(strs[i] == decomp_str);
    }
  }
}


TEST_F(CompDecompTest, LZ78){
  // UTIL::DEBUG_LEVEL = 1;
  unsigned int n = files.size();
  unsigned int codeSizes[] = {8, 12};
  for (unsigned int j = 0; j < 2; j++){
    unsigned int codeSize = codeSizes[j];
    for (unsigned int i = 0; i < n; i++){
      std::stringstream ss;
      ss << out_fnames[i] << ".lz78." << codeSize;
      std::string out_fname = ss.str();
      std::string decomp_fname = out_fname + ".decomp";
      std::string decomp_str;

      // std::cout << "comp file=" << out_fname << std::endl
      //           << "decomp file=" << decomp_fname << std::endl;
      // comp
      std::vector<std::pair<unsigned int, unsigned int> > vars;
      LZ78::LZ78 lz78;
      LZ78::compress(strs[i], lz78);
      LZ78::seq2vars(lz78.seq, vars);
      slp2enc(vars, (unsigned int) strs[i].size(), out_fname);

      std::string cmd_decomp = "../out/lzdDecompress -a lz78 -f " + out_fname+ " -o " + decomp_fname;
      UTIL::runCmd(cmd_decomp);
      UTIL::stringFromFile(decomp_fname, decomp_str);

      EXPECT_EQ(strs[i].size(), decomp_str.size());
      EXPECT_TRUE(strs[i] == decomp_str);
    }
  }
}

