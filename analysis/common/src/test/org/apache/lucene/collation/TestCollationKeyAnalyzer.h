#pragma once
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CollationTestBase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::collation
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using CollationTestBase = org::apache::lucene::analysis::CollationTestBase;
using BytesRef = org::apache::lucene::util::BytesRef;

class TestCollationKeyAnalyzer : public CollationTestBase
{
  GET_CLASS_NAME(TestCollationKeyAnalyzer)
  // Neither Java 1.4.2 nor 1.5.0 has Farsi Locale collation available in
  // RuleBasedCollator.  However, the Arabic Locale seems to order the Farsi
  // characters properly.
private:
  std::shared_ptr<Collator> collator =
      Collator::getInstance(std::make_shared<Locale>(L"ar"));
  std::shared_ptr<Analyzer> analyzer;

public:
  void setUp()  override;

  void tearDown()  override;

private:
  std::shared_ptr<BytesRef> firstRangeBeginning = std::make_shared<BytesRef>(
      collator->getCollationKey(firstRangeBeginningOriginal).toByteArray());
  std::shared_ptr<BytesRef> firstRangeEnd = std::make_shared<BytesRef>(
      collator->getCollationKey(firstRangeEndOriginal).toByteArray());
  std::shared_ptr<BytesRef> secondRangeBeginning = std::make_shared<BytesRef>(
      collator->getCollationKey(secondRangeBeginningOriginal).toByteArray());
  std::shared_ptr<BytesRef> secondRangeEnd = std::make_shared<BytesRef>(
      collator->getCollationKey(secondRangeEndOriginal).toByteArray());

public:
  virtual void testFarsiRangeFilterCollating() ;

  virtual void testFarsiRangeQueryCollating() ;

  virtual void testFarsiTermRangeQuery() ;

  virtual void testThreadSafe() ;

protected:
  std::shared_ptr<TestCollationKeyAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestCollationKeyAnalyzer>(
        org.apache.lucene.analysis.CollationTestBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/collation/
