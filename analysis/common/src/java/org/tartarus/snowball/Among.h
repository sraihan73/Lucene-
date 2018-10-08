#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

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
class Among final : public std::enable_shared_from_this<Among>
{
  GET_CLASS_NAME(Among)

public:
  Among(const std::wstring &s, int substring_i, int result,
        const std::wstring &methodname,
        std::shared_ptr<MethodHandles::Lookup> methodobject);

  const int s_size;             // search string
  std::deque<wchar_t> const s; // search string
  const int substring_i;        // index to longest matching substring
  const int result;             // result of the lookup

  // Make sure this is not accessible outside package for Java security reasons!
  const std::shared_ptr<MethodHandle>
      method; // method to use if substring matches
};
} // #include  "core/src/java/org/tartarus/snowball/
