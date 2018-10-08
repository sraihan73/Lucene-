#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <deque>
#include <queue>

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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Expert: A hit queue for sorting by hits by terms in more than one field.
 *
 * @lucene.experimental
 * @since 2.9
 * @see IndexSearcher#search(Query,int,Sort)
 */
template <typename T>
class FieldValueHitQueue : public PriorityQueue<T>
{
  GET_CLASS_NAME(FieldValueHitQueue)
  static_assert(std::is_base_of<FieldValueHitQueue.Entry, T>::value,
                L"T must inherit from FieldValueHitQueue.Entry");

  /**
   * Extension of ScoreDoc to also store the
   * {@link FieldComparator} slot.
   */
public:
  class Entry : public ScoreDoc
  {
    GET_CLASS_NAME(Entry)
  public:
    int slot = 0;

    Entry(int slot, int doc, float score);

    virtual std::wstring toString();

  protected:
    std::shared_ptr<Entry> shared_from_this()
    {
      return std::static_pointer_cast<Entry>(ScoreDoc::shared_from_this());
    }
  };

  /**
   * An implementation of {@link FieldValueHitQueue} which is optimized in case
   * there is just one comparator.
   */
private:
  template <typename T>
  class OneComparatorFieldValueHitQueue final : public FieldValueHitQueue<T>
  {
    GET_CLASS_NAME(OneComparatorFieldValueHitQueue)
    static_assert(std::is_base_of<FieldValueHitQueue.Entry, T>::value,
                  L"T must inherit from FieldValueHitQueue.Entry");

  private:
    const int oneReverseMul;
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: private final FieldComparator<?> oneComparator;
    const std::shared_ptr < FieldComparator < ? >> oneComparator;

  public:
    OneComparatorFieldValueHitQueue(
        std::deque<std::shared_ptr<SortField>> &fields, int size)
        : FieldValueHitQueue<T>(fields, size),
          oneReverseMul(outerInstance->reverseMul[0]),
          oneComparator(outerInstance->comparators[0])
    {

      assert(fields.size() == 1);
    }

    /**
     * Returns whether <code>hitA</code> is less relevant than
     * <code>hitB</code>.
     * @param hitA Entry
     * @param hitB Entry
     * @return <code>true</code> if document <code>hitA</code> should be sorted
     * after document <code>hitB</code>.
     */
  protected:
    bool lessThan(std::shared_ptr<Entry> hitA,
                  std::shared_ptr<Entry> hitB) override
    {

      assert(hitA != hitB);
      assert(hitA->slot != hitB->slot);

      constexpr int c =
          oneReverseMul * oneComparator->compare(hitA->slot, hitB->slot);
      if (c != 0) {
        return c > 0;
      }

      // avoid random sort order that could lead to duplicates (bug #31241):
      return hitA->doc > hitB->doc;
    }

  protected:
    std::shared_ptr<OneComparatorFieldValueHitQueue> shared_from_this()
    {
      return std::static_pointer_cast<OneComparatorFieldValueHitQueue>(
          FieldValueHitQueue<T>::shared_from_this());
    }
  };

  /**
   * An implementation of {@link FieldValueHitQueue} which is optimized in case
   * there is more than one comparator.
   */
private:
  template <typename T>
  class MultiComparatorsFieldValueHitQueue final : public FieldValueHitQueue<T>
  {
    GET_CLASS_NAME(MultiComparatorsFieldValueHitQueue)
    static_assert(std::is_base_of<FieldValueHitQueue.Entry, T>::value,
                  L"T must inherit from FieldValueHitQueue.Entry");

  public:
    MultiComparatorsFieldValueHitQueue(
        std::deque<std::shared_ptr<SortField>> &fields, int size)
        : FieldValueHitQueue<T>(fields, size)
    {
    }

  protected:
    bool lessThan(std::shared_ptr<Entry> hitA,
                  std::shared_ptr<Entry> hitB) override
    {

      assert(hitA != hitB);
      assert(hitA->slot != hitB->slot);

      int numComparators = outerInstance->comparators->size();
      for (int i = 0; i < numComparators; ++i) {
        constexpr int c =
            outerInstance->reverseMul[i] *
            outerInstance->comparators[i].compare(hitA->slot, hitB->slot);
        if (c != 0) {
          // Short circuit
          return c > 0;
        }
      }

      // avoid random sort order that could lead to duplicates (bug #31241):
      return hitA->doc > hitB->doc;
    }

  protected:
    std::shared_ptr<MultiComparatorsFieldValueHitQueue> shared_from_this()
    {
      return std::static_pointer_cast<MultiComparatorsFieldValueHitQueue>(
          FieldValueHitQueue<T>::shared_from_this());
    }
  };

  // prevent instantiation and extension.
private:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private FieldValueHitQueue(SortField[] fields, int size)
      FieldValueHitQueue(std::deque<std::shared_ptr<SortField>> &fields, int size) : PriorityQueue<T>(size), fields(fields), comparators(std::deque<FieldComparator<?>>(numComparators)), reverseMul(std::deque<int>(numComparators))
      {
        // When we get here, fields.length is guaranteed to be > 0, therefore no
        // need to check it again.

        // All these are required by this class's API - need to return arrays.
        // Therefore even in the case of a single comparator, create an array
        // anyway.
        int numComparators = fields.size();
        for (int i = 0; i < numComparators; ++i) {
          std::shared_ptr<SortField> field = fields[i];

          reverseMul[i] = field->reverse ? -1 : 1;
          comparators[i] = field->getComparator(size, i);
        }
      }

      /**
       * Creates a hit queue sorted by the given deque of fields.
       *
       * <p><b>NOTE</b>: The instances returned by this method
       * pre-allocate a full array of length <code>numHits</code>.
       *
       * @param fields
       *          SortField array we are sorting by in priority order (highest
       *          priority first); cannot be <code>null</code> or empty
       * @param size
       *          The number of hits to retain. Must be greater than zero.
       */
    public:
      template <typename T>
      static std::shared_ptr<FieldValueHitQueue<T>>
      create(std::deque<std::shared_ptr<SortField>> &fields, int size)
      {
        static_assert(std::is_base_of<FieldValueHitQueue.Entry, T>::value,
                      L"T must inherit from FieldValueHitQueue.Entry");

        if (fields.empty()) {
          throw std::invalid_argument("Sort must contain at least one field");
        }

        if (fields.size() == 1) {
          return std::make_shared<OneComparatorFieldValueHitQueue<>>(fields,
                                                                     size);
        } else {
          return std::make_shared<MultiComparatorsFieldValueHitQueue<>>(fields,
                                                                        size);
        }
      }

      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: public FieldComparator<?>[] getComparators()
      virtual std::deque < FieldComparator < ? >> getComparators()
      {
        return comparators;
      }

      virtual std::deque<int> getReverseMul() { return reverseMul; }

      virtual std::deque<std::shared_ptr<LeafFieldComparator>> getComparators(
          std::shared_ptr<LeafReaderContext> context) 
      {
        std::deque<std::shared_ptr<LeafFieldComparator>> comparators(
            this->comparators.size());
        for (int i = 0; i < comparators.size(); ++i) {
          comparators[i] = this->comparators[i]->getLeafComparator(context);
        }
        return comparators;
      }

      /** Stores the sort criteria being used. */
    protected:
      std::deque<std::shared_ptr<SortField>> const fields;
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: protected final FieldComparator<?>[] comparators;
      std::deque < FieldComparator < ? >> const comparators;
      std::deque<int> const reverseMul;

      bool lessThan(std::shared_ptr<Entry> a, std::shared_ptr<Entry> b) = 0;
      override

          /**
           * Given a queue Entry, creates a corresponding FieldDoc
           * that contains the values used to sort the given document.
           * These values are not the raw values out of the index, but the
           * internal representation of them. This is so the given search hit
           * can be collated by a MultiSearcher with other search hits.
           *
           * @param entry The Entry used to create a FieldDoc
           * @return The newly created FieldDoc
           * @see IndexSearcher#search(Query,int,Sort)
           */
          public : virtual std::shared_ptr<FieldDoc>
                   fillFields(std::shared_ptr<Entry> entry)
      {
        constexpr int n = comparators.size();
        const std::deque<std::any> fields = std::deque<std::any>(n);
        for (int i = 0; i < n; ++i) {
          fields[i] = comparators[i]->value(entry->slot);
        }
        // if (maxscore > 1.0f) doc.score /= maxscore;   // normalize scores
        return std::make_shared<FieldDoc>(entry->doc, entry->score, fields);
      }

      /** Returns the SortFields being used by this hit queue. */
      virtual std::deque<std::shared_ptr<SortField>> getFields()
      {
        return fields;
      }

    protected:
      std::shared_ptr<FieldValueHitQueue> shared_from_this()
      {
        return std::static_pointer_cast<FieldValueHitQueue>(
            org.apache.lucene.util.PriorityQueue<T>::shared_from_this());
      }
};

} // namespace org::apache::lucene::search
