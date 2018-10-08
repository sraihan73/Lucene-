#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SegmentCommitInfo;
}

namespace org::apache::lucene::util
{
class Version;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::store
{
class ChecksumIndexInput;
}
namespace org::apache::lucene::codecs
{
class Codec;
}
namespace org::apache::lucene::store
{
class DataInput;
}
namespace org::apache::lucene::store
{
class IndexOutput;
}
namespace org::apache::lucene::index
{
class MergePolicy;
}
namespace org::apache::lucene::index
{
class OneMerge;
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

using Codec = org::apache::lucene::codecs::Codec;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using DataInput = org::apache::lucene::store::DataInput;
using Directory = org::apache::lucene::store::Directory;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Version = org::apache::lucene::util::Version;

/**
 * A collection of segmentInfo objects with methods for operating on those
 * segments in relation to the file system.
 * <p>
 * The active segments in the index are stored in the segment info file,
 * <tt>segments_N</tt>. There may be one or more <tt>segments_N</tt> files in
 * the index; however, the one with the largest generation is the active one
 * (when older segments_N files are present it's because they temporarily cannot
 * be deleted, or a custom {@link IndexDeletionPolicy} is in
 * use). This file lists each segment by name and has details about the codec
 * and generation of deletes.
 * </p>
 * <p>
 * Files:
 * <ul>
 * <li><tt>segments_N</tt>: Header, LuceneVersion, Version, NameCounter,
 * SegCount, MinSegmentLuceneVersion, &lt;SegName, SegID, SegCodec, DelGen,
 * DeletionCount, FieldInfosGen, DocValuesGen,
 * UpdatesFiles&gt;<sup>SegCount</sup>, CommitUserData, Footer
 * </ul>
 * Data types:
 * <ul>
 * <li>Header --&gt; {@link CodecUtil#writeIndexHeader IndexHeader}</li>
 * <li>LuceneVersion --&gt; Which Lucene code {@link Version} was used for this
 * commit, written as three {@link DataOutput#writeVInt vInt}: major, minor,
 * bugfix <li>MinSegmentLuceneVersion --&gt; Lucene code {@link Version} of the
 * oldest segment, written as three {@link DataOutput#writeVInt vInt}: major,
 * minor, bugfix; this is only written only if there's at least one segment
 * <li>NameCounter, SegCount, DeletionCount --&gt;
 * {@link DataOutput#writeInt Int32}</li>
 * <li>Generation, Version, DelGen, Checksum, FieldInfosGen, DocValuesGen --&gt;
 * {@link DataOutput#writeLong Int64}</li>
 * <li>SegID --&gt; {@link DataOutput#writeByte Int8<sup>ID_LENGTH</sup>}</li>
 * <li>SegName, SegCodec --&gt; {@link DataOutput#writeString std::wstring}</li>
 * <li>CommitUserData --&gt; {@link DataOutput#writeMapOfStrings
 * Map&lt;std::wstring,std::wstring&gt;}</li>
 * <li>UpdatesFiles --&gt; Map&lt;{@link DataOutput#writeInt Int32},
 * {@link DataOutput#writeSetOfStrings(Set) Set&lt;std::wstring&gt;}&gt;</li>
 * <li>Footer --&gt; {@link CodecUtil#writeFooter CodecFooter}</li>
 * </ul>
 * Field Descriptions:
 * <ul>
 * <li>Version counts how often the index has been changed by adding or deleting
 * documents.</li>
 * <li>NameCounter is used to generate names for new segment files.</li>
 * <li>SegName is the name of the segment, and is used as the file name prefix
 * for all of the files that compose the segment's index.</li>
 * <li>DelGen is the generation count of the deletes file. If this is -1, there
 * are no deletes. Anything above zero means there are deletes stored by
 * {@link LiveDocsFormat}.</li>
 * <li>DeletionCount records the number of deleted documents in this
 * segment.</li> <li>SegCodec is the {@link Codec#getName() name} of the Codec
 * that encoded this segment.</li> <li>SegID is the identifier of the Codec that
 * encoded this segment. </li> <li>CommitUserData stores an optional
 * user-supplied opaque Map&lt;std::wstring,std::wstring&gt; that was passed to
 * {@link IndexWriter#setLiveCommitData(Iterable)}.</li>
 * <li>FieldInfosGen is the generation count of the fieldInfos file. If this is
 * -1, there are no updates to the fieldInfos in that segment. Anything above
 * zero means there are updates to fieldInfos stored by {@link FieldInfosFormat}
 * .</li>
 * <li>DocValuesGen is the generation count of the updatable DocValues. If this
 * is -1, there are no updates to DocValues in that segment. Anything above zero
 * means there are updates to DocValues stored by {@link DocValuesFormat}.</li>
 * <li>UpdatesFiles stores the set of files that were updated in that segment
 * per field.</li>
 * </ul>
 *
 * @lucene.experimental
 */
class SegmentInfos final
    : public std::enable_shared_from_this<SegmentInfos>,
      public Cloneable,
      public std::deque<std::shared_ptr<SegmentCommitInfo>>
{
  GET_CLASS_NAME(SegmentInfos)

  /** Adds the {@link Version} that committed this segments_N file, as well as
   * the {@link Version} of the oldest segment, since 5.3+ */
public:
  static constexpr int VERSION_53 = 6;
  /** The version that added information about the Lucene version at the time
   * when the index has been created. */
  static constexpr int VERSION_70 = 7;
  /** The version that updated segment name counter to be long instead of int.
   */
  static constexpr int VERSION_72 = 8;
  /** The version that recorded softDelCount */
  static constexpr int VERSION_74 = 9;
  static constexpr int VERSION_CURRENT = VERSION_74;

  /** Used to name new segments. */
  int64_t counter = 0;

  /** Counts how often the index has been changed.  */
  int64_t version = 0;

private:
  int64_t generation =
      0; // generation of the "segments_N" for the next commit
  int64_t lastGeneration =
      0; // generation of the "segments_N" file we last successfully read
         // or wrote; this is normally the same as generation except if
         // there was an IOException that had interrupted a commit

  /** Opaque Map&lt;std::wstring, std::wstring&gt; that user can specify during
   * IndexWriter.commit */
public:
  std::unordered_map<std::wstring, std::wstring> userData =
      Collections::emptyMap();

private:
  std::deque<std::shared_ptr<SegmentCommitInfo>> segments =
      std::deque<std::shared_ptr<SegmentCommitInfo>>();

  /**
   * If non-null, information about loading segments_N files
   * will be printed here.  @see #setInfoStream.
   */
  static std::shared_ptr<PrintStream> infoStream;

  /** Id for this commit; only written starting with Lucene 5.0 */
  std::deque<char> id;

  /** Which Lucene version wrote this commit. */
  std::shared_ptr<Version> luceneVersion;

  /** Version of the oldest segment in the index, or null if there are no
   * segments. */
  std::shared_ptr<Version> minSegmentLuceneVersion;

  /** The Lucene version major that was used to create the index. */
  const int indexCreatedVersionMajor;

  /** Sole constructor.
   *  @param indexCreatedVersionMajor the Lucene version major at index creation
   * time, or 6 if the index was created before 7.0 */
public:
  SegmentInfos(int indexCreatedVersionMajor);

  /** Returns {@link SegmentCommitInfo} at the provided
   *  index. */
  std::shared_ptr<SegmentCommitInfo> info(int i);

  /**
   * Get the generation of the most recent commit to the
   * deque of index files (N in the segments_N file).
   *
   * @param files -- array of file names to check
   */
  static int64_t getLastCommitGeneration(std::deque<std::wstring> &files);

  /**
   * Get the generation of the most recent commit to the
   * index in this directory (N in the segments_N file).
   *
   * @param directory -- directory to search for the latest segments_N file
   */
  static int64_t getLastCommitGeneration(
      std::shared_ptr<Directory> directory) ;

  /**
   * Get the filename of the segments_N file for the most
   * recent commit in the deque of index files.
   *
   * @param files -- array of file names to check
   */

  static std::wstring
  getLastCommitSegmentsFileName(std::deque<std::wstring> &files);

  /**
   * Get the filename of the segments_N file for the most
   * recent commit to the index in this Directory.
   *
   * @param directory -- directory to search for the latest segments_N file
   */
  static std::wstring getLastCommitSegmentsFileName(
      std::shared_ptr<Directory> directory) ;

  /**
   * Get the segments_N filename in use by this segment infos.
   */
  std::wstring getSegmentsFileName();

  /**
   * Parse the generation off the segments file name and
   * return it.
   */
  static int64_t generationFromSegmentsFileName(const std::wstring &fileName);

  /** return generation of the next pending_segments_N that will be written */
private:
  int64_t getNextPendingGeneration();

  /** Since Lucene 5.0, every commit (segments_N) writes a unique id.  This will
   *  return that id */
public:
  std::deque<char> getId();

  /**
   * Read a particular segmentFileName.  Note that this may
   * throw an IOException if a commit is in process.
   *
   * @param directory -- directory containing the segments file
   * @param segmentFileName -- segment file to load
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  static std::shared_ptr<SegmentInfos>
  readCommit(std::shared_ptr<Directory> directory,
             const std::wstring &segmentFileName) ;

  /** Read the commit from the provided {@link ChecksumIndexInput}. */
  static std::shared_ptr<SegmentInfos>
  readCommit(std::shared_ptr<Directory> directory,
             std::shared_ptr<ChecksumIndexInput> input,
             int64_t generation) ;

private:
  static std::shared_ptr<Codec>
  readCodec(std::shared_ptr<DataInput> input) ;

  /** Find the latest commit ({@code segments_N file}) and
   *  load all {@link SegmentCommitInfo}s. */
public:
  static std::shared_ptr<SegmentInfos>
  readLatestCommit(std::shared_ptr<Directory> directory) ;

private:
  class FindSegmentsFileAnonymousInnerClass
      : public FindSegmentsFile<std::shared_ptr<SegmentInfos>>
  {
    GET_CLASS_NAME(FindSegmentsFileAnonymousInnerClass)
  private:
    std::shared_ptr<Directory> directory;

  public:
    FindSegmentsFileAnonymousInnerClass(std::shared_ptr<Directory> directory);

  protected:
    std::shared_ptr<SegmentInfos>
    doBody(const std::wstring &segmentFileName)  override;

  protected:
    std::shared_ptr<FindSegmentsFileAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FindSegmentsFileAnonymousInnerClass>(
          FindSegmentsFile<SegmentInfos>::shared_from_this());
    }
  };

  // Only true after prepareCommit has been called and
  // before finishCommit is called
public:
  bool pendingCommit = false;

private:
  void write(std::shared_ptr<Directory> directory) ;

  /** Write ourselves to the provided {@link IndexOutput} */
public:
  void write(std::shared_ptr<Directory> directory,
             std::shared_ptr<IndexOutput> out) ;

  /**
   * Returns a copy of this instance, also copying each
   * SegmentInfo.
   */

  std::shared_ptr<SegmentInfos> clone() override;

  /**
   * version number when this SegmentInfos was generated.
   */
  int64_t getVersion();

  /** Returns current generation. */
  int64_t getGeneration();

  /** Returns last succesfully read or written generation. */
  int64_t getLastGeneration();

  /** If non-null, information about retries when loading
   * the segments file will be printed to this.
   */
  static void setInfoStream(std::shared_ptr<PrintStream> infoStream);

  /**
   * Returns {@code infoStream}.
   *
   * @see #setInfoStream
   */
  static std::shared_ptr<PrintStream> getInfoStream();

  /**
   * Prints the given message to the infoStream. Note, this method does not
   * check for null infoStream. It assumes this check has been performed by the
   * caller, which is recommended to avoid the (usually) expensive message
   * creation.
   */
private:
  static void message(const std::wstring &message);

  /**
   * Utility class for executing code that needs to do
   * something with the current segments file.  This is
   * necessary with lock-less commits because from the time
   * you locate the current segments file name, until you
   * actually open it, read its contents, or check modified
   * time, etc., it could have been deleted due to a writer
   * commit finishing.
   */
public:
  template <typename T>
  class FindSegmentsFile : public std::enable_shared_from_this<FindSegmentsFile>
  {
    GET_CLASS_NAME(FindSegmentsFile)

  public:
    const std::shared_ptr<Directory> directory;

    /** Sole constructor. */
    FindSegmentsFile(std::shared_ptr<Directory> directory)
        : directory(directory)
    {
    }

    /** Locate the most recent {@code segments} file and
     *  run {@link #doBody} on it. */
    virtual T run()  { return run(nullptr); }

    /** Run {@link #doBody} on the provided commit. */
    virtual T run(std::shared_ptr<IndexCommit> commit) 
    {
      if (commit != nullptr) {
        if (directory != commit->getDirectory()) {
          // C++ TODO: The following line could not be converted:
          throw java.io.IOException(
              L"the specified commit does not match the specified Directory");
        }
        return doBody(commit->getSegmentsFileName());
      }

      int64_t lastGen = -1;
      int64_t gen = -1;
      std::shared_ptr<IOException> exc = nullptr;

      // Loop until we succeed in calling doBody() without
      // hitting an IOException.  An IOException most likely
      // means an IW deleted our commit while opening
      // the time it took us to load the now-old infos files
      // (and segments files).  It's also possible it's a
      // true error (corrupt index).  To distinguish these,
      // on each retry we must see "forward progress" on
      // which generation we are trying to load.  If we
      // don't, then the original error is real and we throw
      // it.

      for (;;) {
        lastGen = gen;
        std::deque<std::wstring> files = directory->listAll();
        std::deque<std::wstring> files2 = directory->listAll();
        Arrays::sort(files);
        Arrays::sort(files2);
        if (!Arrays::equals(files, files2)) {
          // listAll() is weakly consistent, this means we hit "concurrent
          // modification exception"
          continue;
        }
        gen = getLastCommitGeneration(files);

        if (infoStream != nullptr) {
          message(L"directory listing gen=" + std::to_wstring(gen));
        }

        if (gen == -1) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          throw std::make_shared<IndexNotFoundException>(
              L"no segments* file found in " + directory + L": files: " +
              Arrays->toString(files));
        } else if (gen > lastGen) {
          std::wstring segmentFileName = IndexFileNames::fileNameFromGeneration(
              IndexFileNames::SEGMENTS, L"", gen);

          try {
            T t = doBody(segmentFileName);
            if (infoStream != nullptr) {
              message(L"success on " + segmentFileName);
            }
            return t;
          } catch (const IOException &err) {
            // Save the original root cause:
            if (exc == nullptr) {
              exc = err;
            }

            if (infoStream != nullptr) {
              message(L"primary Exception on '" + segmentFileName + L"': " +
                      err + L"'; will retry: gen = " + std::to_wstring(gen));
            }
          }
        } else {
          throw exc;
        }
      }
    }

    /**
     * Subclass must implement this.  The assumption is an
     * IOException will be thrown if something goes wrong
     * during the processing that could have been caused by
     * a writer committing.
     */
  protected:
    virtual T doBody(const std::wstring &segmentFileName) = 0;
  };

  /** Carry over generation numbers from another SegmentInfos
   *
   * @lucene.internal */
public:
  void updateGeneration(std::shared_ptr<SegmentInfos> other);

  // Carry over generation numbers, and version/counter, from another
  // SegmentInfos
  void updateGenerationVersionAndCounter(std::shared_ptr<SegmentInfos> other);

  /** Set the generation to be used for the next commit */
  void setNextWriteGeneration(int64_t generation);

  void rollbackCommit(std::shared_ptr<Directory> dir);

  /** Call this to start a commit.  This writes the new
   *  segments file, but writes an invalid checksum at the
   *  end, so that it is not visible to readers.  Once this
   *  is called you must call {@link #finishCommit} to complete
   *  the commit or {@link #rollbackCommit} to abort it.
   *  <p>
   *  Note: {@link #changed()} should be called prior to this
   *  method if changes have been made to this {@link SegmentInfos} instance
   *  </p>
   **/
  void prepareCommit(std::shared_ptr<Directory> dir) ;

  /** Returns all file names referenced by SegmentInfo.
   *  The returned collection is recomputed on each
   *  invocation.  */
  std::shared_ptr<std::deque<std::wstring>>
  files(bool includeSegmentsFile) ;

  /** Returns the committed segments_N filename. */
  std::wstring finishCommit(std::shared_ptr<Directory> dir) ;

  /** Writes and syncs to the Directory dir, taking care to
   *  remove the segments file on exception
   *  <p>
   *  Note: {@link #changed()} should be called prior to this
   *  method if changes have been made to this {@link SegmentInfos} instance
   *  </p>
   **/
  void commit(std::shared_ptr<Directory> dir) ;

  /** Returns readable description of this segment. */
  virtual std::wstring toString();

  /** Return {@code userData} saved with this commit.
   *
   * @see IndexWriter#commit()
   */
  std::unordered_map<std::wstring, std::wstring> getUserData();

  /** Sets the commit data. */
  void setUserData(std::unordered_map<std::wstring, std::wstring> &data,
                   bool doIncrementVersion);

  /** Replaces all segments in this instance, but keeps
   *  generation, version, counter so that future commits
   *  remain write once.
   */
  void replace(std::shared_ptr<SegmentInfos> other);

  /** Returns sum of all segment's maxDocs.  Note that
   *  this does not include deletions */
  int totalMaxDoc();

  /** Call this before committing if changes have been made to the
   *  segments. */
  void changed();

  void setVersion(int64_t newVersion);

  /** applies all changes caused by committing a merge to this SegmentInfos */
  void applyMergeChanges(std::shared_ptr<MergePolicy::OneMerge> merge,
                         bool dropSegment);

  std::deque<std::shared_ptr<SegmentCommitInfo>> createBackupSegmentInfos();

  void
  rollbackSegmentInfos(std::deque<std::shared_ptr<SegmentCommitInfo>> &infos);

  /** Returns an <b>unmodifiable</b> {@link Iterator} of contained segments in
   * order. */
  // @Override (comment out until Java 6)
  std::shared_ptr<Iterator<std::shared_ptr<SegmentCommitInfo>>>
  iterator() override;

  /** Returns all contained segments as an <b>unmodifiable</b> {@link List}
   * view. */
  std::deque<std::shared_ptr<SegmentCommitInfo>> asList();

  /** Returns number of {@link SegmentCommitInfo}s. */
  int size();

  /** Appends the provided {@link SegmentCommitInfo}. */
  void add(std::shared_ptr<SegmentCommitInfo> si);

  /** Appends the provided {@link SegmentCommitInfo}s. */
  void addAll(std::deque<std::shared_ptr<SegmentCommitInfo>> &sis);

  /** Clear all {@link SegmentCommitInfo}s. */
  void clear();

  /** Remove the provided {@link SegmentCommitInfo}.
   *
   * <p><b>WARNING</b>: O(N) cost */
  bool remove(std::shared_ptr<SegmentCommitInfo> si);

  /** Remove the {@link SegmentCommitInfo} at the
   * provided index.
   *
   * <p><b>WARNING</b>: O(N) cost */
  void remove(int index);

  /** Return true if the provided {@link
   *  SegmentCommitInfo} is contained.
   *
   * <p><b>WARNING</b>: O(N) cost */
  bool contains(std::shared_ptr<SegmentCommitInfo> si);

  /** Returns index of the provided {@link
   *  SegmentCommitInfo}.
   *
   * <p><b>WARNING</b>: O(N) cost */
  int indexOf(std::shared_ptr<SegmentCommitInfo> si);

  /** Returns which Lucene {@link Version} wrote this commit, or null if the
   *  version this index was written with did not directly record the version.
   */
  std::shared_ptr<Version> getCommitLuceneVersion();

  /** Returns the version of the oldest segment, or null if there are no
   * segments. */
  std::shared_ptr<Version> getMinSegmentLuceneVersion();

  /** Return the version major that was used to initially create the index.
   *  This version is set when the index is first created and then never
   *  changes. This information was added as of version 7.0 so older
   *  indices report 6 as a creation version. */
  int getIndexCreatedVersionMajor();
};

} // namespace org::apache::lucene::index
