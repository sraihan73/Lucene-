#pragma once
#include "../util/CharFilterFactory.h"
#include "../util/MultiTermAwareComponent.h"
#include "../util/ResourceLoaderAware.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::charfilter
{
class NormalizeCharMap;
}

namespace org::apache::lucene::analysis::util
{
class ResourceLoader;
}
namespace org::apache::lucene::analysis::charfilter
{
class Builder;
}
namespace org::apache::lucene::analysis::util
{
class AbstractAnalysisFactory;
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
namespace org::apache::lucene::analysis::charfilter
{

using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;

/**
 * Factory for {@link MappingCharFilter}.
 * <pre class="prettyprint">
 * &lt;fieldType name="text_map" class="solr.TextField"
 * positionIncrementGap="100"&gt; &lt;analyzer&gt; &lt;charFilter
 * class="solr.MappingCharFilterFactory" mapping="mapping.txt"/&gt;
 *     &lt;tokenizer class="solr.WhitespaceTokenizerFactory"/&gt;
 *   &lt;/analyzer&gt;
 * &lt;/fieldType&gt;</pre>
 *
 * @since Solr 1.4
 */
class MappingCharFilterFactory : public CharFilterFactory,
                                 public ResourceLoaderAware,
                                 public MultiTermAwareComponent
{
  GET_CLASS_NAME(MappingCharFilterFactory)

protected:
  std::shared_ptr<NormalizeCharMap> normMap;

private:
  const std::wstring mapping;

  /** Creates a new MappingCharFilterFactory */
public:
  MappingCharFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

  std::shared_ptr<Reader> create(std::shared_ptr<Reader> input) override;

  // "source" => "target"
  static std::shared_ptr<Pattern> p;

protected:
  virtual void parseRules(std::deque<std::wstring> &rules,
                          std::shared_ptr<NormalizeCharMap::Builder> builder);

public:
  std::deque<wchar_t> out = std::deque<wchar_t>(256);

protected:
  virtual std::wstring parseString(const std::wstring &s);

public:
  std::shared_ptr<AbstractAnalysisFactory> getMultiTermComponent() override;

protected:
  std::shared_ptr<MappingCharFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<MappingCharFilterFactory>(
        org.apache.lucene.analysis.util.CharFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::charfilter
