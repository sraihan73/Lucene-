#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReaderContext.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

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
 * Common util methods for dealing with {@link IndexReader}s and {@link
 * IndexReaderContext}s.
 *
 * @lucene.internal
 */
class ReaderUtil final : public std::enable_shared_from_this<ReaderUtil>
{
  GET_CLASS_NAME(ReaderUtil)

private:
  ReaderUtil();

  /**
   * Walks up the reader tree and return the given context's top level reader
   * context, or in other words the reader tree's root context.
   */
public:
  static std::shared_ptr<IndexReaderContext>
  getTopLevelContext(std::shared_ptr<IndexReaderContext> context);

  /**
   * Returns index of the searcher/reader for document <code>n</code> in the
   * array used to construct this searcher/reader.
   */
  static int subIndex(int n, std::deque<int> &docStarts);

  /**
   * Returns index of the searcher/reader for document <code>n</code> in the
   * array used to construct this searcher/reader.
   */
  static int subIndex(int n,
                      std::deque<std::shared_ptr<LeafReaderContext>> &leaves);
};

} // #include  "core/src/java/org/apache/lucene/index/
