#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SortedDocValues;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class TermsEnum;
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
namespace org::apache::lucene::index
{

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Exposes multi-valued iterator view over a single-valued iterator.
 * <p>
 * This can be used if you want to have one multi-valued implementation
 * that works for single or multi-valued types.
 */
class SingletonSortedSetDocValues final : public SortedSetDocValues
{
  GET_CLASS_NAME(SingletonSortedSetDocValues)
private:
  const std::shared_ptr<SortedDocValues> in_;
  int64_t ord = 0;

  /** Creates a multi-valued view over the provided SortedDocValues */
public:
  SingletonSortedSetDocValues(std::shared_ptr<SortedDocValues> in_);

  /** Return the wrapped {@link SortedDocValues} */
  std::shared_ptr<SortedDocValues> getSortedDocValues();

  int docID() override;

  int64_t nextOrd() override;

  int nextDoc()  override;

  int advance(int target)  override;

  bool advanceExact(int target)  override;

  std::shared_ptr<BytesRef>
  lookupOrd(int64_t ord)  override;

  int64_t getValueCount() override;

  int64_t
  lookupTerm(std::shared_ptr<BytesRef> key)  override;

  std::shared_ptr<TermsEnum> termsEnum()  override;

  int64_t cost() override;

protected:
  std::shared_ptr<SingletonSortedSetDocValues> shared_from_this()
  {
    return std::static_pointer_cast<SingletonSortedSetDocValues>(
        SortedSetDocValues::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
