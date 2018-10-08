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
 */
class SortedNumericDocValues : public DocValuesIterator
{
  GET_CLASS_NAME(SortedNumericDocValues)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  SortedNumericDocValues();

  /**
   * Iterates to the next value in the current document.  Do not call this more
   * than {@link #docValueCount} times for the document.
   */
public:
  virtual int64_t nextValue() = 0;

  /**
   * Retrieves the number of values for the current document.  This must always
   * be greater than zero.
   * It is illegal to call this method after {@link #advanceExact(int)}
   * returned {@code false}.
   */
  virtual int docValueCount() = 0;

protected:
  std::shared_ptr<SortedNumericDocValues> shared_from_this()
  {
    return std::static_pointer_cast<SortedNumericDocValues>(
        DocValuesIterator::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
