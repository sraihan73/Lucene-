#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::codecs
{
class Codec;
}
namespace org::apache::lucene::search
{
class Sort;
}
namespace org::apache::lucene::util
{
class Version;
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
using Sort = org::apache::lucene::search::Sort;
using Directory = org::apache::lucene::store::Directory;
using Version = org::apache::lucene::util::Version;

/**
 * Information about a segment such as its name, directory, and files related
 * to the segment.
 *
 * @lucene.experimental
 */
class SegmentInfo final : public std::enable_shared_from_this<SegmentInfo>
{
  GET_CLASS_NAME(SegmentInfo)

  // TODO: remove these from this class, for now this is the representation
  /** Used by some member fields to mean not present (e.g.,
   *  norms, deletions). */
public:
  static constexpr int NO = -1; // e.g. no norms; no deletes;

  /** Used by some member fields to mean present (e.g.,
   *  norms, deletions). */
  static constexpr int YES = 1; // e.g. have norms; have deletes;

  /** Unique segment name in the directory. */
  const std::wstring name;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  int maxDoc_ = 0; // number of docs in seg

  /** Where this segment resides. */
public:
  const std::shared_ptr<Directory> dir;

private:
  bool isCompoundFile = false;

  /** Id that uniquely identifies this segment. */
  std::deque<char> const id;

  std::shared_ptr<Codec> codec;

  std::unordered_map<std::wstring, std::wstring> diagnostics;

  const std::unordered_map<std::wstring, std::wstring> attributes;

  const std::shared_ptr<Sort> indexSort;

  // Tracks the Lucene version this segment was created with, since 3.1. Null
  // indicates an older than 3.0 index, and it's used to detect a too old index.
  // The format expected is "x.y" - "2.x" for pre-3.0 indexes (or null), and
  // specific versions afterwards ("3.0.0", "3.1.0" etc.).
  // see o.a.l.util.Version.
  const std::shared_ptr<Version> version;

  // Tracks the minimum version that contributed documents to a segment. For
  // flush segments, that is the version that wrote it. For merged segments,
  // this is the minimum minVersion of all the segments that have been merged
  // into this segment
public:
  std::shared_ptr<Version> minVersion;

  void
  setDiagnostics(std::unordered_map<std::wstring, std::wstring> &diagnostics);

  /** Returns diagnostics saved into the segment when it was
   *  written. The map_obj is immutable. */
  std::unordered_map<std::wstring, std::wstring> getDiagnostics();

  /**
   * Construct a new complete SegmentInfo instance from input.
   * <p>Note: this is public only to allow access from
   * the codecs package.</p>
   */
  SegmentInfo(std::shared_ptr<Directory> dir, std::shared_ptr<Version> version,
              std::shared_ptr<Version> minVersion, const std::wstring &name,
              int maxDoc, bool isCompoundFile, std::shared_ptr<Codec> codec,
              std::unordered_map<std::wstring, std::wstring> &diagnostics,
              std::deque<char> &id,
              std::unordered_map<std::wstring, std::wstring> &attributes,
              std::shared_ptr<Sort> indexSort);

  /**
   * Mark whether this segment is stored as a compound file.
   *
   * @param isCompoundFile true if this is a compound file;
   * else, false
   */
  void setUseCompoundFile(bool isCompoundFile);

  /**
   * Returns true if this segment is stored as a compound
   * file; else, false.
   */
  bool getUseCompoundFile();

  /** Can only be called once. */
  void setCodec(std::shared_ptr<Codec> codec);

  /** Return {@link Codec} that wrote this segment. */
  std::shared_ptr<Codec> getCodec();

  /** Returns number of documents in this segment (deletions
   *  are not taken into account). */
  int maxDoc();

  // NOTE: leave package private
  void setMaxDoc(int maxDoc);

  /** Return all files referenced by this SegmentInfo. */
  std::shared_ptr<Set<std::wstring>> files();

  virtual std::wstring toString();

  /** Used for debugging.  Format may suddenly change.
   *
   *  <p>Current format looks like
   *  <code>_a(3.1):c45/4:[sorter=&lt;long: "timestamp"&gt;!]</code>, which
   * means the segment's name is <code>_a</code>; it was created with Lucene 3.1
   * (or
   *  '?' if it's unknown); it's using compound file
   *  format (would be <code>C</code> if not compound); it
   *  has 45 documents; it has 4 deletions (this part is
   *  left off when there are no deletions); it is sorted by the timestamp field
   *  in descending order (this part is omitted for unsorted segments).</p>
   */
  std::wstring toString(int delCount);

  /** We consider another SegmentInfo instance equal if it
   *  has the same dir and same name. */
  bool equals(std::any obj) override;

  virtual int hashCode();

  /** Returns the version of the code which wrote the segment.
   */
  std::shared_ptr<Version> getVersion();

  /**
   * Return the minimum Lucene version that contributed documents to this
   * segment, or {@code null} if it is unknown.
   */
  std::shared_ptr<Version> getMinVersion();

  /** Return the id that uniquely identifies this segment. */
  std::deque<char> getId();

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  std::shared_ptr<Set<std::wstring>> setFiles_;

  /** Sets the files written for this segment. */
public:
  void setFiles(std::shared_ptr<std::deque<std::wstring>> files);

  /** Add these files to the set of files written for this
   *  segment. */
  void addFiles(std::shared_ptr<std::deque<std::wstring>> files);

  /** Add this file to the set of files written for this
   *  segment. */
  void addFile(const std::wstring &file);

private:
  void checkFileNames(std::shared_ptr<std::deque<std::wstring>> files);

  /**
   * strips any segment name from the file, naming it with this segment
   * this is because "segment names" can change, e.g. by addIndexes(Dir)
   */
public:
  std::wstring namedForThisSegment(const std::wstring &file);

  /**
   * Get a codec attribute value, or null if it does not exist
   */
  std::wstring getAttribute(const std::wstring &key);

  /**
   * Puts a codec attribute value.
   * <p>
   * This is a key-value mapping for the field that the codec can use to store
   * additional metadata, and will be available to the codec when reading the
   * segment via {@link #getAttribute(std::wstring)}
   * <p>
   * If a value already exists for the field, it will be replaced with the new
   * value.
   */
  std::wstring putAttribute(const std::wstring &key, const std::wstring &value);

  /**
   * Returns the internal codec attributes map_obj.
   * @return internal codec attributes map_obj.
   */
  std::unordered_map<std::wstring, std::wstring> getAttributes();

  /** Return the sort order of this segment, or null if the index has no sort.
   */
  std::shared_ptr<Sort> getIndexSort();
};

} // namespace org::apache::lucene::index
