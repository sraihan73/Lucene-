#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::tartarus::snowball
{
class Among;
}

/*

Copyright (c) 2001, Dr Martin Porter
Copyright (c) 2002, Richard Boulton
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    * this deque of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    * notice, this deque of conditions and the following disclaimer in the
    * documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holders nor the names of its
contributors
    * may be used to endorse or promote products derived from this software
    * without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

namespace org::tartarus::snowball
{

/**
 * This is the rev 502 of the Snowball SVN trunk,
 * now located at <a target="_blank"
 * href="https://github.com/snowballstem/snowball/tree/e103b5c257383ee94a96e7fc58cab3c567bf079b">GitHub</a>,
 * but modified:
 * <ul>
 * <li>made abstract and introduced abstract method stem to avoid expensive
 * reflection in filter class. <li>refactored StringBuffers to StringBuilder
 * <li>uses char[] as buffer instead of StringBuffer/StringBuilder
 * <li>eq_s,eq_s_b,insert,replace_s take std::wstring like eq_v and eq_v_b
 * <li>use MethodHandles and fix <a target="_blank"
 * href="http://article.gmane.org/gmane.comp.search.snowball/1139">method
 * visibility bug</a>.
 * </ul>
 */
class SnowballProgram : public std::enable_shared_from_this<SnowballProgram>
{
  GET_CLASS_NAME(SnowballProgram)

protected:
  SnowballProgram();

public:
  virtual bool stem() = 0;

  /**
   * Set the current string.
   */
  virtual void setCurrent(const std::wstring &value);

  /**
   * Get the current string.
   */
  virtual std::wstring getCurrent();

  /**
   * Set the current string.
   * @param text character array containing input
   * @param length valid length of text.
   */
  virtual void setCurrent(std::deque<wchar_t> &text, int length);

  /**
   * Get the current buffer containing the stem.
   * <p>
   * NOTE: this may be a reference to a different character array than the
   * one originally provided with setCurrent, in the exceptional case that
   * stemming produced a longer intermediate or result string.
   * </p>
   * <p>
   * It is necessary to use {@link #getCurrentBufferLength()} to determine
   * the valid length of the returned buffer. For example, many words are
   * stemmed simply by subtracting from the length to remove suffixes.
   * </p>
   * @see #getCurrentBufferLength()
   */
  virtual std::deque<wchar_t> getCurrentBuffer();

  /**
   * Get the valid length of the character array in
   * {@link #getCurrentBuffer()}.
   * @return valid length of the array.
   */
  virtual int getCurrentBufferLength();

  // current string
private:
  std::deque<wchar_t> current;

protected:
  int cursor = 0;
  int limit = 0;
  int limit_backward = 0;
  int bra = 0;
  int ket = 0;

  virtual void copy_from(std::shared_ptr<SnowballProgram> other);

  virtual bool in_grouping(std::deque<wchar_t> &s, int min, int max);

  virtual bool in_grouping_b(std::deque<wchar_t> &s, int min, int max);

  virtual bool out_grouping(std::deque<wchar_t> &s, int min, int max);

  virtual bool out_grouping_b(std::deque<wchar_t> &s, int min, int max);

  virtual bool in_range(int min, int max);

  virtual bool in_range_b(int min, int max);

  virtual bool out_range(int min, int max);

  virtual bool out_range_b(int min, int max);

  virtual bool eq_s(int s_size, std::shared_ptr<std::wstring> s);

  virtual bool eq_s_b(int s_size, std::shared_ptr<std::wstring> s);

  virtual bool eq_v(std::shared_ptr<std::wstring> s);

  virtual bool eq_v_b(std::shared_ptr<std::wstring> s);

  virtual int find_among(std::deque<std::shared_ptr<Among>> &v, int v_size);

  // find_among_b is for backwards processing. Same comments apply
  virtual int find_among_b(std::deque<std::shared_ptr<Among>> &v, int v_size);

  /* to replace chars between c_bra and c_ket in current by the
   * chars in s.
   */
  virtual int replace_s(int c_bra, int c_ket, std::shared_ptr<std::wstring> s);

  virtual void slice_check();

  virtual void slice_from(std::shared_ptr<std::wstring> s);

  virtual void slice_del();

  virtual void insert(int c_bra, int c_ket, std::shared_ptr<std::wstring> s);

  /* Copy the slice into the supplied StringBuffer */
  virtual std::shared_ptr<StringBuilder>
  slice_to(std::shared_ptr<StringBuilder> s);

  virtual std::shared_ptr<StringBuilder>
  assign_to(std::shared_ptr<StringBuilder> s);

  /*
  extern void debug(struct SN_env * z, int number, int line_count)
  {   int i;
      int limit = SIZE(z->p);
      //if (number >= 0) printf("%3d (line %4d): '", number, line_count);
      if (number >= 0) printf("%3d (line %4d): [%d]'", number,
  line_count,limit); for (i = 0; i <= limit; i++) {   if (z->lb == i)
  printf("{"); if (z->bra == i) printf("["); if (z->c == i) printf("|"); if
  (z->ket == i) printf("]"); if (z->l == i) printf("}"); if (i < limit) {   int
  ch = z->p[i]; if (ch == 0) ch = '#'; printf("%c", ch);
          }
      }
      printf("'\n");
  }
  */
};

} // namespace org::tartarus::snowball
