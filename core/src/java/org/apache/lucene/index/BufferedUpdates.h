#pragma once
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Term.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/DocValuesUpdate.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValuesUpdate.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValuesUpdate.h"

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

using BinaryDocValuesUpdate =
    org::apache::lucene::index::DocValuesUpdate::BinaryDocValuesUpdate;
using NumericDocValuesUpdate =
    org::apache::lucene::index::DocValuesUpdate::NumericDocValuesUpdate;
using Query = org::apache::lucene::search::Query;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/** Holds buffered deletes and updates, by docID, term or query for a
 *  single segment. This is used to hold buffered pending
 *  deletes and updates against the to-be-flushed segment.  Once the
 *  deletes and updates are pushed (on flush in DocumentsWriter), they
 *  are converted to a {@link FrozenBufferedUpdates} instance and
 *  pushed to the {@link BufferedUpdatesStream}. */

// NOTE: instances of this class are accessed either via a private
// instance on DocumentWriterPerThread, or via sync'd code by
// DocumentsWriterDeleteQueue

class BufferedUpdates : public std::enable_shared_from_this<BufferedUpdates>
{
  GET_CLASS_NAME(BufferedUpdates)

  /* Rough logic: HashMap has an array[Entry] w/ varying
     load factor (say 2 * POINTER).  Entry is object w/ Term
     key, Integer val, int hash, Entry next
     (OBJ_HEADER + 3*POINTER + INT).  Term is object w/
     std::wstring field and std::wstring text (OBJ_HEADER + 2*POINTER).
     Term's field is std::wstring (OBJ_HEADER + 4*INT + POINTER +
     OBJ_HEADER + string.length*CHAR).
     Term's text is std::wstring (OBJ_HEADER + 4*INT + POINTER +
     OBJ_HEADER + string.length*CHAR).  Integer is
     OBJ_HEADER + INT. */
public:
  static const int BYTES_PER_DEL_TERM =
      9 * RamUsageEstimator::NUM_BYTES_OBJECT_REF +
      7 * RamUsageEstimator::NUM_BYTES_OBJECT_HEADER + 10 * Integer::BYTES;

  /* Rough logic: del docIDs are List<Integer>.  Say deque
     allocates ~2X size (2*POINTER).  Integer is OBJ_HEADER
     + int */
  static const int BYTES_PER_DEL_DOCID =
      2 * RamUsageEstimator::NUM_BYTES_OBJECT_REF +
      RamUsageEstimator::NUM_BYTES_OBJECT_HEADER + Integer::BYTES;

  /* Rough logic: HashMap has an array[Entry] w/ varying
     load factor (say 2 * POINTER).  Entry is object w/
     Query key, Integer val, int hash, Entry next
     (OBJ_HEADER + 3*POINTER + INT).  Query we often
     undercount (say 24 bytes).  Integer is OBJ_HEADER + INT. */
  static const int BYTES_PER_DEL_QUERY =
      5 * RamUsageEstimator::NUM_BYTES_OBJECT_REF +
      2 * RamUsageEstimator::NUM_BYTES_OBJECT_HEADER + 2 * Integer::BYTES + 24;

  /* Rough logic: NumericUpdate calculates its actual size,
   * including the update Term and DV field (std::wstring). The
   * per-field map_obj holds a reference to the updated field, and
   * therefore we only account for the object reference and
   * map_obj space itself. This is incremented when we first see
   * an updated field.
   *
   * HashMap has an array[Entry] w/ varying load
   * factor (say 2*POINTER). Entry is an object w/ std::wstring key,
   * LinkedHashMap val, int hash, Entry next (OBJ_HEADER + 3*POINTER + INT).
   *
   * LinkedHashMap (val) is counted as OBJ_HEADER, array[Entry] ref + header,
   * 4*INT, 1*FLOAT, Set (entrySet) (2*OBJ_HEADER + ARRAY_HEADER + 2*POINTER +
   * 4*INT + FLOAT)
   */
  static const int BYTES_PER_NUMERIC_FIELD_ENTRY =
      7 * RamUsageEstimator::NUM_BYTES_OBJECT_REF +
      3 * RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
      RamUsageEstimator::NUM_BYTES_ARRAY_HEADER + 5 * Integer::BYTES +
      Float::BYTES;

  /* Rough logic: Incremented when we see another Term for an already updated
   * field.
   * LinkedHashMap has an array[Entry] w/ varying load factor
   * (say 2*POINTER). Entry is an object w/ Term key, NumericUpdate val,
   * int hash, Entry next, Entry before, Entry after (OBJ_HEADER + 5*POINTER +
   * INT).
   *
   * Term (key) is counted only as POINTER.
   * NumericUpdate (val) counts its own size and isn't accounted for here.
   */
  static const int BYTES_PER_NUMERIC_UPDATE_ENTRY =
      7 * RamUsageEstimator::NUM_BYTES_OBJECT_REF +
      RamUsageEstimator::NUM_BYTES_OBJECT_HEADER + Integer::BYTES;

  /* Rough logic: BinaryUpdate calculates its actual size,
   * including the update Term and DV field (std::wstring). The
   * per-field map_obj holds a reference to the updated field, and
   * therefore we only account for the object reference and
   * map_obj space itself. This is incremented when we first see
   * an updated field.
   *
   * HashMap has an array[Entry] w/ varying load
   * factor (say 2*POINTER). Entry is an object w/ std::wstring key,
   * LinkedHashMap val, int hash, Entry next (OBJ_HEADER + 3*POINTER + INT).
   *
   * LinkedHashMap (val) is counted as OBJ_HEADER, array[Entry] ref + header,
   * 4*INT, 1*FLOAT, Set (entrySet) (2*OBJ_HEADER + ARRAY_HEADER + 2*POINTER +
   * 4*INT + FLOAT)
   */
  static const int BYTES_PER_BINARY_FIELD_ENTRY =
      7 * RamUsageEstimator::NUM_BYTES_OBJECT_REF +
      3 * RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
      RamUsageEstimator::NUM_BYTES_ARRAY_HEADER + 5 * Integer::BYTES +
      Float::BYTES;

  /* Rough logic: Incremented when we see another Term for an already updated
   * field.
   * LinkedHashMap has an array[Entry] w/ varying load factor
   * (say 2*POINTER). Entry is an object w/ Term key, BinaryUpdate val,
   * int hash, Entry next, Entry before, Entry after (OBJ_HEADER + 5*POINTER +
   * INT).
   *
   * Term (key) is counted only as POINTER.
   * BinaryUpdate (val) counts its own size and isn't accounted for here.
   */
  static const int BYTES_PER_BINARY_UPDATE_ENTRY =
      7 * RamUsageEstimator::NUM_BYTES_OBJECT_REF +
      RamUsageEstimator::NUM_BYTES_OBJECT_HEADER + Integer::BYTES;

  const std::shared_ptr<AtomicInteger> numTermDeletes =
      std::make_shared<AtomicInteger>();
  const std::shared_ptr<AtomicInteger> numNumericUpdates =
      std::make_shared<AtomicInteger>();
  const std::shared_ptr<AtomicInteger> numBinaryUpdates =
      std::make_shared<AtomicInteger>();

  const std::unordered_map<std::shared_ptr<Term>, int> deleteTerms =
      std::unordered_map<std::shared_ptr<Term>, int>();
  const std::unordered_map<std::shared_ptr<Query>, int> deleteQueries =
      std::unordered_map<std::shared_ptr<Query>, int>();
  const std::deque<int> deleteDocIDs = std::deque<int>();

  // Map<dvField,Map<updateTerm,NumericUpdate>>
  // For each field we keep an ordered deque of NumericUpdates, key'd by the
  // update Term. LinkedHashMap guarantees we will later traverse the map_obj in
  // insertion order (so that if two terms affect the same document, the last
  // one that came in wins), and helps us detect faster if the same Term is
  // used to update the same field multiple times (so we later traverse it
  // only once).
  const std::unordered_map<
      std::wstring, LinkedHashMap<std::shared_ptr<Term>,
                                  std::shared_ptr<NumericDocValuesUpdate>>>
      numericUpdates = std::unordered_map<
          std::wstring,
          LinkedHashMap<std::shared_ptr<Term>,
                        std::shared_ptr<NumericDocValuesUpdate>>>();

  // Map<dvField,Map<updateTerm,BinaryUpdate>>
  // For each field we keep an ordered deque of BinaryUpdates, key'd by the
  // update Term. LinkedHashMap guarantees we will later traverse the map_obj in
  // insertion order (so that if two terms affect the same document, the last
  // one that came in wins), and helps us detect faster if the same Term is
  // used to update the same field multiple times (so we later traverse it
  // only once).
  const std::unordered_map<
      std::wstring, LinkedHashMap<std::shared_ptr<Term>,
                                  std::shared_ptr<BinaryDocValuesUpdate>>>
      binaryUpdates = std::unordered_map<
          std::wstring,
          LinkedHashMap<std::shared_ptr<Term>,
                        std::shared_ptr<BinaryDocValuesUpdate>>>();

  static const std::optional<int> MAX_INT;

  const std::shared_ptr<AtomicLong> bytesUsed;

private:
  static constexpr bool VERBOSE_DELETES = false;

public:
  int64_t gen = 0;

  const std::wstring segmentName;

  BufferedUpdates(const std::wstring &segmentName);

  virtual std::wstring toString();

  virtual void addQuery(std::shared_ptr<Query> query, int docIDUpto);

  virtual void addDocID(int docID);

  virtual void addTerm(std::shared_ptr<Term> term, int docIDUpto);

  virtual void addNumericUpdate(std::shared_ptr<NumericDocValuesUpdate> update,
                                int docIDUpto);

  virtual void addBinaryUpdate(std::shared_ptr<BinaryDocValuesUpdate> update,
                               int docIDUpto);

private:
  template <typename T>
  bool addDocValuesUpdate(
      std::unordered_map<std::wstring, LinkedHashMap<std::shared_ptr<Term>, T>>
          &updates,
      T update, int docIDUpto, std::function<T *(int)> &prepareForApply,
      int64_t bytesPerUpdateEntry, int64_t bytesPerFieldEntry);

public:
  virtual void clearDeleteTerms();

  virtual void clear();

  virtual bool any();
};

} // #include  "core/src/java/org/apache/lucene/index/
