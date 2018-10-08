using namespace std;

#include "CollationKeyAnalyzer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../analysis/core/KeywordTokenizer.h"
#include "CollationAttributeFactory.h"

namespace org::apache::lucene::collation
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

CollationKeyAnalyzer::CollationKeyAnalyzer(shared_ptr<Collator> collator)
    : factory(make_shared<CollationAttributeFactory>(collator))
{
}

shared_ptr<AttributeFactory>
CollationKeyAnalyzer::attributeFactory(const wstring &fieldName)
{
  return factory;
}

shared_ptr<Analyzer::TokenStreamComponents>
CollationKeyAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<KeywordTokenizer> tokenizer = make_shared<KeywordTokenizer>(
      factory, KeywordTokenizer::DEFAULT_BUFFER_SIZE);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}
} // namespace org::apache::lucene::collation