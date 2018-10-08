#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoaderAware.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"

#include  "core/src/java/org/apache/lucene/analysis/phonetic/PhoneticFilter.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::analysis::phonetic
{

using org::apache::commons::codec::Encoder;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Factory for {@link PhoneticFilter}.
 *
 * Create tokens based on phonetic encoders from
 * <a
 * href="http://commons.apache.org/codec/api-release/org/apache/commons/codec/language/package-summary.html">Apache
 * Commons Codec</a>. <p> This takes one required argument, "encoder", and the
 * rest are optional: <dl> <dt>encoder</dt><dd> required, one of
 * "DoubleMetaphone", "Metaphone", "Soundex", "RefinedSoundex", "Caverphone"
 * (v2.0), "ColognePhonetic" or "Nysiis" (case insensitive). If encoder isn't
 * one of these, it'll be resolved as a class name either by itself if it
 * already contains a '.' or otherwise as in the same package as these
 * others.</dd> <dt>inject</dt><dd> (default=true) add tokens to the stream with
 * the offset=0</dd> <dt>maxCodeLength</dt><dd>The maximum length of the
 * phonetic codes, as defined by the encoder. If an encoder doesn't support this
 * then specifying this is an error.</dd>
 * </dl>
 *
 * <pre class="prettyprint">
 * &lt;fieldType name="text_phonetic" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;tokenizer
 * class="solr.WhitespaceTokenizerFactory"/&gt; &lt;filter
 * class="solr.PhoneticFilterFactory" encoder="DoubleMetaphone"
 * inject="true"/&gt; &lt;/analyzer&gt; &lt;/fieldType&gt;</pre>
 *
 * @see PhoneticFilter
 */
class PhoneticFilterFactory : public TokenFilterFactory,
                              public ResourceLoaderAware
{
  GET_CLASS_NAME(PhoneticFilterFactory)
  /** parameter name: either a short name or a full class name */
public:
  static const std::wstring ENCODER;
  /** parameter name: true if encoded tokens should be added as synonyms */
  static const std::wstring INJECT; // bool
  /** parameter name: restricts the length of the phonetic code */
  static const std::wstring MAX_CODE_LENGTH;

private:
  static const std::wstring PACKAGE_CONTAINING_ENCODERS;

  // Effectively constants; uppercase keys
  static const std::unordered_map<std::wstring, std::type_info> registry;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static PhoneticFilterFactory::StaticConstructor staticConstructor;

public:
  const bool inject; // accessed by the test
private:
  const std::wstring name;
  const std::optional<int> maxCodeLength;
  std::type_info clazz = nullptr;
  std::shared_ptr<Method> setMaxCodeLenMethod = nullptr;

  /** Creates a new PhoneticFilterFactory */
public:
  PhoneticFilterFactory(std::unordered_map<std::wstring, std::wstring> &args);

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

private:
  std::type_info resolveEncoder(const std::wstring &name,
                                std::shared_ptr<ResourceLoader> loader);

  /** Must be thread-safe. */
protected:
  virtual std::shared_ptr<Encoder> getEncoder();

public:
  std::shared_ptr<PhoneticFilter>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<PhoneticFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<PhoneticFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/phonetic/
