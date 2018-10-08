#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <deque>

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

/** Base class for implementing {@link CompositeReader}s based on an array
 * of sub-readers. The implementing class has to add code for
 * correctly refcounting and closing the sub-readers.
 *
 * <p>User code will most likely use {@link MultiReader} to build a
 * composite reader on a set of sub-readers (like several
 * {@link DirectoryReader}s).
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
 * @see MultiReader
 * @lucene.internal
 */
template <typename R>
class BaseCompositeReader : public CompositeReader
{
  GET_CLASS_NAME(BaseCompositeReader)
  static_assert(std::is_base_of<IndexReader, R>::value,
                L"R must inherit from IndexReader");

private:
  std::deque<R> const subReaders;
  std::deque<int> const starts; // 1st docno for each reader
  // C++ NOTE: Fields cannot have the same name as methods:
  const int maxDoc_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int numDocs_;

  /** List view solely for {@link #getSequentialSubReaders()},
   * for effectiveness the array is used internally. */
  const std::deque<R> subReadersList;

  /**
   * Constructs a {@code BaseCompositeReader} on the given subReaders.
   * @param subReaders the wrapped sub-readers. This array is returned by
   * {@link #getSequentialSubReaders} and used to resolve the correct
   * subreader for docID-based methods. <b>Please note:</b> This array is
   * <b>not</b> cloned and not protected for modification, the subclass is
   * responsible to do this.
   */
protected:
  BaseCompositeReader(std::deque<R> &subReaders) 
      : subReaders(subReaders),
        starts(std::deque<int>(subReaders.size() + 1)) / *build starts array *
            /
        , maxDoc(Math::toIntExact(maxDoc_)),
        numDocs(Math::toIntExact(numDocs_)),
        subReadersList(
            Collections::unmodifiableList(Arrays::asList(subReaders)))
  {
    int64_t maxDoc = 0, numDocs = 0;
    for (int i = 0; i < subReaders.size(); i++) {
      starts[i] = static_cast<int>(maxDoc);
      std::shared_ptr<IndexReader> *const r = subReaders[i];
      maxDoc += r->maxDoc();   // compute maxDocs
      numDocs += r->numDocs(); // compute numDocs
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      r->registerParentReader(shared_from_this());
    }

    if (maxDoc > IndexWriter::getActualMaxDocs()) {
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      if (std::dynamic_pointer_cast<DirectoryReader>(shared_from_this()) !=
          nullptr) {
        // A single index has too many documents and it is corrupt (IndexWriter
        // prevents this as of LUCENE-6299)
        // C++ TODO: There is no native C++ equivalent to 'toString':
        throw std::make_shared<CorruptIndexException>(
            L"Too many documents: an index cannot exceed " +
                std::to_wstring(IndexWriter::getActualMaxDocs()) +
                L" but readers have total maxDoc=" + std::to_wstring(maxDoc),
            Arrays->toString(subReaders));
      } else {
        // Caller is building a MultiReader and it has too many documents; this
        // case is just illegal arguments:
        throw std::invalid_argument(
            "Too many documents: composite IndexReaders cannot exceed " +
            std::to_wstring(IndexWriter::getActualMaxDocs()) +
            L" but readers have total maxDoc=" + std::to_wstring(maxDoc));
      }
    }

    starts[subReaders.size()] = this->maxDoc_;
  }

public:
  std::shared_ptr<Fields>
  getTermVectors(int docID)  override final
  {
    ensureOpen();
    constexpr int i = readerIndex(docID); // find subreader num
    return subReaders[i]->getTermVectors(docID -
                                         starts[i]); // dispatch to subreader
  }

  int numDocs() override final
  {
    // Don't call ensureOpen() here (it could affect performance)
    return numDocs_;
  }

  int maxDoc() override final
  {
    // Don't call ensureOpen() here (it could affect performance)
    return maxDoc_;
  }

  void document(int docID, std::shared_ptr<StoredFieldVisitor> visitor) throw(
      IOException) override final
  {
    ensureOpen();
    constexpr int i = readerIndex(docID); // find subreader num
    subReaders[i]->document(docID - starts[i],
                            visitor); // dispatch to subreader
  }

  int docFreq(std::shared_ptr<Term> term)  override final
  {
    ensureOpen();
    int total = 0; // sum freqs in subreaders
    for (int i = 0; i < subReaders.size(); i++) {
      total += subReaders[i]->docFreq(term);
    }
    return total;
  }

  int64_t
  totalTermFreq(std::shared_ptr<Term> term)  override final
  {
    ensureOpen();
    int64_t total = 0; // sum freqs in subreaders
    for (int i = 0; i < subReaders.size(); i++) {
      int64_t sub = subReaders[i]->totalTermFreq(term);
      if (sub == -1) {
        return -1;
      }
      total += sub;
    }
    return total;
  }

  int64_t
  getSumDocFreq(const std::wstring &field)  override final
  {
    ensureOpen();
    int64_t total = 0; // sum doc freqs in subreaders
    for (auto reader : subReaders) {
      int64_t sub = reader->getSumDocFreq(field);
      if (sub == -1) {
        return -1; // if any of the subs doesn't support it, return -1
      }
      total += sub;
    }
    return total;
  }

  int getDocCount(const std::wstring &field)  override final
  {
    ensureOpen();
    int total = 0; // sum doc counts in subreaders
    for (auto reader : subReaders) {
      int sub = reader->getDocCount(field);
      if (sub == -1) {
        return -1; // if any of the subs doesn't support it, return -1
      }
      total += sub;
    }
    return total;
  }

  int64_t getSumTotalTermFreq(const std::wstring &field) throw(
      IOException) override final
  {
    ensureOpen();
    int64_t total = 0; // sum doc total term freqs in subreaders
    for (auto reader : subReaders) {
      int64_t sub = reader->getSumTotalTermFreq(field);
      if (sub == -1) {
        return -1; // if any of the subs doesn't support it, return -1
      }
      total += sub;
    }
    return total;
  }

  /** Helper method for subclasses to get the corresponding reader for a doc ID
   */
protected:
  int readerIndex(int docID)
  {
    if (docID < 0 || docID >= maxDoc_) {
      throw std::invalid_argument(
          "docID must be >= 0 and < maxDoc=" + std::to_wstring(maxDoc_) +
          L" (got docID=" + std::to_wstring(docID) + L")");
    }
    return ReaderUtil::subIndex(docID, this->starts);
  }

  /** Helper method for subclasses to get the docBase of the given sub-reader
   * index. */
  int readerBase(int readerIndex)
  {
    if (readerIndex < 0 || readerIndex >= subReaders.size()) {
      throw std::invalid_argument(
          "readerIndex must be >= 0 and < getSequentialSubReaders().size()");
    }
    return this->starts[readerIndex];
  }

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: @Override protected final java.util.List<? extends R>
  // getSequentialSubReaders()
  std::deque < ? extends R > getSequentialSubReaders() override final
  {
    return subReadersList;
  }

protected:
  std::shared_ptr<BaseCompositeReader> shared_from_this()
  {
    return std::static_pointer_cast<BaseCompositeReader>(
        CompositeReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
