// Copyright (c) 2009-2011 by Alexander Demin and Alexei Bezborodov

#include "gtest/gtest.h"
#include "luascript/luascript.h"

#include <fstream>
#include <string>

TEST(LuaScript, PointerCast)
{
  LuaScript luaScript;
  EXPECT_EQ(&luaScript, IntToPointer(PointerToInt(&luaScript)));
  EXPECT_EQ(-1, PointerToInt(IntToPointer(-1)));
}

TEST(LuaScript, UrlParsingExample)
{
  try
  {
    LuaScript luaScript;
    std::string url = "URL:host=live.system,user=test";
    luaScript.SetVariable<LuaScript::String_LuaArg>("url", url);
    std::string filter = "if url:find('user=test') then "
      "  url = url:gsub('(host=)[^.]*', '%1test') "
      "end";
    luaScript.Execute(filter);
    EXPECT_EQ(std::string("URL:host=test.system,user=test"),
            luaScript.GetVariable<LuaScript::String_LuaArg>("url").GetValue());
  }
  catch(const LuaScript::LuaException& e)
  {
    FAIL() << "error: " << e.Error() << ", line " << e.Line();
  }
}

TEST(LuaScript, SyntaxErrorUnknownFunction) 
{
  try
  {
    LuaScript luaScript;
    luaScript.Execute("exists = fs.file_exists('test')");
  }
  catch(const LuaScript::LuaException& e)
  {
    EXPECT_EQ(std::string("attempt to index global 'fs' (a nil value)"),
                                                            e.Error());
    EXPECT_EQ(1, e.Line());
  }
}

TEST(LuaScript, SetGetVariable)
{
  try
  {
    LuaScript luaScript;

    luaScript.SetVariable<LuaScript::String_LuaArg>("a", "test");
    luaScript.Execute("b = a;");
    EXPECT_EQ(std::string("test"),
           luaScript.GetVariable<LuaScript::String_LuaArg>("b").GetValue());

    luaScript.SetVariable<LuaScript::Int_LuaArg>("a", 100);
    luaScript.Execute("b = a;");
    EXPECT_EQ(100,
            luaScript.GetVariable<LuaScript::Int_LuaArg>("b").GetValue());

    luaScript.SetVariable<LuaScript::Bool_LuaArg>("a", true);
    luaScript.Execute("b = a;");
    EXPECT_EQ(true,
            luaScript.GetVariable<LuaScript::Bool_LuaArg>("b").GetValue());

    luaScript.SetVariable<LuaScript::Bool_LuaArg>("a", false);
    luaScript.Execute("b = a;");
    EXPECT_EQ(false,
            luaScript.GetVariable<LuaScript::Bool_LuaArg>("b").GetValue());
  }
  catch(const LuaScript::LuaException& e)
  {
    FAIL() << "error: " << e.Error() << ", line " << e.Line();
  }
}

TEST(LuaScript, RequirePackageBase64)
{
  try
  {
    LuaScript luaScript;
    luaScript.Execute("package.path = package.path .. ';./lib/?.lua';");
    luaScript.Execute("require('base64'); a = base64.encode('test');");
    EXPECT_EQ(std::string("dGVzdA=="),
        luaScript.GetVariable<LuaScript::String_LuaArg>("a").GetValue());
  }
  catch(const LuaScript::LuaException& e)
  {
    FAIL() << "error: " << e.Error() << ", line " << e.Line();
  }
}

TEST(LuaScript, TestMath1)
{
  try
  {
    LuaScript luaScript;
    luaScript.Execute("a1 = 8; a2 =3; b = (a1 % (2 ^ a2)) > 0;");
    bool ret = luaScript.GetVariable<LuaScript::Bool_LuaArg>("b").GetValue();
    EXPECT_EQ(false, ret) <<
        "/n check luai_nummod(): may be floor() change to round()";
  }
  catch(const LuaScript::LuaException& e)
  {
    FAIL() << "error: " << e.Error() << ", line " << e.Line();
  }
}

TEST(LuaScript, TestMath2)
{
  try
  {
    LuaScript luaScript;
    luaScript.Execute("a1 = 1; a2 =1; b = (a1 % (2 ^ a2)) == 1;");
    bool ret = luaScript.GetVariable<LuaScript::Bool_LuaArg>("b").GetValue();
    EXPECT_EQ(true, ret) <<
        "/n check luai_nummod(): may be floor() change to round()";
  }
  catch(const LuaScript::LuaException& e)
  {
    FAIL() << "error: " << e.Error() << ", line " << e.Line();
  }
}

class LuaFileExistFunction
{
 public:
  LuaFileExistFunction(LuaScript* a_Parent) { }

  static const LuaScript::LuaArgArray* GetInputArgs()
  {
    LuaScript::LuaArgArray* args = new LuaScript::LuaArgArray();
    args->Add(new LuaScript::String_LuaArg());
    return args;
  }

  static const LuaScript::LuaArgArray* GetOutputArgs() 
  {
    LuaScript::LuaArgArray* args = new LuaScript::LuaArgArray();
    args->Add(new LuaScript::Bool_LuaArg());
    return args;
  }

  static const std::string NameSpace() { return "fs"; }
  static const std::string Name() { return "file_exists"; }

  void Calc(const LuaScript::LuaArgArray& in, LuaScript::LuaArgArray& out)
  {
    std::string filename = 
        dynamic_cast<LuaScript::String_LuaArg&>(*in[0]).GetValue();
    std::ifstream is(filename.c_str());
    dynamic_cast<LuaScript::Bool_LuaArg&>(*out[0]).SetValue(is.good());
  }
};

TEST(LuaScript, FileExistsFunction)
{
  try
  {
    LuaScript luaScript;

    luaScript.RegisterFunction< LuaFileExistFunction >();

    luaScript.SetVariable<LuaScript::String_LuaArg>("fname",
            "./luascript/luascript_unittest.cpp");
    luaScript.Execute("exists = fs.file_exists(fname);");
    EXPECT_EQ(true,
       luaScript.GetVariable<LuaScript::Bool_LuaArg>("exists").GetValue());

    luaScript.SetVariable<LuaScript::String_LuaArg>("fname",
                                    std::string("its_nonexisting_h"));
    luaScript.Execute("exists = fs.file_exists(fname);");
    EXPECT_EQ(false,
       luaScript.GetVariable<LuaScript::Bool_LuaArg>("exists").GetValue());
  }
  catch(const LuaScript::LuaException& e)
  {
    FAIL() << "error: " << e.Error() << ", line " << e.Line();
  }
}

class LuaReturnSomeFunction 
{
 public:
  LuaReturnSomeFunction(LuaScript* a_Parent) { }

  static const LuaScript::LuaArgArray* GetInputArgs()
  {
    LuaScript::LuaArgArray* args = new LuaScript::LuaArgArray();
    return args;
  }

  static const LuaScript::LuaArgArray* GetOutputArgs()
  {
    LuaScript::LuaArgArray* args = new LuaScript::LuaArgArray();
    args->Add(new LuaScript::Bool_LuaArg());
    args->Add(new LuaScript::Int_LuaArg());
    args->Add(new LuaScript::String_LuaArg());
    return args;
  }

  static const std::string NameSpace() { return "test"; }
  static const std::string Name() { return "return_list"; }

  void Calc(const LuaScript::LuaArgArray& in, LuaScript::LuaArgArray& out)
  {
    dynamic_cast<LuaScript::Bool_LuaArg&>(*out[0]).SetValue(true);
    dynamic_cast<LuaScript::Int_LuaArg&>(*out[1]).SetValue(100);
    dynamic_cast<LuaScript::String_LuaArg&>(*out[2]).SetValue("test");
  }
};

TEST(LuaScript, ReturnSomeFromFunction)
{
  try
  {
    LuaScript luaScript;
    luaScript.RegisterFunction< LuaReturnSomeFunction >();
    luaScript.Execute("b, i, s = test.return_list();");
    EXPECT_EQ(true,
           luaScript.GetVariable<LuaScript::Bool_LuaArg>("b").GetValue());
    EXPECT_EQ(100,
           luaScript.GetVariable<LuaScript::Int_LuaArg>("i").GetValue());
    EXPECT_EQ("test",
           luaScript.GetVariable<LuaScript::String_LuaArg>("s").GetValue());
  }
  catch(const LuaScript::LuaException& e)
  {
    FAIL() << "error: " << e.Error() << ", line " << e.Line();
  }
}

class LuaReturnEqualParamFunction
{
 public:
  LuaReturnEqualParamFunction(LuaScript* a_Parent) { }

  static const LuaScript::LuaArgArray* GetInputArgs()
  {
    LuaScript::LuaArgArray* args = new LuaScript::LuaArgArray();
    args->Add(new LuaScript::Bool_LuaArg());
    args->Add(new LuaScript::Int_LuaArg());
    args->Add(new LuaScript::String_LuaArg());
    args->Add(new LuaScript::Bool_LuaArg());
    args->Add(new LuaScript::VectorInt_LuaArg());
    args->Add(new LuaScript::VectorString_LuaArg());
    return args;
  }

  static const LuaScript::LuaArgArray* GetOutputArgs()
  {
    LuaScript::LuaArgArray* args = new LuaScript::LuaArgArray();
    args->Add(new LuaScript::Bool_LuaArg());
    args->Add(new LuaScript::Int_LuaArg());
    args->Add(new LuaScript::String_LuaArg());
    args->Add(new LuaScript::Bool_LuaArg());
    args->Add(new LuaScript::VectorInt_LuaArg());
    args->Add(new LuaScript::VectorString_LuaArg());
    return args;
  }

  static const std::string NameSpace() { return "test"; }
  static const std::string Name() { return "return_list"; }

  void Calc(const LuaScript::LuaArgArray& in, LuaScript::LuaArgArray& out) 
  {
    dynamic_cast<LuaScript::Bool_LuaArg&>(*out[0]).SetValue(
            dynamic_cast<LuaScript::Bool_LuaArg&>(*in[0]).GetValue());
    dynamic_cast<LuaScript::Int_LuaArg&>(*out[1]).SetValue(
            dynamic_cast<LuaScript::Int_LuaArg&>(*in[1]).GetValue());
    dynamic_cast<LuaScript::String_LuaArg&>(*out[2]).SetValue(
            dynamic_cast<LuaScript::String_LuaArg&>(*in[2]).GetValue());
    dynamic_cast<LuaScript::Bool_LuaArg&>(*out[3]).SetValue(
            dynamic_cast<LuaScript::Bool_LuaArg&>(*in[3]).GetValue());
    dynamic_cast<LuaScript::VectorInt_LuaArg&>(*out[4]).SetValue(
            dynamic_cast<LuaScript::VectorInt_LuaArg&>(*in[4]).GetValue());
    dynamic_cast<LuaScript::VectorString_LuaArg&>(*out[5]).SetValue(
            dynamic_cast<LuaScript::VectorString_LuaArg&>(*in[5]).GetValue());
  }
};

TEST(LuaScript, MultiReturnEqualFromFunction) 
{
  try 
  {
    LuaScript luaScript;
    luaScript.RegisterFunction<LuaReturnEqualParamFunction>();
    std::string strScript = 
      "local testTable = { 0, 1, 2, 3 };"
      "local testStringTable = { 'test0', "
      " 'test1', "
      " 'test2', "
      " 'qwertyuiop[]asdfghjkl;zxcvbnm,./`1234567890-=' "
      " };"
      "b, i, s, b1, retTable, retStringTable = "
      "  test.return_list("
      "    true, 10, \"test\", false, testTable, testStringTable"
      "  );";
    luaScript.Execute(strScript.c_str());
    EXPECT_EQ(true,
        luaScript.GetVariable<LuaScript::Bool_LuaArg>("b").GetValue());
    EXPECT_EQ(10,
        luaScript.GetVariable<LuaScript::Int_LuaArg>("i").GetValue());
    EXPECT_EQ("test",
        luaScript.GetVariable<LuaScript::String_LuaArg>("s").GetValue());
    EXPECT_EQ(false,
        luaScript.GetVariable<LuaScript::Bool_LuaArg>("b1").GetValue());
  
    std::vector<int> retInt =
     luaScript.GetVariable<LuaScript::VectorInt_LuaArg>("retTable").GetValue();
    ASSERT_EQ(4, retInt.size());
    EXPECT_EQ(0, retInt[0]);
    EXPECT_EQ(1, retInt[1]);
    EXPECT_EQ(2, retInt[2]);
    EXPECT_EQ(3, retInt[3]);
    
    typedef LuaScript::VectorString_LuaArg LuaVectorString; 
    std::vector<std::string> retStr =
        luaScript.GetVariable<LuaVectorString>("retStringTable").GetValue();
    ASSERT_EQ(4, retStr.size());
    EXPECT_STREQ("test0", retStr[0].c_str());
    EXPECT_STREQ("test1", retStr[1].c_str());
    EXPECT_STREQ("test2", retStr[2].c_str());
    EXPECT_STREQ("qwertyuiop[]asdfghjkl;zxcvbnm,./`1234567890-=",
        retStr[3].c_str());
  }
  catch(const LuaScript::LuaException& e) 
  {
    FAIL() << "error: " << e.Error() << ", line " << e.Line();
  }
}

class TestLuaScript: public LuaScript
{
public:
  TestLuaScript(int a_Value)
  {
    m_TestParametr = a_Value;
  }
  int m_TestParametr;
};

class LuaTestParentFunction
{
 public:
  LuaTestParentFunction(LuaScript* a_Parent)
  {
    m_Parent = static_cast<TestLuaScript*>(a_Parent);
  }

  static const LuaScript::LuaArgArray* GetInputArgs()
  {
    LuaScript::LuaArgArray* args = new LuaScript::LuaArgArray();
    args->Add(new LuaScript::Int_LuaArg());
    return args;
  }

  static const LuaScript::LuaArgArray* GetOutputArgs()
  {
    LuaScript::LuaArgArray* args = new LuaScript::LuaArgArray();
    args->Add(new LuaScript::Bool_LuaArg());
    return args;
  }

  static const std::string NameSpace() { return "test"; }
  static const std::string Name() { return "check_parent"; }

  void Calc(const LuaScript::LuaArgArray& in, LuaScript::LuaArgArray& out) 
  {
    if( !m_Parent )
    {
      dynamic_cast<LuaScript::Bool_LuaArg&>(*out[0]).SetValue(false);
      return;
    }
    m_Parent->m_TestParametr =
            dynamic_cast<LuaScript::Int_LuaArg&>(*in[0]).GetValue();
    dynamic_cast<LuaScript::Bool_LuaArg&>(*out[0]).SetValue(true);
  }
 private:
  TestLuaScript* m_Parent;
};

TEST(LuaScript, CheckParent) 
{
  try 
  {
    TestLuaScript luaScript(1);
    luaScript.RegisterFunction<LuaTestParentFunction>();

    EXPECT_EQ(1, luaScript.m_TestParametr);
    luaScript.Execute("assert(test.check_parent(10));");

    EXPECT_EQ(10, luaScript.m_TestParametr);
  }
  catch(const LuaScript::LuaException& e) 
  {
    FAIL() << "error: " << e.Error() << ", line " << e.Line();
  }
}

class LuaNullNameSpaceFunction
{
 public:
  LuaNullNameSpaceFunction(LuaScript* a_Parent) 
  {
    m_Parent = a_Parent;
  }

  static const LuaScript::LuaArgArray* GetInputArgs()
  {
    LuaScript::LuaArgArray* args = new LuaScript::LuaArgArray();
    return args;
  }

  static const LuaScript::LuaArgArray* GetOutputArgs()
  {
    LuaScript::LuaArgArray* args = new LuaScript::LuaArgArray();
    args->Add(new LuaScript::Bool_LuaArg());
    return args;
  }

  static const std::string NameSpace() { return ""; }
  static const std::string Name() { return "return_true"; }

  void Calc(const LuaScript::LuaArgArray& in, LuaScript::LuaArgArray& out) 
  {
    EXPECT_EQ(static_cast<LuaScript*>(0), m_Parent);
    dynamic_cast<LuaScript::Bool_LuaArg&>(*out[0]).SetValue(true);
  }
 private:
  LuaScript* m_Parent;
};

TEST(LuaScript, CheckNullNameSpaceFunction) 
{
  try 
  {
    LuaScript luaScript;
    luaScript.RegisterFunction<LuaNullNameSpaceFunction>();

    luaScript.Execute("ret = return_true();");
    
    EXPECT_EQ(true,
        luaScript.GetVariable<LuaScript::Bool_LuaArg>("ret").GetValue());
  }
  catch(const LuaScript::LuaException& e) 
  {
    FAIL() << "error: " << e.Error() << ", line " << e.Line();
  }
}

class TestStackLuaScript: public LuaScript
{
 public:
  lua_State* GetLuaState() const;
};

lua_State* TestStackLuaScript::GetLuaState() const
{
  return m_LuaState;
}

class LuaStackChecker
{
 public:
  LuaStackChecker(lua_State* a_LuaState,
                      const char* a_FileName = "", int a_Line = 0);
  ~LuaStackChecker();

 private:
  void OnFail(const std::string& a_Message);
  
  lua_State*  m_LuaState;
  int         m_TopValue;
  const char* m_FileName;
  int         m_Line;
};

LuaStackChecker::LuaStackChecker(lua_State* a_LuaState,
                              const char* a_FileName, int a_Line):
  m_LuaState(a_LuaState),
  m_FileName(a_FileName),
  m_Line(a_Line)
{
  m_TopValue = lua_gettop(m_LuaState);
}

LuaStackChecker::~LuaStackChecker()
{
  int newTop = lua_gettop(m_LuaState);
  if( m_TopValue != newTop )
  {
    std::stringstream fmt;
    fmt << "Lua stack corrupted!\n"
      "File " << m_FileName << " line " << m_Line << "\n"
      "last_top = " << m_TopValue << " new_top " << newTop;
    OnFail(fmt.str());
  }
}

void LuaStackChecker::OnFail(const std::string& a_Message)
{
  FAIL() << a_Message;
}

#define CHECK_LUA_STACK(a_LuaState) \
                    LuaStackChecker guad(a_LuaState, __FILE__, __LINE__)

TEST(LuaScript, CheckStack_GetVariable)
{
  try 
  {
    TestStackLuaScript luaScript;
    CHECK_LUA_STACK(luaScript.GetLuaState());
    
    luaScript.Execute("b = true; i = 10; d = 1.5; s = 'str1'; vect1 = {1, 0}");
    {
      CHECK_LUA_STACK(luaScript.GetLuaState());
      EXPECT_EQ(true,
          luaScript.GetVariable<LuaScript::Bool_LuaArg>("b").GetValue());
    }

    {
      CHECK_LUA_STACK(luaScript.GetLuaState());
      EXPECT_EQ(10,
          luaScript.GetVariable<LuaScript::Int_LuaArg>("i").GetValue());
    }

    {
      CHECK_LUA_STACK(luaScript.GetLuaState());
      EXPECT_EQ(std::string("str1"),
        luaScript.GetVariable<LuaScript::String_LuaArg>("s").GetValue());
    }

    {
      CHECK_LUA_STACK(luaScript.GetLuaState());
      typedef LuaScript::VectorInt_LuaArg VectInt;
      EXPECT_EQ(2,
        luaScript.GetVariable<VectInt>("vect1").GetValue().size());
    }
  }
  catch(const LuaScript::LuaException& e) 
  {
    FAIL() << "error: " << e.Error() << ", line " << e.Line();
  }
}

TEST(LuaScript, CheckStack_SetVariable)
{
  try 
  {
    TestStackLuaScript luaScript;
    CHECK_LUA_STACK(luaScript.GetLuaState());
    
    {
      CHECK_LUA_STACK(luaScript.GetLuaState());
      luaScript.SetVariable<LuaScript::Bool_LuaArg>("b", true);
    }

    {
      CHECK_LUA_STACK(luaScript.GetLuaState());
      luaScript.SetVariable<LuaScript::Int_LuaArg>("i", 10);
    }

    {
      CHECK_LUA_STACK(luaScript.GetLuaState());
      luaScript.SetVariable<LuaScript::String_LuaArg>("s", "str1");
    }

    {
      CHECK_LUA_STACK(luaScript.GetLuaState());
      typedef LuaScript::VectorInt_LuaArg VectInt;
      luaScript.SetVariable<VectInt>("vect1", std::vector<int>(2));
    }
  }
  catch(const LuaScript::LuaException& e) 
  {
    FAIL() << "error: " << e.Error() << ", line " << e.Line();
  }
}

TEST(LuaScript, CheckStack_Function)
{
  try 
  {
    TestStackLuaScript luaScript;
    CHECK_LUA_STACK(luaScript.GetLuaState());

    {
      CHECK_LUA_STACK(luaScript.GetLuaState());
      luaScript.RegisterFunction< LuaReturnSomeFunction >();
    }

    {
      CHECK_LUA_STACK(luaScript.GetLuaState());
      luaScript.RegisterFunction< LuaNullNameSpaceFunction >();
    }
    
    luaScript.Execute("b, i, s = test.return_list(); b1 = return_true();");
  }
  catch(const LuaScript::LuaException& e) 
  {
    FAIL() << "error: " << e.Error() << ", line " << e.Line();
  }
}
