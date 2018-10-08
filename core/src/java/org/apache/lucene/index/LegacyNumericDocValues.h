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
 * A per-document numeric value.
 *
 * @deprecated Use {@link NumericDocValues} instead.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public abstract class LegacyNumericDocValues
class LegacyNumericDocValues
    : public std::enable_shared_from_this<LegacyNumericDocValues>
{

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  LegacyNumericDocValues();

  /**
   * Returns the numeric value for the specified document ID.
   * @param docID document ID to lookup
   * @return numeric value
   */
public:
  virtual int64_t get(int docID) = 0;
};

} // #include  "core/src/java/org/apache/lucene/index/
