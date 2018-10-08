#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArrayMap.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/util/OpenStringBuilder.h"

namespace org::apache::lucene::analysis
{
template <typename V>
class CharArrayMap;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
This file was partially derived from the
original CIIR University of Massachusetts Amherst version of KStemmer.java
(license for the original shown below)
 */

/*
 Copyright © 2003,
 Center for Intelligent Information Retrieval,
 University of Massachusetts, Amherst.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 deque of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this deque of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 3. The names "Center for Intelligent Information Retrieval" and
 "University of Massachusetts" must not be used to endorse or promote products
 derived from this software without prior written permission. To obtain
 permission, contact info@ciir.cs.umass.edu.

 THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF MASSACHUSETTS AND OTHER CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.
 */
namespace org::apache::lucene::analysis::en
{

using CharArrayMap = org::apache::lucene::analysis::CharArrayMap;
using OpenStringBuilder =
    org::apache::lucene::analysis::util::OpenStringBuilder;
/**
 * <p>Title: Kstemmer</p>
 * <p>Description: This is a java version of Bob Krovetz' kstem stemmer</p>
 * <p>Copyright: Copyright 2008, Luicid Imagination, Inc. </p>
 * <p>Copyright: Copyright 2003, CIIR University of Massachusetts Amherst
 * (http://ciir.cs.umass.edu) </p>
 */

/**
 * This class implements the Kstem algorithm
 */
class KStemmer : public std::enable_shared_from_this<KStemmer>
{
  GET_CLASS_NAME(KStemmer)
private:
  static constexpr int MaxWordLen = 50;

  static std::deque<std::wstring> const exceptionWords;

  static std::deque<std::deque<std::wstring>> const directConflations;

  static std::deque<std::deque<std::wstring>> const countryNationality;

  static std::deque<std::wstring> const supplementDict;

  static std::deque<std::wstring> const properNouns;

public:
  class DictEntry : public std::enable_shared_from_this<DictEntry>
  {
    GET_CLASS_NAME(DictEntry)
  public:
    bool exception = false;
    std::wstring root;

    DictEntry(const std::wstring &root, bool isException);
  };

private:
  static const std::shared_ptr<CharArrayMap<std::shared_ptr<DictEntry>>>
      dict_ht;

  /***
   * caching off private int maxCacheSize; private CharArrayMap<std::wstring> cache =
   * null; private static final std::wstring SAME = "SAME"; // use if stemmed form is
   * the same
   ***/

  const std::shared_ptr<OpenStringBuilder> word =
      std::make_shared<OpenStringBuilder>();
  int j = 0; // index of final letter in stem (within word)
  int k = 0; /*
              * INDEX of final letter in word. You must add 1 to k to get
              * the current length of word. When you want the length of
              * word, use the method wordLength, which returns (k+1).
              */

  /*
   * private void initializeStemHash() { if (maxCacheSize > 0) cache = new
   * CharArrayMap<std::wstring>(maxCacheSize,false); }
   ***/

  wchar_t finalChar();

  wchar_t penultChar();

  bool isVowel(int index);

  bool isCons(int index);

  static std::shared_ptr<CharArrayMap<std::shared_ptr<DictEntry>>>
  initializeDictHash();

  bool isAlpha(wchar_t ch);

  /* length of stem within word */
  int stemLength();

  bool endsIn(std::deque<wchar_t> &s);

  bool endsIn(wchar_t a, wchar_t b);

  bool endsIn(wchar_t a, wchar_t b, wchar_t c);

  bool endsIn(wchar_t a, wchar_t b, wchar_t c, wchar_t d);

  std::shared_ptr<DictEntry> wordInDict();

  /* Convert plurals to singular form, and '-ies' to 'y' */
  void plural();

  void setSuffix(const std::wstring &s);

  /* replace old suffix with s */
  void setSuff(const std::wstring &s, int len);

  /* Returns true if the word is found in the dictionary */
  // almost all uses of lookup() return immediately and are
  // followed by another lookup in the dict. Store the match
  // to avoid this double lookup.
public:
  std::shared_ptr<DictEntry> matchedEntry = nullptr;

private:
  bool lookup();

  // Set<std::wstring> lookups = new HashSet<>();

  /* convert past tense (-ed) to present, and `-ied' to `y' */
  void pastTense();

  /* return TRUE if word ends with a double consonant */
  bool doubleC(int i);

  bool vowelInStem();

  /* handle `-ing' endings */
  void aspect();

  /*
   * this routine deals with -ity endings. It accepts -ability, -ibility, and
   * -ality, even without checking the dictionary because they are so
   * productive. The first two are mapped to -ble, and the -ity is remove for
   * the latter
   */
  void ityEndings();

  /* handle -ence and -ance */
  void nceEndings();

  /* handle -ness */
  void nessEndings();

  /* handle -ism */
  void ismEndings();

  /* this routine deals with -ment endings. */
  void mentEndings();

  /* this routine deals with -ize endings. */
  void izeEndings();

  /* handle -ency and -ancy */
  void ncyEndings();

  /* handle -able and -ible */
  void bleEndings();

  /*
   * handle -ic endings. This is fairly straightforward, but this is also the
   * only place we try *expanding* an ending, -ic -> -ical. This is to handle
   * cases like `canonic' -> `canonical'
   */
  void icEndings();

  static std::deque<wchar_t> ization;
  static std::deque<wchar_t> ition;
  static std::deque<wchar_t> ation;
  static std::deque<wchar_t> ication;

  /* handle some derivational endings */
  /*
   * this routine deals with -ion, -ition, -ation, -ization, and -ication. The
   * -ization ending is always converted to -ize
   */
  void ionEndings();

  /*
   * this routine deals with -er, -or, -ier, and -eer. The -izer ending is
   * always converted to -ize
   */
  void erAndOrEndings();

  /*
   * this routine deals with -ly endings. The -ally ending is always converted
   * to -al Sometimes this will temporarily leave us with a non-word (e.g.,
   * heuristically maps to heuristical), but then the -al is removed in the next
   * step.
   */
  void lyEndings();

  /*
   * this routine deals with -al endings. Some of the endings from the previous
   * routine are finished up here.
   */
  void alEndings();

  /*
   * this routine deals with -ive endings. It normalizes some of the -ative
   * endings directly, and also maps some -ive endings to -ion.
   */
  void iveEndings();

public:
  KStemmer();

  virtual std::wstring stem(const std::wstring &term);

  /**
   * Returns the result of the stem (assuming the word was changed) as a std::wstring.
   */
  virtual std::wstring asString();

  virtual std::shared_ptr<std::wstring> asCharSequence();

  virtual std::wstring getString();

  virtual std::deque<wchar_t> getChars();

  virtual int getLength();

  std::wstring result;

private:
  bool matched();

  /**
   * Stems the text in the token. Returns true if changed.
   */
public:
  virtual bool stem(std::deque<wchar_t> &term, int len);
};

} // #include  "core/src/java/org/apache/lucene/analysis/en/
