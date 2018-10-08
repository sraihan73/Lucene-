#pragma once
#include "../util/NamedSPILoader.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
template <typename S>
class NamedSPILoader;
}

namespace org::apache::lucene::codecs
{
class FieldsConsumer;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class FieldsProducer;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
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
namespace org::apache::lucene::codecs
{

using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using NamedSPILoader = org::apache::lucene::util::NamedSPILoader;

/**
 * Encodes/decodes terms, postings, and proximity data.
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
class PostingsFormat : public std::enable_shared_from_this<PostingsFormat>,
                       public NamedSPILoader::NamedSPI
{
  GET_CLASS_NAME(PostingsFormat)

  /**
   * This static holder class prevents classloading deadlock by delaying
   * init of postings formats until needed.
   */
private:
  class Holder final : public std::enable_shared_from_this<Holder>
  {
    GET_CLASS_NAME(Holder)
  private:
    static const std::shared_ptr<
        NamedSPILoader<std::shared_ptr<PostingsFormat>>>
        LOADER;

    Holder();

  public:
    static std::shared_ptr<NamedSPILoader<std::shared_ptr<PostingsFormat>>>
    getLoader();
  };

  /** Zero-length {@code PostingsFormat} array. */
public:
  static std::deque<std::shared_ptr<PostingsFormat>> const EMPTY;

  /** Unique name that's used to retrieve this format when
   *  reading the index.
   */
private:
  const std::wstring name;

  /**
   * Creates a new postings format.
   * <p>
   * The provided name will be written into the index segment in some
   * configurations (such as when using {@link PerFieldPostingsFormat}): in such
   * configurations, for the segment to be read this class should be registered
   * with Java's SPI mechanism (registered in META-INF/ of your jar file, etc).
   * @param name must be all ascii alphanumeric, and less than 128 characters in
   * length.
   */
protected:
  PostingsFormat(const std::wstring &name);

  /** Returns this posting format's name */
public:
  std::wstring getName() override;

  /** Writes a new segment */
  virtual std::shared_ptr<FieldsConsumer>
  fieldsConsumer(std::shared_ptr<SegmentWriteState> state) = 0;

  /** Reads a segment.  NOTE: by the time this call
   *  returns, it must hold open any files it will need to
   *  use; else, those files may be deleted.
   *  Additionally, required files may be deleted during the execution of
   *  this call before there is a chance to open them. Under these
   *  circumstances an IOException should be thrown by the implementation.
   *  IOExceptions are expected and will automatically cause a retry of the
   *  segment opening logic with the newly revised segments.
   *  */
  virtual std::shared_ptr<FieldsProducer>
  fieldsProducer(std::shared_ptr<SegmentReadState> state) = 0;

  virtual std::wstring toString();

  /** looks up a format by name */
  static std::shared_ptr<PostingsFormat> forName(const std::wstring &name);

  /** returns a deque of all available format names */
  static std::shared_ptr<Set<std::wstring>> availablePostingsFormats();

  /**
   * Reloads the postings format deque from the given {@link ClassLoader}.
   * Changes to the postings formats are visible after the method ends, all
   * iterators ({@link #availablePostingsFormats()},...) stay consistent.
   *
   * <p><b>NOTE:</b> Only new postings formats are added, existing ones are
   * never removed or replaced.
   *
   * <p><em>This method is expensive and should only be called for discovery
   * of new postings formats on the given classpath/classloader!</em>
   */
  static void reloadPostingsFormats(std::shared_ptr<ClassLoader> classloader);
};

} // namespace org::apache::lucene::codecs
