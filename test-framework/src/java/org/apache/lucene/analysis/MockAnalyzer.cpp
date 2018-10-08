using namespace std;

#include "MockAnalyzer.h"

namespace org::apache::lucene::analysis
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

MockAnalyzer::MockAnalyzer(shared_ptr<Random> random,
                           shared_ptr<CharacterRunAutomaton> runAutomaton,
                           bool lowerCase,
                           shared_ptr<CharacterRunAutomaton> filter)
    : Analyzer(PER_FIELD_REUSE_STRATEGY), runAutomaton(runAutomaton),
      lowerCase(lowerCase), filter(filter),
      random(make_shared<Random>(random->nextLong()))
{
  // TODO: this should be solved in a different way; Random should not be shared
  // (!).
}

MockAnalyzer::MockAnalyzer(shared_ptr<Random> random,
                           shared_ptr<CharacterRunAutomaton> runAutomaton,
                           bool lowerCase)
    : MockAnalyzer(random, runAutomaton, lowerCase,
                   MockTokenFilter::EMPTY_STOPSET)
{
}

MockAnalyzer::MockAnalyzer(shared_ptr<Random> random)
    : MockAnalyzer(random, MockTokenizer::WHITESPACE, true)
{
}

shared_ptr<TokenStreamComponents>
MockAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(runAutomaton, lowerCase, maxTokenLength);
  tokenizer->setEnableChecks(enableChecks);
  shared_ptr<MockTokenFilter> filt =
      make_shared<MockTokenFilter>(tokenizer, filter);
  return make_shared<TokenStreamComponents>(tokenizer,
                                            maybePayload(filt, fieldName));
}

shared_ptr<TokenStream> MockAnalyzer::normalize(const wstring &fieldName,
                                                shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = in_;
  if (lowerCase) {
    result = make_shared<MockLowerCaseFilter>(result);
  }
  return result;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<TokenFilter>
MockAnalyzer::maybePayload(shared_ptr<TokenFilter> stream,
                           const wstring &fieldName)
{
  optional<int> val = previousMappings[fieldName];
  if (!val) {
    val = -1; // no payloads
    if (LuceneTestCase::rarely(random)) {
      switch (random->nextInt(3)) {
      case 0:
        val = -1; // no payloads
        break;
      case 1:
        val = numeric_limits<int>::max(); // variable length payload
        break;
      case 2:
        val = random->nextInt(12); // fixed length payload
        break;
      }
    }
    if (LuceneTestCase::VERBOSE) {
      if (val == numeric_limits<int>::max()) {
        wcout << L"MockAnalyzer: field=" << fieldName
              << L" gets variable length payloads" << endl;
      } else if (val != -1) {
        wcout << L"MockAnalyzer: field=" << fieldName << L" gets fixed length="
              << val << L" payloads" << endl;
      }
    }
    previousMappings.emplace(
        fieldName, val); // save it so we are consistent for this field
  }

  if (val == -1) {
    return stream;
  } else if (val == numeric_limits<int>::max()) {
    return make_shared<MockVariableLengthPayloadFilter>(random, stream);
  } else {
    return make_shared<MockFixedLengthPayloadFilter>(random, stream, val);
  }
}

void MockAnalyzer::setPositionIncrementGap(int positionIncrementGap)
{
  this->positionIncrementGap = positionIncrementGap;
}

int MockAnalyzer::getPositionIncrementGap(const wstring &fieldName)
{
  return positionIncrementGap;
}

void MockAnalyzer::setOffsetGap(int offsetGap) { this->offsetGap = offsetGap; }

int MockAnalyzer::getOffsetGap(const wstring &fieldName)
{
  return !offsetGap ? Analyzer::getOffsetGap(fieldName) : offsetGap;
}

void MockAnalyzer::setEnableChecks(bool enableChecks)
{
  this->enableChecks = enableChecks;
}

void MockAnalyzer::setMaxTokenLength(int length)
{
  this->maxTokenLength = length;
}
} // namespace org::apache::lucene::analysis