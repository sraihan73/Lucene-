#pragma once
#include "../util/TokenizerFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Tokenizer.h"

#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"

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
namespace org::apache::lucene::analysis::path
{

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Factory for {@link PathHierarchyTokenizer}.
 * <p>
 * This factory is typically configured for use only in the <code>index</code>
 * Analyzer (or only in the <code>query</code> Analyzer, but never both).
 * </p>
 * <p>
 * For example, in the configuration below a query for
 * <code>Books/NonFic</code> will match documents indexed with values like
 * <code>Books/NonFic</code>, <code>Books/NonFic/Law</code>,
 * <code>Books/NonFic/Science/Physics</code>, etc. But it will not match
 * documents indexed with values like <code>Books</code>, or
 * <code>Books/Fic</code>...
 * </p>
 *
 * <pre class="prettyprint">
 * &lt;fieldType name="descendent_path" class="solr.TextField"&gt;
 *   &lt;analyzer type="index"&gt;
 *     &lt;tokenizer class="solr.PathHierarchyTokenizerFactory" delimiter="/"
 * /&gt; &lt;/analyzer&gt; &lt;analyzer type="query"&gt; &lt;tokenizer
 * class="solr.KeywordTokenizerFactory" /&gt; &lt;/analyzer&gt;
 * &lt;/fieldType&gt;
 * </pre>
 * <p>
 * In this example however we see the oposite configuration, so that a query
 * for <code>Books/NonFic/Science/Physics</code> would match documents
 * containing <code>Books/NonFic</code>, <code>Books/NonFic/Science</code>,
 * or <code>Books/NonFic/Science/Physics</code>, but not
 * <code>Books/NonFic/Science/Physics/Theory</code> or
 * <code>Books/NonFic/Law</code>.
 * </p>
 * <pre class="prettyprint">
 * &lt;fieldType name="descendent_path" class="solr.TextField"&gt;
 *   &lt;analyzer type="index"&gt;
 *     &lt;tokenizer class="solr.KeywordTokenizerFactory" /&gt;
 *   &lt;/analyzer&gt;
 *   &lt;analyzer type="query"&gt;
 *     &lt;tokenizer class="solr.PathHierarchyTokenizerFactory" delimiter="/"
 * /&gt; &lt;/analyzer&gt; &lt;/fieldType&gt;
 * </pre>
 */
class PathHierarchyTokenizerFactory : public TokenizerFactory
{
  GET_CLASS_NAME(PathHierarchyTokenizerFactory)
private:
  const wchar_t delimiter;
  const wchar_t replacement;
  const bool reverse;
  const int skip;

  /** Creates a new PathHierarchyTokenizerFactory */
public:
  PathHierarchyTokenizerFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<Tokenizer>
  create(std::shared_ptr<AttributeFactory> factory) override;

protected:
  std::shared_ptr<PathHierarchyTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<PathHierarchyTokenizerFactory>(
        org.apache.lucene.analysis.util.TokenizerFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/path/
