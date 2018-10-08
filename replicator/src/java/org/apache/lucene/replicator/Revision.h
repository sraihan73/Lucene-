#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::replicator
{
class RevisionFile;
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
namespace org::apache::lucene::replicator
{

/**
 * A revision comprises lists of files that come from different sources and need
 * to be replicated together to e.g. guarantee that all resources are in sync.
 * In most cases an application will replicate a single index, and so the
 * revision will contain files from a single source. However, some applications
 * may require to treat a collection of indexes as a single entity so that the
 * files from all sources are replicated together, to guarantee consistency
 * beween them. For example, an application which indexes facets will need to
 * replicate both the search and taxonomy indexes together, to guarantee that
 * they match at the client side.
 *
 * @lucene.experimental
 */
class Revision : public Comparable<std::shared_ptr<Revision>>
{
  GET_CLASS_NAME(Revision)

  /**
   * Compares the revision to the given version string. Behaves like
   * {@link Comparable#compareTo(Object)}.
   */
public:
  virtual int compareTo(const std::wstring &version) = 0;

  /**
   * Returns a string representation of the version of this revision. The
   * version is used by {@link #compareTo(std::wstring)} as well as to
   * serialize/deserialize revision information. Therefore it must be self
   * descriptive as well as be able to identify one revision from another.
   */
  virtual std::wstring getVersion() = 0;

  /**
   * Returns the files that comprise this revision, as a mapping from a source
   * to a deque of files.
   */
  virtual std::unordered_map<std::wstring,
                             std::deque<std::shared_ptr<RevisionFile>>>
  getSourceFiles() = 0;

  /**
   * Returns an {@link IndexInput} for the given fileName and source. It is the
   * caller's respnsibility to close the {@link IndexInput} when it has been
   * consumed.
   */
  virtual std::shared_ptr<InputStream> open(const std::wstring &source,
                                            const std::wstring &fileName) = 0;

  /**
   * Called when this revision can be safely released, i.e. where there are no
   * more references to it.
   */
  virtual void release() = 0;
};

} // namespace org::apache::lucene::replicator
