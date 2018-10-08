#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::ramonly
{
class RAMField;
}

namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::util
{
class Accountable;
}
namespace org::apache::lucene::codecs::ramonly
{
class RAMTerm;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::codecs::ramonly
{
class RAMDoc;
}
namespace org::apache::lucene::codecs::ramonly
{
class RAMTermsConsumer;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::index
{
class Fields;
}
namespace org::apache::lucene::codecs::ramonly
{
class RAMPostingsWriterImpl;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::codecs
{
class TermStats;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::codecs
{
class FieldsConsumer;
}
namespace org::apache::lucene::codecs
{
class FieldsProducer;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
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
namespace org::apache::lucene::codecs::ramonly
{

using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using TermStats = org::apache::lucene::codecs::TermStats;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using Fields = org::apache::lucene::index::Fields;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;

/** Stores all postings data in RAM, but writes a small
 *  token (header + single int) to identify which "slot" the
 *  index is using in RAM HashMap.
 *
 *  NOTE: this codec sorts terms by reverse-unicode-order! */

class RAMOnlyPostingsFormat final : public PostingsFormat
{
  GET_CLASS_NAME(RAMOnlyPostingsFormat)

public:
  RAMOnlyPostingsFormat();

  // Postings state:
public:
  class RAMPostings : public FieldsProducer
  {
    GET_CLASS_NAME(RAMPostings)
  public:
    const std::unordered_map<std::wstring, std::shared_ptr<RAMField>>
        fieldToTerms = std::map_obj<std::wstring, std::shared_ptr<RAMField>>();

    std::shared_ptr<Terms> terms(const std::wstring &field) override;

    int size() override;

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

    virtual ~RAMPostings();

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    void checkIntegrity()  override;

  protected:
    std::shared_ptr<RAMPostings> shared_from_this()
    {
      return std::static_pointer_cast<RAMPostings>(
          org.apache.lucene.codecs.FieldsProducer::shared_from_this());
    }
  };

public:
  class RAMField : public Terms, public Accountable
  {
    GET_CLASS_NAME(RAMField)
  public:
    const std::wstring field;
    const std::shared_ptr<SortedMap<std::wstring, std::shared_ptr<RAMTerm>>>
        termToDocs = std::map_obj<std::wstring, std::shared_ptr<RAMTerm>>();
    int64_t sumTotalTermFreq = 0;
    int64_t sumDocFreq = 0;
    int docCount = 0;
    const std::shared_ptr<FieldInfo> info;

    RAMField(const std::wstring &field, std::shared_ptr<FieldInfo> info);

    int64_t ramBytesUsed() override;

    int64_t size() override;

    int64_t getSumTotalTermFreq() override;

    int64_t getSumDocFreq()  override;

    int getDocCount()  override;

    std::shared_ptr<TermsEnum> iterator() override;

    bool hasFreqs() override;

    bool hasOffsets() override;

    bool hasPositions() override;

    bool hasPayloads() override;

  protected:
    std::shared_ptr<RAMField> shared_from_this()
    {
      return std::static_pointer_cast<RAMField>(
          org.apache.lucene.index.Terms::shared_from_this());
    }
  };

public:
  class RAMTerm : public std::enable_shared_from_this<RAMTerm>,
                  public Accountable
  {
    GET_CLASS_NAME(RAMTerm)
  public:
    const std::wstring term;
    int64_t totalTermFreq = 0;
    const std::deque<std::shared_ptr<RAMDoc>> docs =
        std::deque<std::shared_ptr<RAMDoc>>();
    RAMTerm(const std::wstring &term);

    int64_t ramBytesUsed() override;
  };

public:
  class RAMDoc : public std::enable_shared_from_this<RAMDoc>, public Accountable
  {
    GET_CLASS_NAME(RAMDoc)
  public:
    const int docID;
    std::deque<int> const positions;
    std::deque<std::deque<char>> payloads;

    RAMDoc(int docID, int freq);

    int64_t ramBytesUsed() override;
  };

  // Classes for writing to the postings state
private:
  class RAMFieldsConsumer : public FieldsConsumer
  {
    GET_CLASS_NAME(RAMFieldsConsumer)

  private:
    const std::shared_ptr<RAMPostings> postings;
    const std::shared_ptr<RAMTermsConsumer> termsConsumer =
        std::make_shared<RAMTermsConsumer>();
    const std::shared_ptr<SegmentWriteState> state;

  public:
    RAMFieldsConsumer(std::shared_ptr<SegmentWriteState> writeState,
                      std::shared_ptr<RAMPostings> postings);

    void write(std::shared_ptr<Fields> fields)  override;

    virtual ~RAMFieldsConsumer();

  protected:
    std::shared_ptr<RAMFieldsConsumer> shared_from_this()
    {
      return std::static_pointer_cast<RAMFieldsConsumer>(
          org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
    }
  };

private:
  class RAMTermsConsumer : public std::enable_shared_from_this<RAMTermsConsumer>
  {
    GET_CLASS_NAME(RAMTermsConsumer)
  private:
    std::shared_ptr<RAMField> field;
    const std::shared_ptr<RAMPostingsWriterImpl> postingsWriter =
        std::make_shared<RAMPostingsWriterImpl>();

  public:
    std::shared_ptr<RAMTerm> current;

    virtual void reset(std::shared_ptr<RAMField> field);

    virtual std::shared_ptr<RAMPostingsWriterImpl>
    startTerm(std::shared_ptr<BytesRef> text);

    virtual void finishTerm(std::shared_ptr<BytesRef> text,
                            std::shared_ptr<TermStats> stats);

    virtual void finish(int64_t sumTotalTermFreq, int64_t sumDocFreq,
                        int docCount);
  };

public:
  class RAMPostingsWriterImpl
      : public std::enable_shared_from_this<RAMPostingsWriterImpl>
  {
    GET_CLASS_NAME(RAMPostingsWriterImpl)
  private:
    std::shared_ptr<RAMTerm> term;
    std::shared_ptr<RAMDoc> current;
    int posUpto = 0;

  public:
    virtual void reset(std::shared_ptr<RAMTerm> term);

    virtual void startDoc(int docID, int freq);

    virtual void addPosition(int position, std::shared_ptr<BytesRef> payload,
                             int startOffset, int endOffset);

    virtual void finishDoc();
  };

public:
  class RAMTermsEnum : public TermsEnum
  {
    GET_CLASS_NAME(RAMTermsEnum)
  public:
    std::shared_ptr<Iterator<std::wstring>> it;
    std::wstring current;

  private:
    const std::shared_ptr<RAMField> ramField;

  public:
    RAMTermsEnum(std::shared_ptr<RAMField> field);

    std::shared_ptr<BytesRef> next() override;

    SeekStatus seekCeil(std::shared_ptr<BytesRef> term) override;

    void seekExact(int64_t ord) override;

    int64_t ord() override;

    std::shared_ptr<BytesRef> term() override;

    int docFreq() override;

    int64_t totalTermFreq() override;

    std::shared_ptr<PostingsEnum> postings(std::shared_ptr<PostingsEnum> reuse,
                                           int flags) override;

  protected:
    std::shared_ptr<RAMTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<RAMTermsEnum>(
          org.apache.lucene.index.TermsEnum::shared_from_this());
    }
  };

private:
  class RAMDocsEnum : public PostingsEnum
  {
    GET_CLASS_NAME(RAMDocsEnum)
  private:
    const std::shared_ptr<RAMTerm> ramTerm;
    std::shared_ptr<RAMDoc> current;

  public:
    int upto = -1;
    int posUpto = 0;

    RAMDocsEnum(std::shared_ptr<RAMTerm> ramTerm);

    int advance(int targetDocID)  override;

    // TODO: override bulk read, for better perf
    int nextDoc() override;

    int freq()  override;

    int docID() override;

    int nextPosition() override;

    int startOffset() override;

    int endOffset() override;

    std::shared_ptr<BytesRef> getPayload() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<RAMDocsEnum> shared_from_this()
    {
      return std::static_pointer_cast<RAMDocsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

  // Holds all indexes created, keyed by the ID assigned in fieldsConsumer
private:
  const std::unordered_map<int, std::shared_ptr<RAMPostings>> state =
      std::unordered_map<int, std::shared_ptr<RAMPostings>>();

  const std::shared_ptr<AtomicInteger> nextID =
      std::make_shared<AtomicInteger>();

  const std::wstring RAM_ONLY_NAME = L"RAMOnly";
  static constexpr int VERSION_START = 0;
  static constexpr int VERSION_LATEST = VERSION_START;

  static const std::wstring ID_EXTENSION;

public:
  std::shared_ptr<FieldsConsumer>
  fieldsConsumer(std::shared_ptr<SegmentWriteState> writeState) throw(
      IOException) override;

  std::shared_ptr<FieldsProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> readState)  override;

protected:
  std::shared_ptr<RAMOnlyPostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<RAMOnlyPostingsFormat>(
        org.apache.lucene.codecs.PostingsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::ramonly
