#pragma once
#include "../util/Accountable.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"

#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"

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
namespace org::apache::lucene::codecs
{

using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Accountable = org::apache::lucene::util::Accountable;

/** Abstract API that produces numeric, binary, sorted, sortedset,
 *  and sortednumeric docvalues.
 *
 * @lucene.experimental
 */
class DocValuesProducer
    : public std::enable_shared_from_this<DocValuesProducer>,
      public Accountable
{
  GET_CLASS_NAME(DocValuesProducer)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  DocValuesProducer();

  /** Returns {@link NumericDocValues} for this field.
   *  The returned instance need not be thread-safe: it will only be
   *  used by a single thread. */
public:
  virtual std::shared_ptr<NumericDocValues>
  getNumeric(std::shared_ptr<FieldInfo> field) = 0;

  /** Returns {@link BinaryDocValues} for this field.
   *  The returned instance need not be thread-safe: it will only be
   *  used by a single thread. */
  virtual std::shared_ptr<BinaryDocValues>
  getBinary(std::shared_ptr<FieldInfo> field) = 0;

  /** Returns {@link SortedDocValues} for this field.
   *  The returned instance need not be thread-safe: it will only be
   *  used by a single thread. */
  virtual std::shared_ptr<SortedDocValues>
  getSorted(std::shared_ptr<FieldInfo> field) = 0;

  /** Returns {@link SortedNumericDocValues} for this field.
   *  The returned instance need not be thread-safe: it will only be
   *  used by a single thread. */
  virtual std::shared_ptr<SortedNumericDocValues>
  getSortedNumeric(std::shared_ptr<FieldInfo> field) = 0;

  /** Returns {@link SortedSetDocValues} for this field.
   *  The returned instance need not be thread-safe: it will only be
   *  used by a single thread. */
  virtual std::shared_ptr<SortedSetDocValues>
  getSortedSet(std::shared_ptr<FieldInfo> field) = 0;

  /**
   * Checks consistency of this producer
   * <p>
   * Note that this may be costly in terms of I/O, e.g.
   * may involve computing a checksum value against large data files.
   * @lucene.internal
   */
  virtual void checkIntegrity() = 0;

  /**
   * Returns an instance optimized for merging.
   * <p>
   * The default implementation returns {@code this} */
  virtual std::shared_ptr<DocValuesProducer>
  getMergeInstance() ;
};

} // #include  "core/src/java/org/apache/lucene/codecs/
