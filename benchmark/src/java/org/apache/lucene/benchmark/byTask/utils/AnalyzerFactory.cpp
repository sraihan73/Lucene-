using namespace std;

#include "AnalyzerFactory.h"
#include "../../../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/util/CharFilterFactory.h"
#include "../../../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "../../../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"

namespace org::apache::lucene::benchmark::byTask::utils
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;

AnalyzerFactory::AnalyzerFactory(
    deque<std::shared_ptr<CharFilterFactory>> &charFilterFactories,
    shared_ptr<TokenizerFactory> tokenizerFactory,
    deque<std::shared_ptr<TokenFilterFactory>> &tokenFilterFactories)
    : charFilterFactories(charFilterFactories),
      tokenizerFactory(tokenizerFactory),
      tokenFilterFactories(tokenFilterFactories)
{
  assert(nullptr != tokenizerFactory);
}

void AnalyzerFactory::setName(const wstring &name) { this->name = name; }

void AnalyzerFactory::setPositionIncrementGap(
    optional<int> &positionIncrementGap)
{
  this->positionIncrementGap = positionIncrementGap;
}

void AnalyzerFactory::setOffsetGap(optional<int> &offsetGap)
{
  this->offsetGap = offsetGap;
}

shared_ptr<Analyzer> AnalyzerFactory::create()
{
  return make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

AnalyzerFactory::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<AnalyzerFactory> outerInstance)
{
  this->outerInstance = outerInstance;
  outerInstance->positionIncrementGap = outerInstance->positionIncrementGap;
  outerInstance->offsetGap = outerInstance->offsetGap;
}

shared_ptr<Reader> AnalyzerFactory::AnalyzerAnonymousInnerClass::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  if (outerInstance->charFilterFactories.size() > 0 &&
      outerInstance->charFilterFactories.size() > 0) {
    shared_ptr<Reader> wrappedReader = reader;
    for (auto charFilterFactory : outerInstance->charFilterFactories) {
      wrappedReader = charFilterFactory->create(wrappedReader);
    }
    reader = wrappedReader;
  }
  return reader;
}

shared_ptr<Analyzer::TokenStreamComponents>
AnalyzerFactory::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const tokenizer =
      outerInstance->tokenizerFactory->create();
  shared_ptr<TokenStream> tokenStream = tokenizer;
  for (auto filterFactory : outerInstance->tokenFilterFactories) {
    tokenStream = filterFactory->create(tokenStream);
  }
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenStream);
}

int AnalyzerFactory::AnalyzerAnonymousInnerClass::getPositionIncrementGap(
    const wstring &fieldName)
{
  return nullptr == outerInstance->positionIncrementGap
             ? Analyzer::getPositionIncrementGap(fieldName)
             : outerInstance->positionIncrementGap;
}

int AnalyzerFactory::AnalyzerAnonymousInnerClass::getOffsetGap(
    const wstring &fieldName)
{
  return nullptr == outerInstance->offsetGap ? Analyzer::getOffsetGap(fieldName)
                                             : outerInstance->offsetGap;
}

wstring AnalyzerFactory::toString()
{
  shared_ptr<StringBuilder> sb =
      make_shared<StringBuilder>(L"AnalyzerFactory(");
  if (L"" != name) {
    sb->append(L"name:");
    sb->append(name);
    sb->append(L", ");
  }
  if (nullptr != positionIncrementGap) {
    sb->append(L"positionIncrementGap:");
    sb->append(positionIncrementGap);
    sb->append(L", ");
  }
  if (nullptr != offsetGap) {
    sb->append(L"offsetGap:");
    sb->append(offsetGap);
    sb->append(L", ");
  }
  for (auto charFilterFactory : charFilterFactories) {
    sb->append(charFilterFactory);
    sb->append(L", ");
  }
  sb->append(tokenizerFactory);
  for (auto tokenFilterFactory : tokenFilterFactories) {
    sb->append(L", ");
    sb->append(tokenFilterFactory);
  }
  sb->append(L')');
  return sb->toString();
}
} // namespace org::apache::lucene::benchmark::byTask::utils