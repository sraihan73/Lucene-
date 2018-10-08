#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class RAMDirectory;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::store
{
class IndexOutput;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::util
{
class Accountable;
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
namespace org::apache::lucene::store
{

using RAMDirectory = org::apache::lucene::store::RAMDirectory; // javadocs
using Accountable = org::apache::lucene::util::Accountable;

// TODO
//   - let subclass dictate policy...?
//   - rename to MergeCacheingDir?  NRTCachingDir

/**
 * Wraps a {@link RAMDirectory}
 * around any provided delegate directory, to
 * be used during NRT search.
 *
 * <p>This class is likely only useful in a near-real-time
 * context, where indexing rate is lowish but reopen
 * rate is highish, resulting in many tiny files being
 * written.  This directory keeps such segments (as well as
 * the segments produced by merging them, as long as they
 * are small enough), in RAM.</p>
 *
 * <p>This is safe to use: when your app calls {IndexWriter#commit},
 * all cached files will be flushed from the cached and sync'd.</p>
 *
 * <p>Here's a simple example usage:
 *
 * <pre class="prettyprint">
 *   Directory fsDir = FSDirectory.open(new File("/path/to/index").toPath());
 *   NRTCachingDirectory cachedFSDir = new NRTCachingDirectory(fsDir,
 * 5.0, 60.0); IndexWriterConfig conf = new IndexWriterConfig(analyzer);
 *   IndexWriter writer = new IndexWriter(cachedFSDir, conf);
 * </pre>
 *
 * <p>This will cache all newly flushed segments, all merges
 * whose expected segment size is {@code <= 5 MB}, unless the net
 * cached bytes exceeds 60 MB at which point all writes will
 * not be cached (until the net bytes falls below 60 MB).</p>
 *
 * @lucene.experimental
 */

class NRTCachingDirectory : public FilterDirectory, public Accountable
{
  GET_CLASS_NAME(NRTCachingDirectory)

private:
  const std::shared_ptr<RAMDirectory> cache = std::make_shared<RAMDirectory>();

  const int64_t maxMergeSizeBytes;
  const int64_t maxCachedBytes;

  static constexpr bool VERBOSE = false;

  /**
   *  We will cache a newly created output if 1) it's a
   *  flush or a merge and the estimated size of the merged segment is
   *  {@code <= maxMergeSizeMB}, and 2) the total cached bytes is
   *  {@code <= maxCachedMB} */
public:
  NRTCachingDirectory(std::shared_ptr<Directory> delegate_,
                      double maxMergeSizeMB, double maxCachedMB);

  virtual std::wstring toString();

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::deque<std::wstring> listAll()  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void deleteFile(const std::wstring &name)  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t fileLength(const std::wstring &name)  override;

  virtual std::deque<std::wstring> listCachedFiles();

  std::shared_ptr<IndexOutput>
  createOutput(const std::wstring &name,
               std::shared_ptr<IOContext> context)  override;

  void sync(std::shared_ptr<std::deque<std::wstring>> fileNames) throw(
      IOException) override;

  void rename(const std::wstring &source,
              const std::wstring &dest)  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<IndexInput>
  openInput(const std::wstring &name,
            std::shared_ptr<IOContext> context)  override;

  /** Close this directory, which flushes any cached files
   *  to the delegate and then closes the delegate. */
  virtual ~NRTCachingDirectory();

  /** Subclass can override this to customize logic; return
   *  true if this file should be written to the RAMDirectory. */
protected:
  virtual bool doCacheWrite(const std::wstring &name,
                            std::shared_ptr<IOContext> context);

public:
  std::shared_ptr<IndexOutput> createTempOutput(
      const std::wstring &prefix, const std::wstring &suffix,
      std::shared_ptr<IOContext> context)  override;

  /** Returns true if the file exists
   *  (can be opened), false if it cannot be opened, and
   *  (unlike Java's File.exists) throws IOException if
   *  there's some unexpected error. */
  static bool slowFileExists(std::shared_ptr<Directory> dir,
                             const std::wstring &fileName) ;

private:
  std::mutex uncacheLock;

  void unCache(const std::wstring &fileName) ;

public:
  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

protected:
  std::shared_ptr<NRTCachingDirectory> shared_from_this()
  {
    return std::static_pointer_cast<NRTCachingDirectory>(
        FilterDirectory::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
