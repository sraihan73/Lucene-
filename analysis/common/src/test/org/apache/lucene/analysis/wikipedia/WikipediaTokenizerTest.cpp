using namespace std;

#include "WikipediaTokenizerTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/FlagsAttribute.h"
#include "../../../../../../java/org/apache/lucene/analysis/wikipedia/WikipediaTokenizer.h"

namespace org::apache::lucene::analysis::wikipedia
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;
using namespace org::apache::lucene::analysis::wikipedia;
//    import static org.apache.lucene.analysis.wikipedia.WikipediaTokenizer.*;
const wstring WikipediaTokenizerTest::LINK_PHRASES =
    L"click [[link here again]] click [http://lucene.apache.org here again] "
    L"[[Category:a b c d]]";

void WikipediaTokenizerTest::testSimple() 
{
  wstring text = L"This is a [[Category:foo]]";
  shared_ptr<WikipediaTokenizer> tf = make_shared<WikipediaTokenizer>(
      newAttributeFactory(), WikipediaTokenizer::TOKENS_ONLY,
      Collections::emptySet<wstring>());
  tf->setReader(make_shared<StringReader>(text));
  assertTokenStreamContents(
      tf, std::deque<wstring>{L"This", L"is", L"a", L"foo"},
      std::deque<int>{0, 5, 8, 21}, std::deque<int>{4, 7, 9, 24},
      std::deque<wstring>{L"<ALPHANUM>", L"<ALPHANUM>", L"<ALPHANUM>",
                           WikipediaTokenizer::CATEGORY},
      std::deque<int>{1, 1, 1, 1}, text.length());
}

void WikipediaTokenizerTest::testHandwritten() 
{
  // make sure all tokens are in only one type
  wstring test = wstring(L"[[link]] This is a [[Category:foo]] Category  This "
                         L"is a linked [[:Category:bar none withstanding]] ") +
                 L"Category This is (parens) This is a [[link]]  This is an "
                 L"external URL [http://lucene.apache.org] " +
                 L"Here is ''italics'' and ''more italics'', '''bold''' and "
                 L"'''''five quotes''''' " +
                 L" This is a [[link|display info]]  This is a period.  Here "
                 L"is $3.25 and here is 3.50.  Here's Johnny.  " +
                 L"==heading== ===sub head=== followed by some text  "
                 L"[[Category:blah| ]] " +
                 L"''[[Category:ital_cat]]''  here is some that is ''italics "
                 L"[[Category:foo]] but is never closed." +
                 L"'''same [[Category:foo]] goes for this '''''and2 "
                 L"[[Category:foo]] and this" +
                 L" [http://foo.boo.com/test/test/ Test Test] "
                 L"[http://foo.boo.com/test/test/test.html Test Test]" +
                 L" [http://foo.boo.com/test/test/test.html?g=b&c=d Test Test] "
                 L"<ref>Citation</ref> <sup>martian</sup> <span "
                 L"class=\"glue\">code</span>";

  shared_ptr<WikipediaTokenizer> tf = make_shared<WikipediaTokenizer>(
      newAttributeFactory(), WikipediaTokenizer::TOKENS_ONLY,
      Collections::emptySet<wstring>());
  tf->setReader(make_shared<StringReader>(test));
  assertTokenStreamContents(
      tf,
      std::deque<wstring>{L"link",
                           L"This",
                           L"is",
                           L"a",
                           L"foo",
                           L"Category",
                           L"This",
                           L"is",
                           L"a",
                           L"linked",
                           L"bar",
                           L"none",
                           L"withstanding",
                           L"Category",
                           L"This",
                           L"is",
                           L"parens",
                           L"This",
                           L"is",
                           L"a",
                           L"link",
                           L"This",
                           L"is",
                           L"an",
                           L"external",
                           L"URL",
                           L"http://lucene.apache.org",
                           L"Here",
                           L"is",
                           L"italics",
                           L"and",
                           L"more",
                           L"italics",
                           L"bold",
                           L"and",
                           L"five",
                           L"quotes",
                           L"This",
                           L"is",
                           L"a",
                           L"link",
                           L"display",
                           L"info",
                           L"This",
                           L"is",
                           L"a",
                           L"period",
                           L"Here",
                           L"is",
                           L"3.25",
                           L"and",
                           L"here",
                           L"is",
                           L"3.50",
                           L"Here's",
                           L"Johnny",
                           L"heading",
                           L"sub",
                           L"head",
                           L"followed",
                           L"by",
                           L"some",
                           L"text",
                           L"blah",
                           L"ital",
                           L"cat",
                           L"here",
                           L"is",
                           L"some",
                           L"that",
                           L"is",
                           L"italics",
                           L"foo",
                           L"but",
                           L"is",
                           L"never",
                           L"closed",
                           L"same",
                           L"foo",
                           L"goes",
                           L"for",
                           L"this",
                           L"and2",
                           L"foo",
                           L"and",
                           L"this",
                           L"http://foo.boo.com/test/test/",
                           L"Test",
                           L"Test",
                           L"http://foo.boo.com/test/test/test.html",
                           L"Test",
                           L"Test",
                           L"http://foo.boo.com/test/test/test.html?g=b&c=d",
                           L"Test",
                           L"Test",
                           L"Citation",
                           L"martian",
                           L"code"},
      std::deque<wstring>{WikipediaTokenizer::INTERNAL_LINK,
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           WikipediaTokenizer::CATEGORY,
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           WikipediaTokenizer::CATEGORY,
                           WikipediaTokenizer::CATEGORY,
                           WikipediaTokenizer::CATEGORY,
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           WikipediaTokenizer::INTERNAL_LINK,
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           WikipediaTokenizer::EXTERNAL_LINK_URL,
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           WikipediaTokenizer::ITALICS,
                           L"<ALPHANUM>",
                           WikipediaTokenizer::ITALICS,
                           WikipediaTokenizer::ITALICS,
                           WikipediaTokenizer::BOLD,
                           L"<ALPHANUM>",
                           WikipediaTokenizer::BOLD_ITALICS,
                           WikipediaTokenizer::BOLD_ITALICS,
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           WikipediaTokenizer::INTERNAL_LINK,
                           WikipediaTokenizer::INTERNAL_LINK,
                           WikipediaTokenizer::INTERNAL_LINK,
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<NUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<NUM>",
                           L"<APOSTROPHE>",
                           L"<ALPHANUM>",
                           WikipediaTokenizer::HEADING,
                           WikipediaTokenizer::SUB_HEADING,
                           WikipediaTokenizer::SUB_HEADING,
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           WikipediaTokenizer::CATEGORY,
                           WikipediaTokenizer::CATEGORY,
                           WikipediaTokenizer::CATEGORY,
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           WikipediaTokenizer::ITALICS,
                           WikipediaTokenizer::CATEGORY,
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           WikipediaTokenizer::BOLD,
                           WikipediaTokenizer::CATEGORY,
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           WikipediaTokenizer::BOLD_ITALICS,
                           WikipediaTokenizer::CATEGORY,
                           L"<ALPHANUM>",
                           L"<ALPHANUM>",
                           WikipediaTokenizer::EXTERNAL_LINK_URL,
                           WikipediaTokenizer::EXTERNAL_LINK,
                           WikipediaTokenizer::EXTERNAL_LINK,
                           WikipediaTokenizer::EXTERNAL_LINK_URL,
                           WikipediaTokenizer::EXTERNAL_LINK,
                           WikipediaTokenizer::EXTERNAL_LINK,
                           WikipediaTokenizer::EXTERNAL_LINK_URL,
                           WikipediaTokenizer::EXTERNAL_LINK,
                           WikipediaTokenizer::EXTERNAL_LINK,
                           WikipediaTokenizer::CITATION,
                           L"<ALPHANUM>",
                           L"<ALPHANUM>"});
}

void WikipediaTokenizerTest::testLinkPhrases() 
{
  shared_ptr<WikipediaTokenizer> tf = make_shared<WikipediaTokenizer>(
      newAttributeFactory(), WikipediaTokenizer::TOKENS_ONLY,
      Collections::emptySet<wstring>());
  tf->setReader(make_shared<StringReader>(LINK_PHRASES));
  checkLinkPhrases(tf);
}

void WikipediaTokenizerTest::checkLinkPhrases(
    shared_ptr<WikipediaTokenizer> tf) 
{
  assertTokenStreamContents(
      tf,
      std::deque<wstring>{L"click", L"link", L"here", L"again", L"click",
                           L"http://lucene.apache.org", L"here", L"again", L"a",
                           L"b", L"c", L"d"},
      std::deque<int>{1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1});
}

void WikipediaTokenizerTest::testLinks() 
{
  wstring test = L"[http://lucene.apache.org/java/docs/index.html#news here] "
                 L"[http://lucene.apache.org/java/docs/index.html?b=c here] "
                 L"[https://lucene.apache.org/java/docs/index.html?b=c here]";
  shared_ptr<WikipediaTokenizer> tf = make_shared<WikipediaTokenizer>(
      newAttributeFactory(), WikipediaTokenizer::TOKENS_ONLY,
      Collections::emptySet<wstring>());
  tf->setReader(make_shared<StringReader>(test));
  assertTokenStreamContents(
      tf,
      std::deque<wstring>{
          L"http://lucene.apache.org/java/docs/index.html#news", L"here",
          L"http://lucene.apache.org/java/docs/index.html?b=c", L"here",
          L"https://lucene.apache.org/java/docs/index.html?b=c", L"here"},
      std::deque<wstring>{WikipediaTokenizer::EXTERNAL_LINK_URL,
                           WikipediaTokenizer::EXTERNAL_LINK,
                           WikipediaTokenizer::EXTERNAL_LINK_URL,
                           WikipediaTokenizer::EXTERNAL_LINK,
                           WikipediaTokenizer::EXTERNAL_LINK_URL,
                           WikipediaTokenizer::EXTERNAL_LINK});
}

void WikipediaTokenizerTest::testLucene1133() 
{
  shared_ptr<Set<wstring>> untoks = unordered_set<wstring>();
  untoks->add(WikipediaTokenizer::CATEGORY);
  untoks->add(WikipediaTokenizer::ITALICS);
  // should be exactly the same, regardless of untoks
  shared_ptr<WikipediaTokenizer> tf = make_shared<WikipediaTokenizer>(
      newAttributeFactory(), WikipediaTokenizer::TOKENS_ONLY, untoks);
  tf->setReader(make_shared<StringReader>(LINK_PHRASES));
  checkLinkPhrases(tf);
  wstring test =
      L"[[Category:a b c d]] [[Category:e f g]] [[link here]] [[link there]] "
      L"''italics here'' something ''more italics'' [[Category:h   i   j]]";
  tf = make_shared<WikipediaTokenizer>(WikipediaTokenizer::UNTOKENIZED_ONLY,
                                       untoks);
  tf->setReader(make_shared<StringReader>(test));
  assertTokenStreamContents(
      tf,
      std::deque<wstring>{L"a b c d", L"e f g", L"link", L"here", L"link",
                           L"there", L"italics here", L"something",
                           L"more italics", L"h   i   j"},
      std::deque<int>{11, 32, 42, 47, 56, 61, 71, 86, 98, 124},
      std::deque<int>{18, 37, 46, 51, 60, 66, 83, 95, 110, 133},
      std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
}

void WikipediaTokenizerTest::testBoth() 
{
  shared_ptr<Set<wstring>> untoks = unordered_set<wstring>();
  untoks->add(WikipediaTokenizer::CATEGORY);
  untoks->add(WikipediaTokenizer::ITALICS);
  wstring test =
      L"[[Category:a b c d]] [[Category:e f g]] [[link here]] [[link there]] "
      L"''italics here'' something ''more italics'' [[Category:h   i   j]]";
  // should output all the indivual tokens plus the untokenized tokens as well.
  // Untokenized tokens
  shared_ptr<WikipediaTokenizer> tf = make_shared<WikipediaTokenizer>(
      newAttributeFactory(), WikipediaTokenizer::BOTH, untoks);
  tf->setReader(make_shared<StringReader>(test));
  assertTokenStreamContents(
      tf, std::deque<wstring>{L"a b c d",   L"a",
                               L"b",         L"c",
                               L"d",         L"e f g",
                               L"e",         L"f",
                               L"g",         L"link",
                               L"here",      L"link",
                               L"there",     L"italics here",
                               L"italics",   L"here",
                               L"something", L"more italics",
                               L"more",      L"italics",
                               L"h   i   j", L"h",
                               L"i",         L"j"},
      std::deque<int>{11, 11, 13, 15, 17, 32, 32, 34,  36,  42,  47,  56,
                       61, 71, 71, 79, 86, 98, 98, 103, 124, 124, 128, 132},
      std::deque<int>{18, 12, 14, 16, 18, 37,  33,  35,  37,  46,  51,  60,
                       66, 83, 78, 83, 95, 110, 102, 110, 133, 125, 129, 133},
      std::deque<int>{1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,
                       1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1});

  // now check the flags, TODO: add way to check flags from
  // BaseTokenStreamTestCase?
  tf = make_shared<WikipediaTokenizer>(newAttributeFactory(),
                                       WikipediaTokenizer::BOTH, untoks);
  tf->setReader(make_shared<StringReader>(test));
  std::deque<int> expectedFlags = {WikipediaTokenizer::UNTOKENIZED_TOKEN_FLAG,
                                    0,
                                    0,
                                    0,
                                    0,
                                    WikipediaTokenizer::UNTOKENIZED_TOKEN_FLAG,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    WikipediaTokenizer::UNTOKENIZED_TOKEN_FLAG,
                                    0,
                                    0,
                                    0,
                                    WikipediaTokenizer::UNTOKENIZED_TOKEN_FLAG,
                                    0,
                                    0,
                                    WikipediaTokenizer::UNTOKENIZED_TOKEN_FLAG,
                                    0,
                                    0,
                                    0};
  shared_ptr<FlagsAttribute> flagsAtt =
      tf->addAttribute(FlagsAttribute::typeid);
  tf->reset();
  for (int i = 0; i < expectedFlags.size(); i++) {
    assertTrue(tf->incrementToken());
    assertEquals(L"flags " + to_wstring(i), expectedFlags[i],
                 flagsAtt->getFlags());
  }
  assertFalse(tf->incrementToken());
  delete tf;
}

void WikipediaTokenizerTest::testRandomStrings() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  // TODO: properly support positionLengthAttribute
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER, 20, false, false);
  delete a;
}

WikipediaTokenizerTest::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<WikipediaTokenizerTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
WikipediaTokenizerTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<WikipediaTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(),
      WikipediaTokenizer::TOKENS_ONLY, Collections::emptySet<wstring>());
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void WikipediaTokenizerTest::testRandomHugeStrings() 
{
  shared_ptr<Random> random = WikipediaTokenizerTest::random();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  // TODO: properly support positionLengthAttribute
  checkRandomData(random, a, 100 * RANDOM_MULTIPLIER, 8192, false, false);
  delete a;
}

WikipediaTokenizerTest::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<WikipediaTokenizerTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
WikipediaTokenizerTest::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<WikipediaTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(),
      WikipediaTokenizer::TOKENS_ONLY, Collections::emptySet<wstring>());
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}
} // namespace org::apache::lucene::analysis::wikipedia