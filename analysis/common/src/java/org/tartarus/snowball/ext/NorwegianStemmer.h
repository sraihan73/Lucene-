#pragma once
#include "../SnowballProgram.h"
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
// ORIGINAL LINE: @SuppressWarnings("unused") public class NorwegianStemmer
// extends org.tartarus.snowball.SnowballProgram
class NorwegianStemmer : public SnowballProgram
{

private:
  static constexpr int64_t serialVersionUID = 1LL;

  /* patched */
  static const std::shared_ptr<java::lang::invoke::MethodHandles::Lookup>
      methodObject;

  static std::deque<std::shared_ptr<Among>> const a_0;

  static std::deque<std::shared_ptr<Among>> const a_1;

  static std::deque<std::shared_ptr<Among>> const a_2;

  static std::deque<wchar_t> const g_v;

  static std::deque<wchar_t> const g_s_ending;

  int I_x = 0;
  int I_p1 = 0;

  void copy_from(std::shared_ptr<NorwegianStemmer> other);

  bool r_mark_regions();

  bool r_main_suffix();

  bool r_consonant_pair();

  bool r_other_suffix();

public:
  bool stem() override;

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<NorwegianStemmer> shared_from_this()
  {
    return std::static_pointer_cast<NorwegianStemmer>(
        org.tartarus.snowball.SnowballProgram::shared_from_this());
  }
};

} // namespace org::tartarus::snowball::ext
