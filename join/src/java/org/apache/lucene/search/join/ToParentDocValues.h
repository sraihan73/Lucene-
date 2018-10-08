#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SortedDocValues;
}

namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::util
{
class BitSet;
}
namespace org::apache::lucene::search::join
{
class BlockJoinSelector;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
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
namespace org::apache::lucene::search::join
{

using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Type = org::apache::lucene::search::join::BlockJoinSelector::Type;
using BitSet = org::apache::lucene::util::BitSet;
using BytesRef = org::apache::lucene::util::BytesRef;

class ToParentDocValues : public DocIdSetIterator
{
  GET_CLASS_NAME(ToParentDocValues)

public:
  class Accumulator
  {
    GET_CLASS_NAME(Accumulator)
  public:
    virtual void reset() = 0;
    virtual void increment() = 0;
  };

private:
  class SortedDVs final : public SortedDocValues, public Accumulator
  {
    GET_CLASS_NAME(SortedDVs)
  private:
    const std::shared_ptr<SortedDocValues> values;
    const BlockJoinSelector::Type selection;
    int ord = -1;
    const std::shared_ptr<ToParentDocValues> iter;

    SortedDVs(std::shared_ptr<SortedDocValues> values,
              BlockJoinSelector::Type selection,
              std::shared_ptr<BitSet> parents,
              std::shared_ptr<DocIdSetIterator> children);

  public:
    int docID() override;

    void reset()  override;

    void increment()  override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int targetParentDocID)  override;

    int ordValue() override;

    std::shared_ptr<BytesRef> lookupOrd(int ord)  override;

    int getValueCount() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SortedDVs> shared_from_this()
    {
      return std::static_pointer_cast<SortedDVs>(
          org.apache.lucene.index.SortedDocValues::shared_from_this());
    }
  };

private:
  class NumDV final : public NumericDocValues, public Accumulator
  {
    GET_CLASS_NAME(NumDV)
  private:
    const std::shared_ptr<NumericDocValues> values;
    int64_t value = 0;
    const BlockJoinSelector::Type selection;

    const std::shared_ptr<ToParentDocValues> iter;

    NumDV(std::shared_ptr<NumericDocValues> values,
          BlockJoinSelector::Type selection, std::shared_ptr<BitSet> parents,
          std::shared_ptr<DocIdSetIterator> children);

  public:
    void reset()  override;

    void increment()  override;

    int nextDoc()  override;

    int advance(int targetParentDocID)  override;

    bool advanceExact(int targetParentDocID)  override;

    int64_t longValue() override;

    int docID() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<NumDV> shared_from_this()
    {
      return std::static_pointer_cast<NumDV>(
          org.apache.lucene.index.NumericDocValues::shared_from_this());
    }
  };

private:
  ToParentDocValues(std::shared_ptr<DocIdSetIterator> values,
                    std::shared_ptr<BitSet> parents,
                    std::shared_ptr<DocIdSetIterator> children,
                    std::shared_ptr<Accumulator> collect);

  const std::shared_ptr<BitSet> parents;
  // C++ NOTE: Fields cannot have the same name as methods:
  int docID_ = -1;
  const std::shared_ptr<Accumulator> collector;

public:
  bool seen = false;

private:
  std::shared_ptr<DocIdSetIterator> childWithValues;

public:
  int docID() override;

  int nextDoc()  override;

  int advance(int target)  override;

  //@Override
  virtual bool advanceExact(int targetParentDocID) ;

  int64_t cost() override;

  static std::shared_ptr<NumericDocValues>
  wrap(std::shared_ptr<NumericDocValues> values, Type selection,
       std::shared_ptr<BitSet> parents2,
       std::shared_ptr<DocIdSetIterator> children);

  static std::shared_ptr<SortedDocValues>
  wrap(std::shared_ptr<SortedDocValues> values, Type selection,
       std::shared_ptr<BitSet> parents2,
       std::shared_ptr<DocIdSetIterator> children);

protected:
  std::shared_ptr<ToParentDocValues> shared_from_this()
  {
    return std::static_pointer_cast<ToParentDocValues>(
        org.apache.lucene.search.DocIdSetIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::join
