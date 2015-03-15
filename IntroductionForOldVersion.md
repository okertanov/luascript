# Calling Lua code from C++ code #

This example passes the variable `a` set to "test" to the script, the script appends "123" to it and assigns the result to the variable `b`. The C++ program picks the variable `b` up and prints "test123".

```
  #include "luascript/luascript.h"
  ...
  try {
    lua script;
    script.set_variable<lua::string_arg_t>("a", "test");
    script.exec("b = a .. '123';");
    std::cout << script.get_variable<lua::string_arg_t>("b").value());
  } catch (lua::exception& e) {
    std::cerr << "error: " << e.error() << ", line " << e.line();
  }
```

# Adding a new function #

```
class file_exists_func_t {
 public:

  // This method registers the function arguments.
  // Available type are:
  // - lua::string_arg_t
  // - lua::int_arg_t
  // - lua::bool_arg_t
  static const lua::args_t* in_args() {
    lua::args_t* args = new lua::args_t();
    args->add(new lua::string_arg_t());
    return args;
  }

  // This method registers the set of return values.
  // Lua allows to have multiple return values.
  // Available type are:
  // - lua::string_arg_t
  // - lua::int_arg_t
  // - lua::bool_arg_t
  static const lua::args_t* out_args() {
    lua::args_t* args = new lua::args_t();
    args->add(new lua::bool_arg_t());
    return args;
  }

  // These two methods set the function name: namespace and name.
  // In this example the function name is "fs.file_exists".
  static const std::string ns() { return "fs"; }
  static const std::string name() { return "file_exists"; }

  // This method performs the function logic. 
  // The method has to parse the arguments and fill in the array of
  // return values. The method must take care that it uses valid types of 
  // arguments and return values.
  static void calc(const lua::args_t& in, lua::args_t& out) {
    std::string filename = dynamic_cast<lua::string_arg_t&>(*in[0]).value();
    std::ifstream is(filename.c_str());
    dynamic_cast<lua::bool_arg_t&>(*out[0]).value() = is.good();
  }
};
...
  try {
    // Create the script executor.
    lua script;
    // Register new function "fs.file_exists()".
    script.register_function< file_exists_func_t >();
    // Set the variable "fname" to "readme.txt".
    script.set_variable<lua::string_arg_t>("fname", "readme.txt");
    // Execute the script calling this function.
    script.exec("exists = fs.file_exists(fname);");
    // Call "get_variable" method to extract the value of the
    // "exists" variable.
    bool exists = script.get_variable<lua::bool_arg_t>("exists").value();
  } catch (lua::exception& e) {
    std::cerr << "error: " << e.error() << ", line " << e.line();
  }
...
```