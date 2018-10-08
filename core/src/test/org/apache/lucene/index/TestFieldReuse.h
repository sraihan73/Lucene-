#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

#include  "core/src/java/org/apache/lucene/index/IndexableFieldType.h"
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
namespace org::apache::lucene::index
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BytesRef = org::apache::lucene::util::BytesRef;

/** test tokenstream reuse by DefaultIndexingChain */
class TestFieldReuse : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestFieldReuse)

public:
  virtual void testStringField() ;

public:
  class MyField : public std::enable_shared_from_this<MyField>,
                  public IndexableField
  {
    GET_CLASS_NAME(MyField)
  public:
    std::shared_ptr<TokenStream> lastSeen;
    std::shared_ptr<TokenStream> lastReturned;

    std::wstring name() override;

    std::shared_ptr<IndexableFieldType> fieldType() override;

    std::shared_ptr<TokenStream>
    tokenStream(std::shared_ptr<Analyzer> analyzer,
                std::shared_ptr<TokenStream> reuse) override;

    std::shared_ptr<BytesRef> binaryValue() override;

    std::wstring stringValue() override;

    std::shared_ptr<Reader> readerValue() override;

    std::shared_ptr<Number> numericValue() override;
  };

public:
  virtual void testIndexWriterActuallyReuses() ;

protected:
  std::shared_ptr<TestFieldReuse> shared_from_this()
  {
    return std::static_pointer_cast<TestFieldReuse>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
