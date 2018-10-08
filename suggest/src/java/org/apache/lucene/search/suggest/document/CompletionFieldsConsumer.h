#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/suggest/document/CompletionMetaData.h"

#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldsConsumer.h"
#include  "core/src/java/org/apache/lucene/codecs/PostingsFormat.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/search/suggest/document/NRTSuggesterBuilder.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
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
namespace org::apache::lucene::search::suggest::document
{

using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using Fields = org::apache::lucene::index::Fields;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.CODEC_NAME;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.COMPLETION_VERSION_CURRENT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.DICT_EXTENSION;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.INDEX_EXTENSION;

/**
 * <p>
 * Weighted FSTs for any indexed {@link SuggestField} is built on {@link
 * #write(Fields)}. A weighted FST maps the analyzed forms of a field to its
 * surface form and document id. FSTs are stored in the CompletionDictionary
 * (.lkp).
 * </p>
 * <p>
 * The file offsets of a field's FST are stored in the CompletionIndex (.cmp)
 * along with the field's internal number {@link FieldInfo#number} on {@link
 * #close()}.
 * </p>
 *
 */
class CompletionFieldsConsumer final : public FieldsConsumer
{
  GET_CLASS_NAME(CompletionFieldsConsumer)

private:
  const std::wstring delegatePostingsFormatName;
  const std::unordered_map<std::wstring, std::shared_ptr<CompletionMetaData>>
      seenFields = std::unordered_map<std::wstring,
                                      std::shared_ptr<CompletionMetaData>>();
  const std::shared_ptr<SegmentWriteState> state;
  std::shared_ptr<IndexOutput> dictOut;
  std::shared_ptr<FieldsConsumer> delegateFieldsConsumer;

public:
  CompletionFieldsConsumer(
      std::shared_ptr<PostingsFormat> delegatePostingsFormat,
      std::shared_ptr<SegmentWriteState> state) ;

  void write(std::shared_ptr<Fields> fields)  override;

private:
  bool closed = false;

public:
  virtual ~CompletionFieldsConsumer();

private:
  class CompletionMetaData
      : public std::enable_shared_from_this<CompletionMetaData>
  {
    GET_CLASS_NAME(CompletionMetaData)
  private:
    const int64_t filePointer;
    const int64_t minWeight;
    const int64_t maxWeight;
    const char type;

    CompletionMetaData(int64_t filePointer, int64_t minWeight,
                       int64_t maxWeight, char type);
  };

  // builds an FST based on the terms written
private:
  class CompletionTermWriter
      : public std::enable_shared_from_this<CompletionTermWriter>
  {
    GET_CLASS_NAME(CompletionTermWriter)

  private:
    std::shared_ptr<PostingsEnum> postingsEnum = nullptr;
    int docCount = 0;
    int64_t maxWeight = 0;
    int64_t minWeight = std::numeric_limits<int64_t>::max();
    char type = 0;
    bool first = false;

    const std::shared_ptr<BytesRefBuilder> scratch =
        std::make_shared<BytesRefBuilder>();
    const std::shared_ptr<NRTSuggesterBuilder> builder;

  public:
    CompletionTermWriter();

    /**
     * Stores the built FST in <code>output</code>
     * Returns true if there was anything stored, false otherwise
     */
    virtual bool finish(std::shared_ptr<IndexOutput> output) ;

    /**
     * Writes all postings (surface form, weight, document id) for
     * <code>term</code>
     */
    virtual void write(std::shared_ptr<BytesRef> term,
                       std::shared_ptr<TermsEnum> termsEnum) ;
  };

protected:
  std::shared_ptr<CompletionFieldsConsumer> shared_from_this()
  {
    return std::static_pointer_cast<CompletionFieldsConsumer>(
        org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/document/
