call vs_tools\cl2008_32.cmd
cl /Zi /MP4 /nologo /EHsc /I. /Feluascript_unittest_vs2008.exe /DWIN32 ^
  luascript\luascript.cpp luascript\luascript_unittest.cpp runner.cpp ^
  luascript\lua\lua-files.c gtest\gtest-all.cc
