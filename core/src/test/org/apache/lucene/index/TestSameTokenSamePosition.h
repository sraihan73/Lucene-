#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
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

using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestSameTokenSamePosition : public LuceneTestCase
{
  GET_CLASS_NAME(TestSameTokenSamePosition)

  /**
   * Attempt to reproduce an assertion error that happens
   * only with the trunk version around April 2011.
   */
public:
  virtual void test() ;

  /**
   * Same as the above, but with more docs
   */
  virtual void testMoreDocs() ;

protected:
  std::shared_ptr<TestSameTokenSamePosition> shared_from_this()
  {
    return std::static_pointer_cast<TestSameTokenSamePosition>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

class BugReproTokenStream final : public TokenStream
{
  GET_CLASS_NAME(BugReproTokenStream)
private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  static constexpr int TOKEN_COUNT = 4;
  int nextTokenIndex = 0;
  std::deque<std::wstring> const terms =
      std::deque<std::wstring>{L"six", L"six", L"drunken", L"drunken"};
  std::deque<int> const starts = std::deque<int>{0, 0, 4, 4};
  std::deque<int> const ends = std::deque<int>{3, 3, 11, 11};
  std::deque<int> const incs = std::deque<int>{1, 0, 1, 0};

public:
  bool incrementToken() override;

  void reset()  override;

protected:
  std::shared_ptr<BugReproTokenStream> shared_from_this()
  {
    return std::static_pointer_cast<BugReproTokenStream>(
        org.apache.lucene.analysis.TokenStream::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
