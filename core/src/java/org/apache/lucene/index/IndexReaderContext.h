#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class CompositeReaderContext;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
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
namespace org::apache::lucene::index
{

/**
 * A struct like class that represents a hierarchical relationship between
 * {@link IndexReader} instances.
GET_CLASS_NAME(that)
 */
class IndexReaderContext
    : public std::enable_shared_from_this<IndexReaderContext>
{
  GET_CLASS_NAME(IndexReaderContext)
  /** The reader context for this reader's immediate parent, or null if none */
public:
  const std::shared_ptr<CompositeReaderContext> parent;
  /** <code>true</code> if this context struct represents the top level reader
   * within the hierarchical context */
  const bool isTopLevel;
  /** the doc base for this reader in the parent, <tt>0</tt> if parent is null
   */
  const int docBaseInParent;
  /** the ord for this reader in the parent, <tt>0</tt> if parent is null */
  const int ordInParent;

  // An object that uniquely identifies this context without referencing
  // segments. The goal is to make it fine to have references to this
  // identity object, even after the index reader has been closed
  const std::any identity = nullptr;

  IndexReaderContext(std::shared_ptr<CompositeReaderContext> parent,
                     int ordInParent, int docBaseInParent);

  /** Expert: Return an {@link Object} that uniquely identifies this context.
   *  The returned object does neither reference this {@link IndexReaderContext}
   *  nor the wrapped {@link IndexReader}.
   *  @lucene.experimental */
  virtual std::any id();

  /** Returns the {@link IndexReader}, this context represents. */
  virtual std::shared_ptr<IndexReader> reader() = 0;

  /**
   * Returns the context's leaves if this context is a top-level context.
   * For convenience, if this is an {@link LeafReaderContext} this
   * returns itself as the only leaf.
   * <p>Note: this is convenience method since leaves can always be obtained by
   * walking the context tree using {@link #children()}.
   * @throws UnsupportedOperationException if this is not a top-level context.
   * @see #children()
   */
  virtual std::deque<std::shared_ptr<LeafReaderContext>> leaves() = 0;

  /**
   * Returns the context's children iff this context is a composite context
   * otherwise <code>null</code>.
   */
  virtual std::deque<std::shared_ptr<IndexReaderContext>> children() = 0;
};
} // namespace org::apache::lucene::index
