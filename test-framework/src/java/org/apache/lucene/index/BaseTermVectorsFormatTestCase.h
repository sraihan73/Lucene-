#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/FieldType.h"

#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/util/AttributeImpl.h"
#include  "core/src/java/org/apache/lucene/util/AttributeReflector.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"

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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.PostingsEnum.ALL;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.PostingsEnum.FREQS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.PostingsEnum.NONE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.PostingsEnum.OFFSETS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.PostingsEnum.PAYLOADS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.PostingsEnum.POSITIONS;

using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Base class aiming at testing {@link TermVectorsFormat term vectors formats}.
 * To test a new format, all you need is to register a new {@link Codec} which
GET_CLASS_NAME(aiming)
 * uses it and extend this class and override {@link #getCodec()}.
 * @lucene.experimental
 */
class BaseTermVectorsFormatTestCase : public BaseIndexFileFormatTestCase
{
  GET_CLASS_NAME(BaseTermVectorsFormatTestCase)

  /**
   * A combination of term vectors options.
   */
protected:
  class Options final
  {
    GET_CLASS_NAME(Options)
  public:
    static Options NONE;
    static Options POSITIONS;
    static Options OFFSETS;
    static Options POSITIONS_AND_OFFSETS;
    static Options POSITIONS_AND_PAYLOADS;
    static Options POSITIONS_AND_OFFSETS_AND_PAYLOADS;

  private:
    static std::deque<Options> valueList;

    class StaticConstructor
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

    static StaticConstructor staticConstructor;

  public:
    enum class InnerEnum {
      GET_CLASS_NAME(InnerEnum) NONE,
      POSITIONS,
      OFFSETS,
      POSITIONS_AND_OFFSETS,
      POSITIONS_AND_PAYLOADS,
      POSITIONS_AND_OFFSETS_AND_PAYLOADS
    };

    const InnerEnum innerEnumValue;

  private:
    const std::wstring nameValue;
    const int ordinalValue;
    static int nextOrdinal;

  public:
    const bool positions, offsets, payloads;

  private:
    Options(const std::wstring &name, InnerEnum innerEnum,
            std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance,
            bool positions, bool offsets, bool payloads);

  public:
    bool operator==(const Options &other);

    bool operator!=(const Options &other);

    static std::deque<Options> values();

    int ordinal();

    std::wstring toString();

    static Options valueOf(const std::wstring &name);
  };

protected:
  virtual std::shared_ptr<Set<Options>> validOptions();

  virtual Options randomOptions();

  virtual std::shared_ptr<FieldType> fieldType(Options options);

  void addRandomFields(std::shared_ptr<Document> doc) override;

  // custom impl to test cases that are forbidden by the default OffsetAttribute
  // impl
private:
  class PermissiveOffsetAttributeImpl : public AttributeImpl,
                                        public OffsetAttribute
  {
    GET_CLASS_NAME(PermissiveOffsetAttributeImpl)

  public:
    int start = 0, end = 0;

    int startOffset() override;

    int endOffset() override;

    void setOffset(int startOffset, int endOffset) override;

    void clear() override;

    bool equals(std::any other) override;

    virtual int hashCode();

    void copyTo(std::shared_ptr<AttributeImpl> target) override;

    void reflectWith(AttributeReflector reflector) override;

  protected:
    std::shared_ptr<PermissiveOffsetAttributeImpl> shared_from_this()
    {
      return std::static_pointer_cast<PermissiveOffsetAttributeImpl>(
          org.apache.lucene.util.AttributeImpl::shared_from_this());
    }
  };

  // TODO: use CannedTokenStream?
  // TODO: pull out and make top-level-utility, separate from TermVectors
  /** Produces a random TokenStream based off of provided terms. */
public:
  class RandomTokenStream : public TokenStream
  {
    GET_CLASS_NAME(RandomTokenStream)

  public:
    std::deque<std::wstring> const terms;
    std::deque<std::shared_ptr<BytesRef>> const termBytes;
    std::deque<int> const positionsIncrements;
    std::deque<int> const positions;
    std::deque<int> const startOffsets, endOffsets;
    std::deque<std::shared_ptr<BytesRef>> const payloads;

    const std::unordered_map<std::wstring, int> freqs;
    const std::unordered_map<int, Set<int>> positionToTerms;
    const std::unordered_map<int, Set<int>> startOffsetToTerms;

    const std::shared_ptr<CharTermAttribute> termAtt;
    const std::shared_ptr<PositionIncrementAttribute> piAtt;
    const std::shared_ptr<OffsetAttribute> oAtt;
    const std::shared_ptr<PayloadAttribute> pAtt;
    int i = 0;

    RandomTokenStream(int len, std::deque<std::wstring> &sampleTerms,
                      std::deque<std::shared_ptr<BytesRef>> &sampleTermBytes);

  protected:
    virtual std::shared_ptr<BytesRef> randomPayload();

  public:
    virtual bool hasPayloads();

    virtual std::deque<std::wstring> getTerms();

    virtual std::deque<std::shared_ptr<BytesRef>> getTermBytes();

    virtual std::deque<int> getPositionsIncrements();

    virtual std::deque<int> getStartOffsets();

    virtual std::deque<int> getEndOffsets();

    virtual std::deque<std::shared_ptr<BytesRef>> getPayloads();

    void reset()  override;

    bool incrementToken()  override final;

  protected:
    std::shared_ptr<RandomTokenStream> shared_from_this()
    {
      return std::static_pointer_cast<RandomTokenStream>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

protected:
  class RandomDocument : public std::enable_shared_from_this<RandomDocument>
  {
    GET_CLASS_NAME(RandomDocument)
  private:
    std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance;

    std::deque<std::wstring> const fieldNames;
    std::deque<std::shared_ptr<FieldType>> const fieldTypes;
    std::deque<std::shared_ptr<RandomTokenStream>> const tokenStreams;

  protected:
    RandomDocument(std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance,
                   int fieldCount, int maxTermCount, Options options,
                   std::deque<std::wstring> &fieldNames,
                   std::deque<std::wstring> &sampleTerms,
                   std::deque<std::shared_ptr<BytesRef>> &sampleTermBytes);

  public:
    virtual std::shared_ptr<Document> toDocument();
  };

protected:
  class RandomDocumentFactory
      : public std::enable_shared_from_this<RandomDocumentFactory>
  {
    GET_CLASS_NAME(RandomDocumentFactory)
  private:
    std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance;

    std::deque<std::wstring> const fieldNames;
    std::deque<std::wstring> const terms;
    std::deque<std::shared_ptr<BytesRef>> const termBytes;

  protected:
    RandomDocumentFactory(
        std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance,
        int distinctFieldNames, int disctinctTerms);

  public:
    virtual std::shared_ptr<RandomDocument>
    newDocument(int fieldCount, int maxTermCount, Options options);
  };

protected:
  virtual void assertEquals(std::shared_ptr<RandomDocument> doc,
                            std::shared_ptr<Fields> fields) ;

  static bool equals(std::any o1, std::any o2);

  // to test reuse
private:
  const std::shared_ptr<ThreadLocal<std::shared_ptr<PostingsEnum>>> docsEnum =
      std::make_shared<ThreadLocal<std::shared_ptr<PostingsEnum>>>();
  const std::shared_ptr<ThreadLocal<std::shared_ptr<PostingsEnum>>>
      docsAndPositionsEnum =
          std::make_shared<ThreadLocal<std::shared_ptr<PostingsEnum>>>();

protected:
  virtual void assertEquals(std::shared_ptr<RandomTokenStream> tk,
                            std::shared_ptr<FieldType> ft,
                            std::shared_ptr<Terms> terms) ;

  virtual std::shared_ptr<Document> addId(std::shared_ptr<Document> doc,
                                          const std::wstring &id);

  virtual int docID(std::shared_ptr<IndexReader> reader,
                    const std::wstring &id) ;

  // only one doc with vectors
public:
  virtual void testRareVectors() ;

  virtual void testHighFreqs() ;

  virtual void testLotsOfFields() ;

  // different options for the same field
  virtual void testMixedOptions() ;

  virtual void testRandom() ;

  virtual void testMerge() ;

  // run random tests from different threads to make sure the per-thread clones
  // don't share mutable data
  virtual void testClone() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance;

    int numDocs = 0;
    std::deque<
        std::shared_ptr<org::apache::lucene::index::
                            BaseTermVectorsFormatTestCase::RandomDocument>>
        docs;
    std::shared_ptr<org::apache::lucene::index::IndexReader> reader;
    std::shared_ptr<AtomicReference<std::runtime_error>> exception;
    int i = 0;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance,
        int numDocs,
        std::deque<std::shared_ptr<
            org::apache::lucene::index::BaseTermVectorsFormatTestCase::
                RandomDocument>> &docs,
        std::shared_ptr<org::apache::lucene::index::IndexReader> reader,
        std::shared_ptr<AtomicReference<std::runtime_error>> exception, int i);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testPostingsEnumFreqs() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testPostingsEnumPositions() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testPostingsEnumOffsets() ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void
  testPostingsEnumOffsetsWithoutPositions() ;

private:
  class AnalyzerAnonymousInnerClass4 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass4)
  private:
    std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance;

  public:
    AnalyzerAnonymousInnerClass4(
        std::shared_ptr<BaseTermVectorsFormatTestCase> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass4>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testPostingsEnumPayloads() ;

  virtual void testPostingsEnumAll() ;

protected:
  std::shared_ptr<BaseTermVectorsFormatTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseTermVectorsFormatTestCase>(
        BaseIndexFileFormatTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
