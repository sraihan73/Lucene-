#pragma once
#include "stringhelper.h"
#include <memory>

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

/**
 * A deque of per-document numeric values, sorted
 * according to {@link Long#compare(long, long)}.
 *
 * @deprecated Use {@link SortedNumericDocValues} instead.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public abstract class LegacySortedNumericDocValues
class LegacySortedNumericDocValues
    : public std::enable_shared_from_this<LegacySortedNumericDocValues>
{

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  LegacySortedNumericDocValues();

  /**
   * Positions to the specified document
   */
public:
  virtual void setDocument(int doc) = 0;

  /**
   * Retrieve the value for the current document at the specified index.
   * An index ranges from {@code 0} to {@code count()-1}.
   */
  virtual int64_t valueAt(int index) = 0;

  /**
   * Retrieves the count of values for the current document.
   * This may be zero if a document has no values.
   */
  virtual int count() = 0;
};

} // #include  "core/src/java/org/apache/lucene/index/
