using namespace std;

#include "TestPatternTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/MappingCharFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/NormalizeCharMap.h"
#include "../../../../../../java/org/apache/lucene/analysis/pattern/PatternTokenizer.h"

namespace org::apache::lucene::analysis::pattern
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharFilter = org::apache::lucene::analysis::CharFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using MappingCharFilter =
    org::apache::lucene::analysis::charfilter::MappingCharFilter;
using NormalizeCharMap =
    org::apache::lucene::analysis::charfilter::NormalizeCharMap;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

void TestPatternTokenizer::testSplitting() 
{
  wstring qpattern = L"\\'([^\\']+)\\'"; // get stuff between "'"
  std::deque<std::deque<wstring>> tests = {
      std::deque<wstring>{L"-1", L"--", L"aaa--bbb--ccc", L"aaa bbb ccc"},
      std::deque<wstring>{L"-1", L":", L"aaa:bbb:ccc", L"aaa bbb ccc"},
      std::deque<wstring>{L"-1", L"\\p{Space}", L"aaa   bbb \t\tccc  ",
                           L"aaa bbb ccc"},
      std::deque<wstring>{L"-1", L":", L"boo:and:foo", L"boo and foo"},
      std::deque<wstring>{L"-1", L"o", L"boo:and:foo", L"b :and:f"},
      std::deque<wstring>{L"0", L":", L"boo:and:foo", L": :"},
      std::deque<wstring>{L"0", qpattern, L"aaa 'bbb' 'ccc'", L"'bbb' 'ccc'"},
      std::deque<wstring>{L"1", qpattern, L"aaa 'bbb' 'ccc'", L"bbb ccc"}};

  for (auto test : tests) {
    shared_ptr<TokenStream> stream = make_shared<PatternTokenizer>(
        newAttributeFactory(), Pattern::compile(test[1]), stoi(test[0]));
    (std::static_pointer_cast<Tokenizer>(stream))
        ->setReader(make_shared<StringReader>(test[2]));
    wstring out = tsToString(stream);
    // System.out.println( test[2] + " ==> " + out );

    assertEquals(L"pattern: " + test[1] + L" with input: " + test[2], test[3],
                 out);

    // Make sure it is the same as if we called 'split'
    // test disabled, as we remove empty tokens
    /*if( "-1".equals( test[0] ) ) {
      std::wstring[] split = test[2].split( test[1] );
      stream = tokenizer.create( new StringReader( test[2] ) );
      int i=0;
      for( Token t = stream.next(); null != t; t = stream.next() )
      {
        assertEquals( "split: "+test[1] + " "+i, split[i++], new
    std::wstring(t.termBuffer(), 0, t.termLength()) );
      }
    }*/
  }
}

void TestPatternTokenizer::testOffsetCorrection() 
{
  const wstring INPUT = L"G&uuml;nther G&uuml;nther is here";

  // create MappingCharFilter
  deque<wstring> mappingRules = deque<wstring>();
  mappingRules.push_back(L"\"&uuml;\" => \"ü\"");
  shared_ptr<NormalizeCharMap::Builder> builder =
      make_shared<NormalizeCharMap::Builder>();
  builder->add(L"&uuml;", L"ü");
  shared_ptr<NormalizeCharMap> normMap = builder->build();
  shared_ptr<CharFilter> charStream =
      make_shared<MappingCharFilter>(normMap, make_shared<StringReader>(INPUT));

  // create PatternTokenizer
  shared_ptr<Tokenizer> stream = make_shared<PatternTokenizer>(
      newAttributeFactory(), Pattern::compile(L"[,;/\\s]+"), -1);
  stream->setReader(charStream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"Günther", L"Günther", L"is", L"here"},
      std::deque<int>{0, 13, 26, 29}, std::deque<int>{12, 25, 28, 33},
      INPUT.length());

  charStream =
      make_shared<MappingCharFilter>(normMap, make_shared<StringReader>(INPUT));
  stream = make_shared<PatternTokenizer>(newAttributeFactory(),
                                         Pattern::compile(L"Günther"), 0);
  stream->setReader(charStream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"Günther", L"Günther"},
      std::deque<int>{0, 13}, std::deque<int>{12, 25}, INPUT.length());
}

wstring
TestPatternTokenizer::tsToString(shared_ptr<TokenStream> in_) 
{
  shared_ptr<StringBuilder> out = make_shared<StringBuilder>();
  shared_ptr<CharTermAttribute> termAtt =
      in_->addAttribute(CharTermAttribute::typeid);
  // extra safety to enforce, that the state is not preserved and also
  // assign bogus values
  in_->clearAttributes();
  termAtt->setEmpty()->append(L"bogusTerm");
  in_->reset();
  while (in_->incrementToken()) {
    if (out->length() > 0) {
      out->append(L' ');
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    out->append(termAtt->toString());
    in_->clearAttributes();
    termAtt->setEmpty()->append(L"bogusTerm");
  }

  delete in_;
  return out->toString();
}

void TestPatternTokenizer::testRandomStrings() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;

  shared_ptr<Analyzer> b =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkRandomData(random(), b, 1000 * RANDOM_MULTIPLIER);
  delete b;
}

TestPatternTokenizer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestPatternTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPatternTokenizer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<PatternTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), Pattern::compile(L"a"),
      -1);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
}

TestPatternTokenizer::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(shared_ptr<TestPatternTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPatternTokenizer::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<PatternTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), Pattern::compile(L"a"),
      0);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
}

void TestPatternTokenizer::testHeapFreedAfterClose() 
{
  // TODO: can we move this to BaseTSTC to catch other "hangs onto heap"ers?

  // Build a 1MB string:
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  for (int i = 0; i < 1024; i++) {
    // 1023 spaces, then an x
    for (int j = 0; j < 1023; j++) {
      b->append(L' ');
    }
    b->append(L'x');
  }

  wstring big = b->toString();

  shared_ptr<Pattern> x = Pattern::compile(L"x");

  deque<std::shared_ptr<Tokenizer>> tokenizers =
      deque<std::shared_ptr<Tokenizer>>();
  for (int i = 0; i < 512; i++) {
    shared_ptr<Tokenizer> stream = make_shared<PatternTokenizer>(x, -1);
    tokenizers.push_back(stream);
    stream->setReader(make_shared<StringReader>(big));
    stream->reset();
    for (int j = 0; j < 1024; j++) {
      assertTrue(stream->incrementToken());
    }
    assertFalse(stream->incrementToken());
    stream->end();
    delete stream;
  }
}
} // namespace org::apache::lucene::analysis::pattern