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
// ORIGINAL LINE: @SuppressWarnings("unused") public class TurkishStemmer
// extends org.tartarus.snowball.SnowballProgram
class TurkishStemmer : public SnowballProgram
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

  static std::deque<std::shared_ptr<Among>> const a_11;

  static std::deque<std::shared_ptr<Among>> const a_12;

  static std::deque<std::shared_ptr<Among>> const a_13;

  static std::deque<std::shared_ptr<Among>> const a_14;

  static std::deque<std::shared_ptr<Among>> const a_15;

  static std::deque<std::shared_ptr<Among>> const a_16;

  static std::deque<std::shared_ptr<Among>> const a_17;

  static std::deque<std::shared_ptr<Among>> const a_18;

  static std::deque<std::shared_ptr<Among>> const a_19;

  static std::deque<std::shared_ptr<Among>> const a_20;

  static std::deque<std::shared_ptr<Among>> const a_21;

  static std::deque<std::shared_ptr<Among>> const a_22;

  static std::deque<std::shared_ptr<Among>> const a_23;

  static std::deque<wchar_t> const g_vowel;

  static std::deque<wchar_t> const g_U;

  static std::deque<wchar_t> const g_vowel1;

  static std::deque<wchar_t> const g_vowel2;

  static std::deque<wchar_t> const g_vowel3;

  static std::deque<wchar_t> const g_vowel4;

  static std::deque<wchar_t> const g_vowel5;

  static std::deque<wchar_t> const g_vowel6;

  bool B_continue_stemming_noun_suffixes = false;
  int I_strlen = 0;

  void copy_from(std::shared_ptr<TurkishStemmer> other);

  bool r_check_vowel_harmony();

  bool r_mark_suffix_with_optional_n_consonant();

  bool r_mark_suffix_with_optional_s_consonant();

  bool r_mark_suffix_with_optional_y_consonant();

  bool r_mark_suffix_with_optional_U_vowel();

  bool r_mark_possessives();

  bool r_mark_sU();

  bool r_mark_lArI();

  bool r_mark_yU();

  bool r_mark_nU();

  bool r_mark_nUn();

  bool r_mark_yA();

  bool r_mark_nA();

  bool r_mark_DA();

  bool r_mark_ndA();

  bool r_mark_DAn();

  bool r_mark_ndAn();

  bool r_mark_ylA();

  bool r_mark_ki();

  bool r_mark_ncA();

  bool r_mark_yUm();

  bool r_mark_sUn();

  bool r_mark_yUz();

  bool r_mark_sUnUz();

  bool r_mark_lAr();

  bool r_mark_nUz();

  bool r_mark_DUr();

  bool r_mark_cAsInA();

  bool r_mark_yDU();

  bool r_mark_ysA();

  bool r_mark_ymUs_();

  bool r_mark_yken();

  bool r_stem_nominal_verb_suffixes();

  bool r_stem_suffix_chain_before_ki();

  bool r_stem_noun_suffixes();

  bool r_post_process_last_consonants();

  bool r_append_U_to_stems_ending_with_d_or_g();

  bool r_more_than_one_syllable_word();

  bool r_is_reserved_word();

  bool r_postlude();

public:
  bool stem() override;

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<TurkishStemmer> shared_from_this()
  {
    return std::static_pointer_cast<TurkishStemmer>(
        org.tartarus.snowball.SnowballProgram::shared_from_this());
  }
};

} // #include  "core/src/java/org/tartarus/snowball/ext/
