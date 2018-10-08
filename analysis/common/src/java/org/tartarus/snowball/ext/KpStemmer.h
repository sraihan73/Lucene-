#pragma once
#include "../SnowballProgram.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::tartarus::snowball
{
class Among;
}

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
// ORIGINAL LINE: @SuppressWarnings("unused") public class KpStemmer extends
// org.tartarus.snowball.SnowballProgram
class KpStemmer : public SnowballProgram
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

  static std::deque<wchar_t> const g_v;

  static std::deque<wchar_t> const g_v_WX;

  static std::deque<wchar_t> const g_AOU;

  static std::deque<wchar_t> const g_AIOU;

  bool B_GE_removed = false;
  bool B_stemmed = false;
  bool B_Y_found = false;
  int I_p2 = 0;
  int I_p1 = 0;
  int I_x = 0;
  std::shared_ptr<StringBuilder> S_ch = std::make_shared<StringBuilder>();

  void copy_from(std::shared_ptr<KpStemmer> other);

  bool r_R1();

  bool r_R2();

  bool r_V();

  bool r_VX();

  bool r_C();

  bool r_lengthen_V();

  bool r_Step_1();

  bool r_Step_2();

  bool r_Step_3();

  bool r_Step_4();

  bool r_Step_7();

  bool r_Step_6();

  bool r_Step_1c();

  bool r_Lose_prefix();

  bool r_Lose_infix();

  bool r_measure();

public:
  bool stem() override;

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<KpStemmer> shared_from_this()
  {
    return std::static_pointer_cast<KpStemmer>(
        org.tartarus.snowball.SnowballProgram::shared_from_this());
  }
};

} // namespace org::tartarus::snowball::ext