#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
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

/** A {@link CompositeReader} which reads multiple indexes, appending
 *  their content. It can be used to create a view on several
 *  sub-readers (like {@link DirectoryReader}) and execute searches on it.
 *
 * <p> For efficiency, in this API documents are often referred to via
 * <i>document numbers</i>, non-negative integers which each name a unique
 * document in the index.  These document numbers are ephemeral -- they may
 * change as documents are added to and deleted from an index.  Clients should
 * thus not rely on a given document having the same number between sessions.
 *
 * <p><a name="thread-safety"></a><p><b>NOTE</b>: {@link
 * IndexReader} instances are completely thread
 * safe, meaning multiple threads can call any of its methods,
 * concurrently.  If your application requires external
 * synchronization, you should <b>not</b> synchronize on the
 * <code>IndexReader</code> instance; use your own
 * (non-Lucene) objects instead.
 */
class MultiReader : public BaseCompositeReader<std::shared_ptr<IndexReader>>
{
  GET_CLASS_NAME(MultiReader)
private:
  const bool closeSubReaders;

  /**
   * <p>Construct a MultiReader aggregating the named set of (sub)readers.
   * <p>Note that all subreaders are closed if this Multireader is closed.</p>
   * @param subReaders set of (sub)readers
   */
public:
  MultiReader(std::deque<IndexReader> &subReaders) ;

  /**
   * <p>Construct a MultiReader aggregating the named set of (sub)readers.
   * @param subReaders set of (sub)readers; this array will be cloned.
   * @param closeSubReaders indicates whether the subreaders should be closed
   * when this MultiReader is closed
   */
  MultiReader(std::deque<std::shared_ptr<IndexReader>> &subReaders,
              bool closeSubReaders) ;

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

protected:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void doClose()  override;

protected:
  std::shared_ptr<MultiReader> shared_from_this()
  {
    return std::static_pointer_cast<MultiReader>(
        BaseCompositeReader<IndexReader>::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
