#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CompoundFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"

#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/store/Lock.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"

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
namespace org::apache::lucene::codecs::simpletext
{

using CompoundFormat = org::apache::lucene::codecs::CompoundFormat;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/**
 * plain text compound format.
 * <p>
 * <b>FOR RECREATIONAL USE ONLY</b>
 * @lucene.experimental
 */
class SimpleTextCompoundFormat : public CompoundFormat
{
  GET_CLASS_NAME(SimpleTextCompoundFormat)

  /** Sole constructor. */
public:
  SimpleTextCompoundFormat();

  std::shared_ptr<Directory> getCompoundReader(
      std::shared_ptr<Directory> dir, std::shared_ptr<SegmentInfo> si,
      std::shared_ptr<IOContext> context)  override;

private:
  class DirectoryAnonymousInnerClass : public Directory
  {
    GET_CLASS_NAME(DirectoryAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTextCompoundFormat> outerInstance;

    std::shared_ptr<IOContext> context;
    std::shared_ptr<IndexInput> in_;
    std::deque<std::wstring> fileNames;
    std::deque<int64_t> startOffsets;
    std::deque<int64_t> endOffsets;

  public:
    DirectoryAnonymousInnerClass(
        std::shared_ptr<SimpleTextCompoundFormat> outerInstance,
        std::shared_ptr<IOContext> context, std::shared_ptr<IndexInput> in_,
        std::deque<std::wstring> &fileNames,
        std::deque<int64_t> &startOffsets,
        std::deque<int64_t> &endOffsets);

  private:
    int getIndex(const std::wstring &name) ;

  public:
    std::deque<std::wstring> listAll()  override;

    int64_t fileLength(const std::wstring &name)  override;

    std::shared_ptr<IndexInput>
    openInput(const std::wstring &name,
              std::shared_ptr<IOContext> context)  override;

    virtual ~DirectoryAnonymousInnerClass();

    // write methods: disabled

    std::shared_ptr<IndexOutput>
    createOutput(const std::wstring &name,
                 std::shared_ptr<IOContext> context) override;
    std::shared_ptr<IndexOutput>
    createTempOutput(const std::wstring &prefix, const std::wstring &suffix,
                     std::shared_ptr<IOContext> context) override;
    void sync(std::shared_ptr<std::deque<std::wstring>> names) override;
    void deleteFile(const std::wstring &name) override;
    void rename(const std::wstring &source, const std::wstring &dest) override;
    void syncMetaData() override;
    std::shared_ptr<Lock> obtainLock(const std::wstring &name) override;

  protected:
    std::shared_ptr<DirectoryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DirectoryAnonymousInnerClass>(
          org.apache.lucene.store.Directory::shared_from_this());
    }
  };

public:
  void write(std::shared_ptr<Directory> dir, std::shared_ptr<SegmentInfo> si,
             std::shared_ptr<IOContext> context)  override;

  // helper method to strip strip away 'prefix' from 'scratch' and return as
  // std::wstring
private:
  std::wstring stripPrefix(std::shared_ptr<BytesRefBuilder> scratch,
                           std::shared_ptr<BytesRef> prefix);

  /** Extension of compound file */
public:
  static const std::wstring DATA_EXTENSION;

  static const std::shared_ptr<BytesRef> HEADER;

  static const std::shared_ptr<BytesRef> TABLE;
  static const std::shared_ptr<BytesRef> TABLENAME;
  static const std::shared_ptr<BytesRef> TABLESTART;
  static const std::shared_ptr<BytesRef> TABLEEND;

  static const std::shared_ptr<BytesRef> TABLEPOS;

  static const std::wstring OFFSETPATTERN;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static SimpleTextCompoundFormat::StaticConstructor staticConstructor;

protected:
  std::shared_ptr<SimpleTextCompoundFormat> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextCompoundFormat>(
        org.apache.lucene.codecs.CompoundFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/simpletext/
