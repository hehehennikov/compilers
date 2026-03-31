#include <gtest/gtest.h>

#include <user.hpp>

std::string RunAndCapture(std::string_view code) {
  std::stringstream buffer;
  std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

  auto result = RunCompiler(code);
  std::cout.rdbuf(old);

  return result ? buffer.str() : "ERROR";
}

bool TestCode(std::string_view code) {
  if (auto result = RunCompiler(code); not result) {
    return false;
  }

  return true;
}

TEST(LexerUnit, Declare) {
  std::string_view code = R"(
    var x: int;
  )";

  EXPECT_TRUE(TestCode(code));
}

TEST(LexerUnit, Assign) {
  std::string code = R"(
    var x: int;
    x = 42;
  )";

  EXPECT_TRUE(TestCode(code));
}

TEST(LexerUnit, PrintNum) {
  std::string code = R"(
    print(42);
  )";

  EXPECT_EQ(RunAndCapture(code), "42\n");
}

TEST(LexerUnit, PrintVar) {
  std::string code = R"(
    var x: int;
    x = 42;
    print(x);
  )";

  EXPECT_EQ(RunAndCapture(code), "42\n");
}

TEST(LexerUnit, PrintMultipleVars) {
  std::string code = R"(
    var x: int;
    var y: int;
    var z: int;

    x = 42;
    y = 17;
    z = 91;

    print(x);
    print(y);
    print(z);
    print(1337);
  )";

  EXPECT_EQ(RunAndCapture(code), "42\n17\n91\n1337\n");
}

TEST(LexerUnit, MultipleDeclare) {
  std::string code = R"(
    var x: int;
    var y: int;
    var z: int;
  )";

  EXPECT_TRUE(TestCode(code));
}

TEST(LexerUnit, MultipleAssign) {
  std::string code = R"(
    var x: int;
    x = 1;
    var y: int;
    y = 2;
    var z: int;
    z = 3;
  )";

  EXPECT_TRUE(TestCode(code));
}

TEST(LexerUnit, CondExpr) {
  {
    std::string code = R"(
      var x: int;
      x = 10;
      if (x == 10) {
        print(42);
      } else {}
    )";

    EXPECT_EQ(RunAndCapture(code), "42\n");
  }
  {
    std::string code = R"(
      var x: int;
      x = 11;
      if (x == 10) {}
      else {
        print(42);
      }
    )";

    EXPECT_EQ(RunAndCapture(code), "42\n");
  }
}

TEST(LexerUnit, NestedCondExpr) {
  {
    std::string code = R"(
      var x: int;
      var y: int;
      x = 10;
      y = 42;
      if (x == 10) {
        if (y == 42) {
          print(42);
        }
        else {}
      } else {}
    )";

    EXPECT_EQ(RunAndCapture(code), "42\n");
  }
  {
    std::string code = R"(
      var x: int;
      var y: int;
      x = 11;
      y = 42;

      if (x == 10) {}
      else {
        if (y == 42) {
          print(42);
        }
        else {}
      }
    )";

    EXPECT_EQ(RunAndCapture(code), "42\n");
  }
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}