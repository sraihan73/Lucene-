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

namespace org::apache::lucene::codecs
{
class PostingsFormat;
}
namespace org::apache::lucene::codecs
{
class DocValuesFormat;
}
namespace org::apache::lucene::codecs
{
class StoredFieldsFormat;
}
namespace org::apache::lucene::codecs
{
class TermVectorsFormat;
}
namespace org::apache::lucene::codecs
{
class FieldInfosFormat;
}
namespace org::apache::lucene::codecs
{
class SegmentInfoFormat;
}
namespace org::apache::lucene::codecs
{
class NormsFormat;
}
namespace org::apache::lucene::codecs
{
class LiveDocsFormat;
}
namespace org::apache::lucene::codecs
{
class CompoundFormat;
}
namespace org::apache::lucene::codecs
{
class PointsFormat;
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

using NamedSPILoader = org::apache::lucene::util::NamedSPILoader;

/**
 * Encodes/decodes an inverted index segment.
 * <p>
 * Note, when extending this class, the name ({@link #getName}) is
 * written into the index. In order for the segment to be read, the
 * name must resolve to your implementation via {@link #forName(std::wstring)}.
 * This method uses Java's
 * {@link ServiceLoader Service Provider Interface} (SPI) to resolve codec
 * names. <p> If you implement your own codec, make sure that it has a no-arg
 * constructor so SPI can load it.
 * @see ServiceLoader
 */
class Codec : public std::enable_shared_from_this<Codec>,
              public NamedSPILoader::NamedSPI
{
  GET_CLASS_NAME(Codec)

  /**
   * This static holder class prevents classloading deadlock by delaying
   * init of default codecs and available codecs until needed.
   */
private:
  class Holder final : public std::enable_shared_from_this<Holder>
  {
    GET_CLASS_NAME(Holder)
  private:
    static const std::shared_ptr<NamedSPILoader<std::shared_ptr<Codec>>> LOADER;

    Holder();

  public:
    static std::shared_ptr<NamedSPILoader<std::shared_ptr<Codec>>> getLoader();

    // TODO: should we use this, or maybe a system property is better?
    static std::shared_ptr<Codec> defaultCodec;
  };

private:
  const std::wstring name;

  /**
   * Creates a new codec.
   * <p>
   * The provided name will be written into the index segment: in order to
   * for the segment to be read this class should be registered with Java's
   * SPI mechanism (registered in META-INF/ of your jar file, etc).
   * @param name must be all ascii alphanumeric, and less than 128 characters in
   * length.
   */
protected:
  Codec(const std::wstring &name);

  /** Returns this codec's name */
public:
  std::wstring getName() override;

  /** Encodes/decodes postings */
  virtual std::shared_ptr<PostingsFormat> postingsFormat() = 0;

  /** Encodes/decodes docvalues */
  virtual std::shared_ptr<DocValuesFormat> docValuesFormat() = 0;

  /** Encodes/decodes stored fields */
  virtual std::shared_ptr<StoredFieldsFormat> storedFieldsFormat() = 0;

  /** Encodes/decodes term vectors */
  virtual std::shared_ptr<TermVectorsFormat> termVectorsFormat() = 0;

  /** Encodes/decodes field infos file */
  virtual std::shared_ptr<FieldInfosFormat> fieldInfosFormat() = 0;

  /** Encodes/decodes segment info file */
  virtual std::shared_ptr<SegmentInfoFormat> segmentInfoFormat() = 0;

  /** Encodes/decodes document normalization values */
  virtual std::shared_ptr<NormsFormat> normsFormat() = 0;

  /** Encodes/decodes live docs */
  virtual std::shared_ptr<LiveDocsFormat> liveDocsFormat() = 0;

  /** Encodes/decodes compound files */
  virtual std::shared_ptr<CompoundFormat> compoundFormat() = 0;

  /** Encodes/decodes points index */
  virtual std::shared_ptr<PointsFormat> pointsFormat() = 0;

  /** looks up a codec by name */
  static std::shared_ptr<Codec> forName(const std::wstring &name);

  /** returns a deque of all available codec names */
  static std::shared_ptr<Set<std::wstring>> availableCodecs();

  /**
   * Reloads the codec deque from the given {@link ClassLoader}.
   * Changes to the codecs are visible after the method ends, all
   * iterators ({@link #availableCodecs()},...) stay consistent.
   *
   * <p><b>NOTE:</b> Only new codecs are added, existing ones are
   * never removed or replaced.
   *
   * <p><em>This method is expensive and should only be called for discovery
   * of new codecs on the given classpath/classloader!</em>
   */
  static void reloadCodecs(std::shared_ptr<ClassLoader> classloader);

  /** expert: returns the default codec used for newly created
   *  {@link IndexWriterConfig}s.
   */
  static std::shared_ptr<Codec> getDefault();

  /** expert: sets the default codec used for newly created
   *  {@link IndexWriterConfig}s.
   */
  static void setDefault(std::shared_ptr<Codec> codec);

  /**
   * returns the codec's name. Subclasses can override to provide
   * more detail (such as parameters).
   */
  virtual std::wstring toString();
};

} // namespace org::apache::lucene::codecs
