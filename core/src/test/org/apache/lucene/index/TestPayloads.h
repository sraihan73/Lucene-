#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/index/ByteArrayPool.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

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
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestPayloads : public LuceneTestCase
{
  GET_CLASS_NAME(TestPayloads)

  // Simple tests to test the Payload class
public:
  virtual void testPayload() ;

  // Tests whether the DocumentWriter and SegmentMerger correctly enable the
  // payload bit in the FieldInfo
  virtual void testPayloadFieldBit() ;

  // Tests if payloads are correctly stored and loaded using both RamDirectory
  // and FSDirectory
  virtual void testPayloadsEncoding() ;

  // builds an index with payloads in the given Directory and performs
  // different tests to verify the payload encoding
private:
  void performTest(std::shared_ptr<Directory> dir) ;

public:
  static const std::shared_ptr<Charset> utf8;

private:
  void generateRandomData(std::deque<char> &data);

  std::deque<char> generateRandomData(int n);

  std::deque<std::shared_ptr<Term>>
  generateTerms(const std::wstring &fieldName, int n);

public:
  virtual void assertByteArrayEquals(std::deque<char> &b1,
                                     std::deque<char> &b2);

  virtual void assertByteArrayEquals(std::deque<char> &b1,
                                     std::deque<char> &b2, int b2offset,
                                     int b2length);

public:
  class PayloadData : public std::enable_shared_from_this<PayloadData>
  {
    GET_CLASS_NAME(PayloadData)
  public:
    std::deque<char> data;
    int offset = 0;
    int length = 0;

    PayloadData(std::deque<char> &data, int offset, int length);
  };

  /**
   * This Analyzer uses an MockTokenizer and PayloadFilter.
   */
private:
  class PayloadAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(PayloadAnalyzer)
  public:
    std::unordered_map<std::wstring, std::shared_ptr<PayloadData>> fieldToData =
        std::unordered_map<std::wstring, std::shared_ptr<PayloadData>>();

    PayloadAnalyzer();

    PayloadAnalyzer(const std::wstring &field, std::deque<char> &data,
                    int offset, int length);

    virtual void setPayloadData(const std::wstring &field,
                                std::deque<char> &data, int offset,
                                int length);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<PayloadAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<PayloadAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  /**
   * This Filter adds payloads to the tokens.
   */
private:
  class PayloadFilter : public TokenFilter
  {
    GET_CLASS_NAME(PayloadFilter)
  public:
    std::shared_ptr<PayloadAttribute> payloadAtt;
    std::shared_ptr<CharTermAttribute> termAttribute;

  private:
    std::unordered_map<std::wstring, std::shared_ptr<PayloadData>> fieldToData;
    std::wstring fieldName;
    std::shared_ptr<PayloadData> payloadData;
    int offset = 0;

  public:
    PayloadFilter(std::shared_ptr<TokenStream> in_,
                  const std::wstring &fieldName,
                  std::unordered_map<std::wstring, std::shared_ptr<PayloadData>>
                      &fieldToData);

    bool incrementToken()  override;

    void reset()  override;

  protected:
    std::shared_ptr<PayloadFilter> shared_from_this()
    {
      return std::static_pointer_cast<PayloadFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

public:
  virtual void testThreadSafety() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestPayloads> outerInstance;

    int numDocs = 0;
    std::shared_ptr<org::apache::lucene::index::TestPayloads::ByteArrayPool>
        pool;
    std::shared_ptr<org::apache::lucene::index::IndexWriter> writer;
    std::wstring field;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestPayloads> outerInstance, int numDocs,
        std::shared_ptr<org::apache::lucene::index::TestPayloads::ByteArrayPool>
            pool,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> writer,
        const std::wstring &field);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

private:
  class PoolingPayloadTokenStream : public TokenStream
  {
    GET_CLASS_NAME(PoolingPayloadTokenStream)
  private:
    std::shared_ptr<TestPayloads> outerInstance;

    std::deque<char> payload;
    bool first = false;
    std::shared_ptr<ByteArrayPool> pool;
    std::wstring term;

  public:
    std::shared_ptr<CharTermAttribute> termAtt;
    std::shared_ptr<PayloadAttribute> payloadAtt;

    PoolingPayloadTokenStream(std::shared_ptr<TestPayloads> outerInstance,
                              std::shared_ptr<ByteArrayPool> pool);

    bool incrementToken()  override;

    virtual ~PoolingPayloadTokenStream();

  protected:
    std::shared_ptr<PoolingPayloadTokenStream> shared_from_this()
    {
      return std::static_pointer_cast<PoolingPayloadTokenStream>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

private:
  class ByteArrayPool : public std::enable_shared_from_this<ByteArrayPool>
  {
    GET_CLASS_NAME(ByteArrayPool)
  private:
    std::deque<std::deque<char>> pool;

  public:
    ByteArrayPool(int capacity, int size);

    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual std::deque<char> get();

    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual void release(std::deque<char> &b);

    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual int size();
  };

public:
  virtual void testAcrossFields() ;

  /** some docs have payload att, some not */
  virtual void testMixupDocs() ;

  /** some field instances have payload att, some not */
  virtual void testMixupMultiValued() ;

protected:
  std::shared_ptr<TestPayloads> shared_from_this()
  {
    return std::static_pointer_cast<TestPayloads>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
