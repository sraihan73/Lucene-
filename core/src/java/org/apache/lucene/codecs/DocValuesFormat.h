#pragma once
#include "../util/NamedSPILoader.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
template <typename S>
class NamedSPILoader;
}

#include  "core/src/java/org/apache/lucene/codecs/DocValuesConsumer.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"

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
namespace org::apache::lucene::codecs
{

using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using NamedSPILoader = org::apache::lucene::util::NamedSPILoader;

/**
 * Encodes/decodes per-document values.
 * <p>
 * Note, when extending this class, the name ({@link #getName}) may
 * written into the index in certain configurations. In order for the segment
 * to be read, the name must resolve to your implementation via {@link
 * #forName(std::wstring)}. This method uses Java's
 * {@link ServiceLoader Service Provider Interface} (SPI) to resolve format
 * names. <p> If you implement your own format, make sure that it has a no-arg
 * constructor so SPI can load it.
 * @see ServiceLoader
 * @lucene.experimental */
class DocValuesFormat : public std::enable_shared_from_this<DocValuesFormat>,
                        public NamedSPILoader::NamedSPI
{
  GET_CLASS_NAME(DocValuesFormat)

  /**
   * This static holder class prevents classloading deadlock by delaying
   * init of doc values formats until needed.
   */
private:
  class Holder final : public std::enable_shared_from_this<Holder>
  {
    GET_CLASS_NAME(Holder)
  private:
    static const std::shared_ptr<
        NamedSPILoader<std::shared_ptr<DocValuesFormat>>>
        LOADER;

    Holder();

  public:
    static std::shared_ptr<NamedSPILoader<std::shared_ptr<DocValuesFormat>>>
    getLoader();
  };

  /** Unique name that's used to retrieve this format when
   *  reading the index.
   */
private:
  const std::wstring name;

  /**
   * Creates a new docvalues format.
   * <p>
   * The provided name will be written into the index segment in some
   * configurations (such as when using {@code PerFieldDocValuesFormat}): in
   * such configurations, for the segment to be read this class should be
   * registered with Java's SPI mechanism (registered in META-INF/ of your jar
   * file, etc).
   * @param name must be all ascii alphanumeric, and less than 128 characters in
   * length.
   */
protected:
  DocValuesFormat(const std::wstring &name);

  /** Returns a {@link DocValuesConsumer} to write docvalues to the
   *  index. */
public:
  virtual std::shared_ptr<DocValuesConsumer>
  fieldsConsumer(std::shared_ptr<SegmentWriteState> state) = 0;

  /**
   * Returns a {@link DocValuesProducer} to read docvalues from the index.
   * <p>
   * NOTE: by the time this call returns, it must hold open any files it will
   * need to use; else, those files may be deleted. Additionally, required files
   * may be deleted during the execution of this call before there is a chance
   * to open them. Under these circumstances an IOException should be thrown by
   * the implementation. IOExceptions are expected and will automatically cause
   * a retry of the segment opening logic with the newly revised segments.
   */
  virtual std::shared_ptr<DocValuesProducer>
  fieldsProducer(std::shared_ptr<SegmentReadState> state) = 0;

  std::wstring getName() override;

  virtual std::wstring toString();

  /** looks up a format by name */
  static std::shared_ptr<DocValuesFormat> forName(const std::wstring &name);

  /** returns a deque of all available format names */
  static std::shared_ptr<Set<std::wstring>> availableDocValuesFormats();

  /**
   * Reloads the DocValues format deque from the given {@link ClassLoader}.
   * Changes to the docvalues formats are visible after the method ends, all
   * iterators ({@link #availableDocValuesFormats()},...) stay consistent.
   *
   * <p><b>NOTE:</b> Only new docvalues formats are added, existing ones are
   * never removed or replaced.
   *
   * <p><em>This method is expensive and should only be called for discovery
   * of new docvalues formats on the given classpath/classloader!</em>
   */
  static void reloadDocValuesFormats(std::shared_ptr<ClassLoader> classloader);
};

} // #include  "core/src/java/org/apache/lucene/codecs/
