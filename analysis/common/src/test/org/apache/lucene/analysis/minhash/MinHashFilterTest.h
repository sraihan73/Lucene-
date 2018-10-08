#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class TokenStream;
}

namespace org::apache::lucene::analysis
{
class Tokenizer;
}
namespace org::apache::lucene::analysis::minhash
{
class MinHashFilter;
}
namespace org::apache::lucene::analysis::minhash
{
class LongPair;
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

namespace org::apache::lucene::analysis::minhash
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using LongPair =
    org::apache::lucene::analysis::minhash::MinHashFilter::LongPair;

/**
 * Tests for {@link MinHashFilter}
 */
class MinHashFilterTest : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(MinHashFilterTest)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testIntHash()
  virtual void testIntHash();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testStringHash() throws
  // java.io.UnsupportedEncodingException
  virtual void testStringHash() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSimpleOrder() throws
  // java.io.UnsupportedEncodingException
  virtual void testSimpleOrder() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testHashOrder()
  virtual void testHashOrder();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testHashNotRepeated()
  virtual void testHashNotRepeated();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMockShingleTokenizer() throws
  // java.io.IOException
  virtual void testMockShingleTokenizer() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTokenStreamSingleInput() throws
  // java.io.IOException
  virtual void testTokenStreamSingleInput() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTokenStream1() throws
  // java.io.IOException
  virtual void testTokenStream1() ;

private:
  std::deque<std::wstring>
  getTokens(std::shared_ptr<TokenStream> ts) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTokenStream2() throws
  // java.io.IOException
  virtual void testTokenStream2() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTokenStream3() throws
  // java.io.IOException
  virtual void testTokenStream3() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTokenStream4() throws
  // java.io.IOException
  virtual void testTokenStream4() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTokenStream5() throws
  // java.io.IOException
  virtual void testTokenStream5() ;

  static std::shared_ptr<TokenStream>
  createTokenStream(int shingleSize, const std::wstring &shingles,
                    int hashCount, int bucketCount, int hashSetSize,
                    bool withRotation);

private:
  static std::shared_ptr<Tokenizer>
  createMockShingleTokenizer(int shingleSize, const std::wstring &shingles);

  bool isLessThan(std::shared_ptr<LongPair> hash1,
                  std::shared_ptr<LongPair> hash2);

protected:
  std::shared_ptr<MinHashFilterTest> shared_from_this()
  {
    return std::static_pointer_cast<MinHashFilterTest>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::minhash
