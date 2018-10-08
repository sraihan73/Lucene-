#pragma once
#include "../util/BaseTokenStreamFactoryTestCase.h"
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
namespace org::apache::lucene::analysis::compound
{

using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

/**
 * Simple tests to ensure the Hyphenation compound filter factory is working.
 */
class TestHyphenationCompoundWordTokenFilterFactory
    : public BaseTokenStreamFactoryTestCase
{
  GET_CLASS_NAME(TestHyphenationCompoundWordTokenFilterFactory)
  /**
   * Ensure the factory works with hyphenation grammar+dictionary: using default
   * options.
   */
public:
  virtual void testHyphenationWithDictionary() ;

  /**
   * Ensure the factory works with no dictionary: using hyphenation grammar
   * only. Also change the min/max subword sizes from the default. When using no
   * dictionary, it's generally necessary to tweak these, or you get lots of
   * expansions.
   */
  virtual void testHyphenationOnly() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusArguments() ;

protected:
  std::shared_ptr<TestHyphenationCompoundWordTokenFilterFactory>
  shared_from_this()
  {
    return std::static_pointer_cast<
        TestHyphenationCompoundWordTokenFilterFactory>(
        org.apache.lucene.analysis.util
            .BaseTokenStreamFactoryTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/compound/
