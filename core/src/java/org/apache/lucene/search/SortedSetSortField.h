#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/SortedSetSelector.h"

namespace org::apache::lucene::search
{
template <typename T>
class FieldComparator;
}
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"

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
namespace org::apache::lucene::search
{

/**
 * SortField for {@link SortedSetDocValues}.
 * <p>
 * A SortedSetDocValues contains multiple values for a field, so sorting with
 * this technique "selects" a value as the representative sort value for the
 * document. <p> By default, the minimum value in the set is selected as the
 * sort value, but this can be customized. Selectors other than the default do
 * have some limitations to ensure that all selections happen in constant-time
 * for performance. <p> Like sorting by string, this also supports sorting
 * missing values as first or last, via {@link #setMissingValue(Object)}.
 * @see SortedSetSelector
 */
class SortedSetSortField : public SortField
{
  GET_CLASS_NAME(SortedSetSortField)

private:
  const SortedSetSelector::Type selector;

  /**
   * Creates a sort, possibly in reverse, by the minimum value in the set
   * for the document.
   * @param field Name of field to sort by.  Must not be null.
   * @param reverse True if natural order should be reversed.
   */
public:
  SortedSetSortField(const std::wstring &field, bool reverse);

  /**
   * Creates a sort, possibly in reverse, specifying how the sort value from
   * the document's set is selected.
   * @param field Name of field to sort by.  Must not be null.
   * @param reverse True if natural order should be reversed.
   * @param selector custom selector type for choosing the sort value from the
   * set. <p> NOTE: selectors other than {@link SortedSetSelector.Type#MIN}
   * require optional codec support.
   */
  SortedSetSortField(const std::wstring &field, bool reverse,
                     SortedSetSelector::Type selector);

  /** Returns the selector in use for this sort */
  virtual SortedSetSelector::Type getSelector();

  virtual int hashCode();

  bool equals(std::any obj) override;

  virtual std::wstring toString();

  /**
   * Set how missing values (the empty set) are sorted.
   * <p>
   * Note that this must be {@link #STRING_FIRST} or {@link #STRING_LAST}.
   */
  void setMissingValue(std::any missingValue) override;

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: @Override public FieldComparator<?> getComparator(int
  // numHits, int sortPos)
  std::shared_ptr < FieldComparator <
      ? >> getComparator(int numHits, int sortPos) override;

private:
  class TermOrdValComparatorAnonymousInnerClass
      : public FieldComparator::TermOrdValComparator
  {
    GET_CLASS_NAME(TermOrdValComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<SortedSetSortField> outerInstance;

  public:
    TermOrdValComparatorAnonymousInnerClass(
        std::shared_ptr<SortedSetSortField> outerInstance, int numHits,
        const std::wstring &getField, bool missingValue);

  protected:
    std::shared_ptr<SortedDocValues>
    getSortedDocValues(std::shared_ptr<LeafReaderContext> context,
                       const std::wstring &field)  override;

  protected:
    std::shared_ptr<TermOrdValComparatorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TermOrdValComparatorAnonymousInnerClass>(
          FieldComparator.TermOrdValComparator::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SortedSetSortField> shared_from_this()
  {
    return std::static_pointer_cast<SortedSetSortField>(
        SortField::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
