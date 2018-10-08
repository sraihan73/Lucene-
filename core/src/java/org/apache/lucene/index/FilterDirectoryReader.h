#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"

#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/index/IndexCommit.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

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
 * A FilterDirectoryReader wraps another DirectoryReader, allowing
 * implementations to transform or extend it.
 *
 * Subclasses should implement doWrapDirectoryReader to return an instance of
 * the subclass.
 *
 * If the subclass wants to wrap the DirectoryReader's subreaders, it should
 * also implement a SubReaderWrapper subclass, and pass an instance to its super
 * constructor.
 */
class FilterDirectoryReader : public DirectoryReader
{
  GET_CLASS_NAME(FilterDirectoryReader)

  /** Get the wrapped instance by <code>reader</code> as long as this reader is
   *  an instance of {@link FilterDirectoryReader}.  */
public:
  static std::shared_ptr<DirectoryReader>
  unwrap(std::shared_ptr<DirectoryReader> reader);

  /**
   * Factory class passed to FilterDirectoryReader constructor that allows
   * subclasses to wrap the filtered DirectoryReader's subreaders.  You
   * can use this to, e.g., wrap the subreaders with specialised
   * FilterLeafReader implementations.
   */
public:
  class SubReaderWrapper : public std::enable_shared_from_this<SubReaderWrapper>
  {
    GET_CLASS_NAME(SubReaderWrapper)

  private:
    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: private LeafReader[] wrap(java.util.List<?
    // extends LeafReader> readers)
    std::deque<std::shared_ptr<LeafReader>> wrap(std::deque<T1> readers);

    /** Constructor */
  public:
    SubReaderWrapper();

    /**
     * Wrap one of the parent DirectoryReader's subreaders
     * @param reader the subreader to wrap
     * @return a wrapped/filtered LeafReader
     */
    virtual std::shared_ptr<LeafReader>
    wrap(std::shared_ptr<LeafReader> reader) = 0;
  };

  /** The filtered DirectoryReader */
protected:
  const std::shared_ptr<DirectoryReader> in_;

  /**
   * Create a new FilterDirectoryReader that filters a passed in
   * DirectoryReader, using the supplied SubReaderWrapper to wrap its subreader.
   * @param in the DirectoryReader to filter
   * @param wrapper the SubReaderWrapper to use to wrap subreaders
   */
public:
  FilterDirectoryReader(
      std::shared_ptr<DirectoryReader> in_,
      std::shared_ptr<SubReaderWrapper> wrapper) ;

  /**
   * Called by the doOpenIfChanged() methods to return a new wrapped
   * DirectoryReader.
   *
   * Implementations should just return an instantiation of themselves, wrapping
   * the passed in DirectoryReader.
   *
   * @param in the DirectoryReader to wrap
   * @return the wrapped DirectoryReader
   */
protected:
  virtual std::shared_ptr<DirectoryReader>
  doWrapDirectoryReader(std::shared_ptr<DirectoryReader> in_) = 0;

private:
  std::shared_ptr<DirectoryReader>
  wrapDirectoryReader(std::shared_ptr<DirectoryReader> in_) ;

protected:
  std::shared_ptr<DirectoryReader>
  doOpenIfChanged()  override final;

  std::shared_ptr<DirectoryReader> doOpenIfChanged(
      std::shared_ptr<IndexCommit> commit)  override final;

  std::shared_ptr<DirectoryReader>
  doOpenIfChanged(std::shared_ptr<IndexWriter> writer,
                  bool applyAllDeletes)  override final;

public:
  int64_t getVersion() override;

  bool isCurrent()  override;

  std::shared_ptr<IndexCommit> getIndexCommit()  override;

protected:
  void doClose()  override;

  /** Returns the wrapped {@link DirectoryReader}. */
public:
  virtual std::shared_ptr<DirectoryReader> getDelegate();

protected:
  std::shared_ptr<FilterDirectoryReader> shared_from_this()
  {
    return std::static_pointer_cast<FilterDirectoryReader>(
        DirectoryReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
