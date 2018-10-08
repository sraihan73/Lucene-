#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"

#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/index/TermState.h"

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

using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/** Implements a {@link TermsEnum} wrapping a provided
 * {@link SortedDocValues}. */

class SortedDocValuesTermsEnum : public TermsEnum
{
  GET_CLASS_NAME(SortedDocValuesTermsEnum)
private:
  const std::shared_ptr<SortedDocValues> values;
  int currentOrd = -1;
  const std::shared_ptr<BytesRefBuilder> scratch;

  /** Creates a new TermsEnum over the provided values */
public:
  SortedDocValuesTermsEnum(std::shared_ptr<SortedDocValues> values);

  SeekStatus
  seekCeil(std::shared_ptr<BytesRef> text)  override;

  bool seekExact(std::shared_ptr<BytesRef> text)  override;

  void seekExact(int64_t ord)  override;

  std::shared_ptr<BytesRef> next()  override;

  std::shared_ptr<BytesRef> term()  override;

  int64_t ord()  override;

  int docFreq() override;

  int64_t totalTermFreq() override;

  std::shared_ptr<PostingsEnum> postings(std::shared_ptr<PostingsEnum> reuse,
                                         int flags)  override;

  void seekExact(std::shared_ptr<BytesRef> term,
                 std::shared_ptr<TermState> state)  override;

  std::shared_ptr<TermState> termState()  override;

protected:
  std::shared_ptr<SortedDocValuesTermsEnum> shared_from_this()
  {
    return std::static_pointer_cast<SortedDocValuesTermsEnum>(
        TermsEnum::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
