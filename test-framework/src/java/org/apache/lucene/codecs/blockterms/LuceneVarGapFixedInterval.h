#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class FieldsConsumer;
}

namespace org::apache::lucene::index
{
class SegmentWriteState;
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
namespace org::apache::lucene::codecs::blockterms
{

using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

// TODO: we could make separate base class that can wrap
// any PostingsFormat and make it ord-able...

/**
 * Customized version of {@link Lucene50PostingsFormat} that uses
 * {@link VariableGapTermsIndexWriter} with a fixed interval.
 */
class LuceneVarGapFixedInterval final : public PostingsFormat
{
  GET_CLASS_NAME(LuceneVarGapFixedInterval)
public:
  const int termIndexInterval;

  LuceneVarGapFixedInterval();

  LuceneVarGapFixedInterval(int termIndexInterval);

  std::shared_ptr<FieldsConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<FieldsProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

  /** Extension of freq postings file */
  static const std::wstring FREQ_EXTENSION;

  /** Extension of prox postings file */
  static const std::wstring PROX_EXTENSION;

protected:
  std::shared_ptr<LuceneVarGapFixedInterval> shared_from_this()
  {
    return std::static_pointer_cast<LuceneVarGapFixedInterval>(
        org.apache.lucene.codecs.PostingsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blockterms
