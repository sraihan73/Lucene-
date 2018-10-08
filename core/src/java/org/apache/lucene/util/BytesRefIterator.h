#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::util
{

/**
 * A simple iterator interface for {@link BytesRef} iteration.
 */
class BytesRefIterator
{
  GET_CLASS_NAME(BytesRefIterator)

  /**
   * Increments the iteration to the next {@link BytesRef} in the iterator.
   * Returns the resulting {@link BytesRef} or <code>null</code> if the end of
   * the iterator is reached. The returned BytesRef may be re-used across calls
   * to next. After this method returns null, do not call it again: the results
   * are undefined.
   *
   * @return the next {@link BytesRef} in the iterator or <code>null</code> if
   *         the end of the iterator is reached.
   * @throws IOException If there is a low-level I/O error.
   */
public:
  virtual std::shared_ptr<BytesRef> next() = 0;

  /** Singleton BytesRefIterator that iterates over 0 BytesRefs. */
  static const std::shared_ptr<BytesRefIterator> EMPTY;
};

class BytesRefIteratorAnonymousInnerClass
    : public std::enable_shared_from_this<BytesRefIteratorAnonymousInnerClass>,
      public BytesRefIterator
{
  GET_CLASS_NAME(BytesRefIteratorAnonymousInnerClass)
public:
  BytesRefIteratorAnonymousInnerClass();

  std::shared_ptr<BytesRef> next() override;
};
} // #include  "core/src/java/org/apache/lucene/util/
