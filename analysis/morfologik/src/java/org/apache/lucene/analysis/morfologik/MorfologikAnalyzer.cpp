using namespace std;

#include "MorfologikAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "MorfologikFilter.h"

namespace org::apache::lucene::analysis::morfologik
{
using morfologik::stemming::Dictionary;
using morfologik::stemming::polish::PolishStemmer;
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;

MorfologikAnalyzer::MorfologikAnalyzer(shared_ptr<Dictionary> dictionary)
    : dictionary(dictionary)
{
}

MorfologikAnalyzer::MorfologikAnalyzer()
    : MorfologikAnalyzer(new PolishStemmer().getDictionary())
{
}

shared_ptr<Analyzer::TokenStreamComponents>
MorfologikAnalyzer::createComponents(const wstring &field)
{
  shared_ptr<Tokenizer> *const src = make_shared<StandardTokenizer>();

  return make_shared<Analyzer::TokenStreamComponents>(
      src, make_shared<MorfologikFilter>(make_shared<StandardFilter>(src),
                                         dictionary));
}

shared_ptr<TokenStream>
MorfologikAnalyzer::normalize(const wstring &fieldName,
                              shared_ptr<TokenStream> in_)
{
  return make_shared<StandardFilter>(in_);
}
} // namespace org::apache::lucene::analysis::morfologik