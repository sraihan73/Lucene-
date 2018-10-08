#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

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
namespace org::apache::lucene
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * validate that assertions are enabled during tests
 */
class TestAssertions : public LuceneTestCase
{
  GET_CLASS_NAME(TestAssertions)

public:
  class TestTokenStream1 : public TokenStream
  {
    GET_CLASS_NAME(TestTokenStream1)
  public:
    bool incrementToken() override final;

  protected:
    std::shared_ptr<TestTokenStream1> shared_from_this()
    {
      return std::static_pointer_cast<TestTokenStream1>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

public:
  class TestTokenStream2 final : public TokenStream
  {
    GET_CLASS_NAME(TestTokenStream2)
  public:
    bool incrementToken() override;

  protected:
    std::shared_ptr<TestTokenStream2> shared_from_this()
    {
      return std::static_pointer_cast<TestTokenStream2>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

public:
  class TestTokenStream3 : public TokenStream
  {
    GET_CLASS_NAME(TestTokenStream3)
  public:
    bool incrementToken() override;

  protected:
    std::shared_ptr<TestTokenStream3> shared_from_this()
    {
      return std::static_pointer_cast<TestTokenStream3>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

public:
  virtual void testTokenStreams();

protected:
  std::shared_ptr<TestAssertions> shared_from_this()
  {
    return std::static_pointer_cast<TestAssertions>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/
