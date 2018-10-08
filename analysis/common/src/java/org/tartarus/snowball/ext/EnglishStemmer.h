#pragma once
#include "../SnowballProgram.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/tartarus/snowball/Among.h"

// This file was generated automatically by the Snowball to Java compiler

namespace org::tartarus::snowball::ext
{

using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;

/**
 * This class was automatically generated by a Snowball to Java compiler
 * It implements the stemming algorithm defined by a snowball script.
 */

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unused") public class EnglishStemmer
// extends org.tartarus.snowball.SnowballProgram
class EnglishStemmer : public SnowballProgram
{

private:
  static constexpr int64_t serialVersionUID = 1LL;

  /* patched */
  static const std::shared_ptr<java::lang::invoke::MethodHandles::Lookup>
      methodObject;

  static std::deque<std::shared_ptr<Among>> const a_0;

  static std::deque<std::shared_ptr<Among>> const a_1;

  static std::deque<std::shared_ptr<Among>> const a_2;

  static std::deque<std::shared_ptr<Among>> const a_3;

  static std::deque<std::shared_ptr<Among>> const a_4;

  static std::deque<std::shared_ptr<Among>> const a_5;

  static std::deque<std::shared_ptr<Among>> const a_6;

  static std::deque<std::shared_ptr<Among>> const a_7;

  static std::deque<std::shared_ptr<Among>> const a_8;

  static std::deque<std::shared_ptr<Among>> const a_9;

  static std::deque<std::shared_ptr<Among>> const a_10;

  static std::deque<wchar_t> const g_v;

  static std::deque<wchar_t> const g_v_WXY;

  static std::deque<wchar_t> const g_valid_LI;

  bool B_Y_found = false;
  int I_p2 = 0;
  int I_p1 = 0;

  void copy_from(std::shared_ptr<EnglishStemmer> other);

  bool r_prelude();

  bool r_mark_regions();

  bool r_shortv();

  bool r_R1();

  bool r_R2();

  bool r_Step_1a();

  bool r_Step_1b();

  bool r_Step_1c();

  bool r_Step_2();

  bool r_Step_3();

  bool r_Step_4();

  bool r_Step_5();

  bool r_exception2();

  bool r_exception1();

  bool r_postlude();

public:
  bool stem() override;

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<EnglishStemmer> shared_from_this()
  {
    return std::static_pointer_cast<EnglishStemmer>(
        org.tartarus.snowball.SnowballProgram::shared_from_this());
  }
};

} // #include  "core/src/java/org/tartarus/snowball/ext/
