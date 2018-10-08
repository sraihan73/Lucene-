#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <map_obj>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/index/SeedAndOrd.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/FieldAndTerm.h"
#include  "core/src/java/org/apache/lucene/index/SeedPostings.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/codecs/Codec.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"

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

using Codec = org::apache::lucene::codecs::Codec;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertEquals;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertFalse;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertNotNull;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertNull;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.junit.Assert.assertTrue;

/** Helper class extracted from BasePostingsFormatTestCase to exercise a
 * postings format. */
class RandomPostingsTester
    : public std::enable_shared_from_this<RandomPostingsTester>
{

  /** Which features to test. */
public:
  enum class Option {
    GET_CLASS_NAME(Option)
    // Sometimes use .advance():
    SKIPPING,

    // Sometimes reuse the PostingsEnum across terms:
    REUSE_ENUMS,

    // Sometimes pass non-null live docs:
    LIVE_DOCS,

    // Sometimes seek to term using previously saved TermState:
    TERM_STATE,

    // Sometimes don't fully consume docs from the enum
    PARTIAL_DOC_CONSUME,

    // Sometimes don't fully consume positions at each doc
    PARTIAL_POS_CONSUME,

    // Sometimes check payloads
    PAYLOADS,

    // Test w/ multiple threads
    THREADS
  };

private:
  int64_t totalPostings = 0;
  int64_t totalPayloadBytes = 0;

  // Holds all postings:
  std::unordered_map<std::wstring, SortedMap<std::shared_ptr<BytesRef>,
                                             std::shared_ptr<SeedAndOrd>>>
      fields;

  std::shared_ptr<FieldInfos> fieldInfos;

public:
  std::deque<std::shared_ptr<FieldAndTerm>> allTerms;

private:
  int maxDoc = 0;

public:
  const std::shared_ptr<Random> random;

  RandomPostingsTester(std::shared_ptr<Random> random) ;

  static std::shared_ptr<SeedPostings> getSeedPostings(const std::wstring &term,
                                                       int64_t seed,
                                                       IndexOptions options,
                                                       bool allowPayloads);

  /** Given the same random seed this always enumerates the
   *  same random postings */
public:
  class SeedPostings : public PostingsEnum
  {
    GET_CLASS_NAME(SeedPostings)
    // Used only to generate docIDs; this way if you pull w/
    // or w/o positions you get the same docID sequence:
  private:
    const std::shared_ptr<Random> docRandom;
    const std::shared_ptr<Random> random;

  public:
    int docFreq = 0;

  private:
    const int maxDocSpacing;
    const int payloadSize;
    const bool fixedPayloads;
    const std::shared_ptr<BytesRef> payload;
    const bool doPositions;
    const bool allowPayloads;

    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;
    // C++ NOTE: Fields cannot have the same name as methods:
    int freq_ = 0;

  public:
    int upto = 0;

  private:
    int pos = 0;
    int offset = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int startOffset_ = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int endOffset_ = 0;
    int posSpacing = 0;
    int posUpto = 0;

  public:
    SeedPostings(int64_t seed, int minDocFreq, int maxDocFreq,
                 IndexOptions options, bool allowPayloads);

    int nextDoc() override;

  private:
    int _nextDoc();

  public:
    int docID() override;

    int freq() override;

    int nextPosition() override;

    int startOffset() override;

    int endOffset() override;

    std::shared_ptr<BytesRef> getPayload() override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SeedPostings> shared_from_this()
    {
      return std::static_pointer_cast<SeedPostings>(
          PostingsEnum::shared_from_this());
    }
  };

  /** Holds one field, term and ord. */
public:
  class FieldAndTerm : public std::enable_shared_from_this<FieldAndTerm>
  {
    GET_CLASS_NAME(FieldAndTerm)
  public:
    const std::wstring field;
    const std::shared_ptr<BytesRef> term;
    const int64_t ord;

    FieldAndTerm(const std::wstring &field, std::shared_ptr<BytesRef> term,
                 int64_t ord);
  };

private:
  class SeedAndOrd : public std::enable_shared_from_this<SeedAndOrd>
  {
    GET_CLASS_NAME(SeedAndOrd)
  public:
    const int64_t seed;
    int64_t ord = 0;

    SeedAndOrd(int64_t seed);
  };

private:
  class SeedFields : public Fields
  {
    GET_CLASS_NAME(SeedFields)
  public:
    const std::unordered_map<
        std::wstring,
        SortedMap<std::shared_ptr<BytesRef>, std::shared_ptr<SeedAndOrd>>>
        fields;
    const std::shared_ptr<FieldInfos> fieldInfos;
    const IndexOptions maxAllowed;
    const bool allowPayloads;

    SeedFields(
        std::unordered_map<std::wstring, SortedMap<std::shared_ptr<BytesRef>,
                                                   std::shared_ptr<SeedAndOrd>>>
            &fields,
        std::shared_ptr<FieldInfos> fieldInfos, IndexOptions maxAllowed,
        bool allowPayloads);

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

    std::shared_ptr<Terms> terms(const std::wstring &field) override;

    int size() override;

  protected:
    std::shared_ptr<SeedFields> shared_from_this()
    {
      return std::static_pointer_cast<SeedFields>(Fields::shared_from_this());
    }
  };

private:
  class SeedTerms : public Terms
  {
    GET_CLASS_NAME(SeedTerms)
  public:
    const std::shared_ptr<
        SortedMap<std::shared_ptr<BytesRef>, std::shared_ptr<SeedAndOrd>>>
        terms;
    const std::shared_ptr<FieldInfo> fieldInfo;
    const IndexOptions maxAllowed;
    const bool allowPayloads;

    SeedTerms(std::shared_ptr<SortedMap<std::shared_ptr<BytesRef>,
                                        std::shared_ptr<SeedAndOrd>>>
                  terms,
              std::shared_ptr<FieldInfo> fieldInfo, IndexOptions maxAllowed,
              bool allowPayloads);

    std::shared_ptr<TermsEnum> iterator() override;

    int64_t size() override;

    int64_t getSumTotalTermFreq() override;

    int64_t getSumDocFreq() override;

    int getDocCount() override;

    bool hasFreqs() override;

    bool hasOffsets() override;

    bool hasPositions() override;

    bool hasPayloads() override;

  protected:
    std::shared_ptr<SeedTerms> shared_from_this()
    {
      return std::static_pointer_cast<SeedTerms>(Terms::shared_from_this());
    }
  };

private:
  class SeedTermsEnum : public TermsEnum
  {
    GET_CLASS_NAME(SeedTermsEnum)
  public:
    const std::shared_ptr<
        SortedMap<std::shared_ptr<BytesRef>, std::shared_ptr<SeedAndOrd>>>
        terms;
    const IndexOptions maxAllowed;
    const bool allowPayloads;

  private:
    std::shared_ptr<Iterator<std::unordered_map::Entry<
        std::shared_ptr<BytesRef>, std::shared_ptr<SeedAndOrd>>>>
        iterator;

    std::unordered_map::Entry<std::shared_ptr<BytesRef>,
                              std::shared_ptr<SeedAndOrd>>
        current;

  public:
    SeedTermsEnum(std::shared_ptr<SortedMap<std::shared_ptr<BytesRef>,
                                            std::shared_ptr<SeedAndOrd>>>
                      terms,
                  IndexOptions maxAllowed, bool allowPayloads);

    virtual void reset();

    SeekStatus seekCeil(std::shared_ptr<BytesRef> text) override;

    std::shared_ptr<BytesRef> next() override;

    void seekExact(int64_t ord) override;

    std::shared_ptr<BytesRef> term() override;

    int64_t ord() override;

    int docFreq() override;

    int64_t totalTermFreq() override;

    std::shared_ptr<PostingsEnum>
    postings(std::shared_ptr<PostingsEnum> reuse,
             int flags)  override final;

  protected:
    std::shared_ptr<SeedTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<SeedTermsEnum>(
          TermsEnum::shared_from_this());
    }
  };

private:
  class ThreadState : public std::enable_shared_from_this<ThreadState>
  {
    GET_CLASS_NAME(ThreadState)
    // Only used with REUSE option:
  public:
    std::shared_ptr<PostingsEnum> reusePostingsEnum;
  };

private:
  std::shared_ptr<FieldInfos> currentFieldInfos;

  // maxAllowed = the "highest" we can index, but we will still
  // randomly index at lower IndexOption
public:
  virtual std::shared_ptr<FieldsProducer>
  buildIndex(std::shared_ptr<Codec> codec, std::shared_ptr<Directory> dir,
             IndexOptions maxAllowed, bool allowPayloads,
             bool alwaysTestMax) ;

private:
  void verifyEnum(std::shared_ptr<Random> random,
                  std::shared_ptr<ThreadState> threadState,
                  const std::wstring &field, std::shared_ptr<BytesRef> term,
                  std::shared_ptr<TermsEnum> termsEnum,
                  IndexOptions maxTestOptions, IndexOptions maxIndexOptions,
                  std::shared_ptr<EnumSet<Option>> options,
                  bool alwaysTestMax) ;

private:
  class TestThread : public Thread
  {
    GET_CLASS_NAME(TestThread)
  private:
    std::shared_ptr<Fields> fieldsSource;
    std::shared_ptr<EnumSet<Option>> options;
    IndexOptions maxIndexOptions = static_cast<IndexOptions>(0);
    IndexOptions maxTestOptions = static_cast<IndexOptions>(0);
    bool alwaysTestMax = false;
    std::shared_ptr<RandomPostingsTester> postingsTester;
    std::shared_ptr<Random> random;

  public:
    TestThread(std::shared_ptr<Random> random,
               std::shared_ptr<RandomPostingsTester> postingsTester,
               std::shared_ptr<Fields> fieldsSource,
               std::shared_ptr<EnumSet<Option>> options,
               IndexOptions maxTestOptions, IndexOptions maxIndexOptions,
               bool alwaysTestMax);

    void run() override;

  protected:
    std::shared_ptr<TestThread> shared_from_this()
    {
      return std::static_pointer_cast<TestThread>(Thread::shared_from_this());
    }
  };

public:
  virtual void testTerms(std::shared_ptr<Fields> fieldsSource,
                         std::shared_ptr<EnumSet<Option>> options,
                         IndexOptions const maxTestOptions,
                         IndexOptions const maxIndexOptions,
                         bool const alwaysTestMax) ;

private:
  void testTermsOneThread(std::shared_ptr<Random> random,
                          std::shared_ptr<Fields> fieldsSource,
                          std::shared_ptr<EnumSet<Option>> options,
                          IndexOptions maxTestOptions,
                          IndexOptions maxIndexOptions,
                          bool alwaysTestMax) ;

public:
  virtual void
  testFields(std::shared_ptr<Fields> fields) ;

  /** Indexes all fields/terms at the specified
   *  IndexOptions, and fully tests at that IndexOptions. */
  virtual void testFull(std::shared_ptr<Codec> codec,
                        std::shared_ptr<Path> path, IndexOptions options,
                        bool withPayloads) ;
};

} // #include  "core/src/java/org/apache/lucene/index/
