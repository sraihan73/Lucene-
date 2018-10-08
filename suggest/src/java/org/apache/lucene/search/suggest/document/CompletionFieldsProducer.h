#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class FieldsProducer;
}

namespace org::apache::lucene::search::suggest::document
{
class CompletionsTermsReader;
}
namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::util
{
class Accountable;
}
namespace org::apache::lucene::index
{
class Terms;
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
namespace org::apache::lucene::search::suggest::document
{

using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using Terms = org::apache::lucene::index::Terms;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.CODEC_NAME;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.CompletionPostingsFormat.COMPLETION_CODEC_VERSION;
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
 * Completion index (.cmp) is opened and read at instantiation to read in {@link
 * SuggestField} numbers and their FST offsets in the Completion dictionary
 * (.lkp).
 * </p>
 * <p>
 * Completion dictionary (.lkp) is opened at instantiation and a field's FST is
 * loaded into memory the first time it is requested via {@link #terms(std::wstring)}.
 * </p>
 * <p>
 * NOTE: Only the footer is validated for Completion dictionary (.lkp) and not
 * the checksum due to random access pattern and checksum validation being too
 * costly at instantiation
 * </p>
 *
 */
class CompletionFieldsProducer final : public FieldsProducer
{
  GET_CLASS_NAME(CompletionFieldsProducer)

private:
  std::shared_ptr<FieldsProducer> delegateFieldsProducer;
  std::unordered_map<std::wstring, std::shared_ptr<CompletionsTermsReader>>
      readers;
  std::shared_ptr<IndexInput> dictIn;

  // copy ctr for merge instance
  CompletionFieldsProducer(
      std::shared_ptr<FieldsProducer> delegateFieldsProducer,
      std::unordered_map<std::wstring, std::shared_ptr<CompletionsTermsReader>>
          &readers);

public:
  CompletionFieldsProducer(std::shared_ptr<SegmentReadState> state) throw(
      IOException);

  virtual ~CompletionFieldsProducer();

  void checkIntegrity()  override;

  std::shared_ptr<FieldsProducer>
  getMergeInstance()  override;

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  std::shared_ptr<Iterator<std::wstring>> iterator() override;

  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

  int size() override;

protected:
  std::shared_ptr<CompletionFieldsProducer> shared_from_this()
  {
    return std::static_pointer_cast<CompletionFieldsProducer>(
        org.apache.lucene.codecs.FieldsProducer::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::document
