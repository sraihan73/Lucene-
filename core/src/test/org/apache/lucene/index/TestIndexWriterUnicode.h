#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
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
namespace org::apache::lucene::index
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestIndexWriterUnicode : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexWriterUnicode)

public:
  std::deque<std::wstring> const utf8Data =
      std::deque<std::wstring>{L"ab\udc17cd",
                                L"ab\ufffdcd",
                                L"\udc17abcd",
                                L"\ufffdabcd",
                                L"\udc17",
                                L"\ufffd",
                                L"ab\udc17\udc17cd",
                                L"ab\ufffd\ufffdcd",
                                L"\udc17\udc17abcd",
                                L"\ufffd\ufffdabcd",
                                L"\udc17\udc17",
                                L"\ufffd\ufffd",
                                L"ab\ud917cd",
                                L"ab\ufffdcd",
                                L"\ud917abcd",
                                L"\ufffdabcd",
                                L"\ud917",
                                L"\ufffd",
                                L"ab\ud917\ud917cd",
                                L"ab\ufffd\ufffdcd",
                                L"\ud917\ud917abcd",
                                L"\ufffd\ufffdabcd",
                                L"\ud917\ud917",
                                L"\ufffd\ufffd",
                                L"ab\udc17\ud917cd",
                                L"ab\ufffd\ufffdcd",
                                L"\udc17\ud917abcd",
                                L"\ufffd\ufffdabcd",
                                L"\udc17\ud917",
                                L"\ufffd\ufffd",
                                L"ab\udc17\ud917\udc17\ud917cd",
                                L"ab\ufffd\ud917\udc17\ufffdcd",
                                L"\udc17\ud917\udc17\ud917abcd",
                                L"\ufffd\ud917\udc17\ufffdabcd",
                                L"\udc17\ud917\udc17\ud917",
                                L"\ufffd\ud917\udc17\ufffd"};

private:
  int nextInt(int lim);

  int nextInt(int start, int end);

  bool fillUnicode(std::deque<wchar_t> &buffer, std::deque<wchar_t> &expected,
                   int offset, int count);

  // both start & end are inclusive
  int getInt(std::shared_ptr<Random> r, int start, int end);

  std::wstring asUnicodeChar(wchar_t c);

  std::wstring termDesc(const std::wstring &s);

  void checkTermsOrder(std::shared_ptr<IndexReader> r,
                       std::shared_ptr<Set<std::wstring>> allTerms,
                       bool isTop) ;

  // LUCENE-510
public:
  virtual void testRandomUnicodeStrings() ;

  // LUCENE-510
  virtual void testAllUnicodeChars() ;

  virtual void testEmbeddedFFFF() ;

  // LUCENE-510
  virtual void testInvalidUTF16() ;

  // Make sure terms, including ones with surrogate pairs,
  // sort in codepoint sort order by default
  virtual void testTermUTF16SortOrder() ;

protected:
  std::shared_ptr<TestIndexWriterUnicode> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterUnicode>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
