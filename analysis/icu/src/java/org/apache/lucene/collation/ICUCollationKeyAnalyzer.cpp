using namespace std;

#include "ICUCollationKeyAnalyzer.h"
#include "../../../../../../../common/src/java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "ICUCollationAttributeFactory.h"

namespace org::apache::lucene::collation
{
using com::ibm::icu::text::Collator;
using Analyzer = org::apache::lucene::analysis::Analyzer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using CollationKeyAnalyzer =
    org::apache::lucene::collation::CollationKeyAnalyzer;

ICUCollationKeyAnalyzer::ICUCollationKeyAnalyzer(shared_ptr<Collator> collator)
    : factory(make_shared<ICUCollationAttributeFactory>(collator))
{
}

shared_ptr<Analyzer::TokenStreamComponents>
ICUCollationKeyAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<KeywordTokenizer> tokenizer = make_shared<KeywordTokenizer>(
      factory, KeywordTokenizer::DEFAULT_BUFFER_SIZE);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}
} // namespace org::apache::lucene::collation