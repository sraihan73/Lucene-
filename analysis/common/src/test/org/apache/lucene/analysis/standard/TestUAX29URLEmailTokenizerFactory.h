#pragma once
#include "../util/BaseTokenStreamFactoryTestCase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
namespace org::apache::lucene::analysis::standard
{

using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

/**
 * A few tests based on org.apache.lucene.analysis.TestUAX29URLEmailTokenizer
 */

class TestUAX29URLEmailTokenizerFactory : public BaseTokenStreamFactoryTestCase
{
  GET_CLASS_NAME(TestUAX29URLEmailTokenizerFactory)

public:
  virtual void testUAX29URLEmailTokenizer() ;

  virtual void testArabic() ;

  virtual void testChinese() ;

  virtual void testKorean() ;

  virtual void testHyphen() ;

  // Test with some URLs from TestUAX29URLEmailTokenizer's
  // urls.from.random.text.with.urls.txt
  virtual void testURLs() ;

  // Test with some emails from TestUAX29URLEmailTokenizer's
  // email.addresses.from.random.text.with.email.addresses.txt
  virtual void testEmails() ;

  virtual void testMaxTokenLength() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusArguments() ;

  virtual void testIllegalArguments() ;

protected:
  std::shared_ptr<TestUAX29URLEmailTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestUAX29URLEmailTokenizerFactory>(
        org.apache.lucene.analysis.util
            .BaseTokenStreamFactoryTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/standard/
