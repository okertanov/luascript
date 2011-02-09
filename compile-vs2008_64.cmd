call vs_tools\cl2008_64.cmd
cl /MP4 /nologo /EHsc /I. /Feluascript_unittest_vs2008.exe /DWIN64 ^
  luascript\luascript.cpp luascript\luascript_unittest.cpp runner.cpp ^
  luascript\lua\lua-files.c gtest\gtest-all.cc
