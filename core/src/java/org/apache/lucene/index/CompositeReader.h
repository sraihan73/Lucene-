#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/CompositeReaderContext.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

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

using namespace org::apache::lucene::store;

/**
 Instances of this reader type can only
 be used to get stored fields from the underlying LeafReaders,
 but it is not possible to directly retrieve postings. To do that, get
 the {@link LeafReaderContext} for all sub-readers via {@link #leaves()}.

 <p>IndexReader instances for indexes on disk are usually constructed
 with a call to one of the static <code>DirectoryReader.open()</code> methods,
 e.g. {@link DirectoryReader#open(Directory)}. {@link DirectoryReader}
implements the {@code CompositeReader} interface, it is not possible to directly
get postings. <p> Concrete subclasses of IndexReader are usually constructed
with a call to one of the static <code>open()</code> methods, e.g. {@link
GET_CLASS_NAME(es)
 DirectoryReader#open(Directory)}.

 <p> For efficiency, in this API documents are often referred to via
 <i>document numbers</i>, non-negative integers which each name a unique
 document in the index.  These document numbers are ephemeral -- they may change
 as documents are added to and deleted from an index.  Clients should thus not
 rely on a given document having the same number between sessions.

 <p>
 <a name="thread-safety"></a><p><b>NOTE</b>: {@link
 IndexReader} instances are completely thread
 safe, meaning multiple threads can call any of its methods,
 concurrently.  If your application requires external
 synchronization, you should <b>not</b> synchronize on the
 <code>IndexReader</code> instance; use your own
 (non-Lucene) objects instead.
*/
class CompositeReader : public IndexReader
{
  GET_CLASS_NAME(CompositeReader)

private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile CompositeReaderContext readerContext =
  // null;
  std::shared_ptr<CompositeReaderContext> readerContext = nullptr; // lazy init

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  CompositeReader();

public:
  virtual std::wstring toString();

  /** Expert: returns the sequential sub readers that this
   *  reader is logically composed of. This method may not
   *  return {@code null}.
   *
   *  <p><b>NOTE:</b> In contrast to previous Lucene versions this method
   *  is no longer public, code that wants to get all {@link LeafReader}s
   *  this composite is composed of should use {@link IndexReader#leaves()}.
   * @see IndexReader#leaves()
   */
protected:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: protected abstract java.util.List<? extends IndexReader>
  // getSequentialSubReaders();
  virtual std::deque < ? extends IndexReader > getSequentialSubReaders() = 0;

public:
  std::shared_ptr<CompositeReaderContext> getContext() override final;

protected:
  std::shared_ptr<CompositeReader> shared_from_this()
  {
    return std::static_pointer_cast<CompositeReader>(
        IndexReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
