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
// ORIGINAL LINE: @SuppressWarnings("unused") public class RomanianStemmer
// extends org.tartarus.snowball.SnowballProgram
class RomanianStemmer : public SnowballProgram
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

  static std::deque<wchar_t> const g_v;

  bool B_standard_suffix_removed = false;
  int I_p2 = 0;
  int I_p1 = 0;
  int I_pV = 0;

  void copy_from(std::shared_ptr<RomanianStemmer> other);

  bool r_prelude();

  bool r_mark_regions();

  bool r_postlude();

  bool r_RV();

  bool r_R1();

  bool r_R2();

  bool r_step_0();

  bool r_combo_suffix();

  bool r_standard_suffix();

  bool r_verb_suffix();

  bool r_vowel_suffix();

public:
  bool stem() override;

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<RomanianStemmer> shared_from_this()
  {
    return std::static_pointer_cast<RomanianStemmer>(
        org.tartarus.snowball.SnowballProgram::shared_from_this());
  }
};

} // #include  "core/src/java/org/tartarus/snowball/ext/
