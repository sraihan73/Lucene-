using namespace std;

#include "TestSimpleQueryParser.h"

namespace org::apache::lucene::queryparser::simple
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using LevenshteinAutomata =
    org::apache::lucene::util::automaton::LevenshteinAutomata;
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.AND_OPERATOR;
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.ESCAPE_OPERATOR;
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.FUZZY_OPERATOR;
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.NOT_OPERATOR;
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.OR_OPERATOR; import
//    static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.PHRASE_OPERATOR;
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.PRECEDENCE_OPERATORS;
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.PREFIX_OPERATOR;
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.NEAR_OPERATOR;
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.WHITESPACE_OPERATOR;

shared_ptr<Query> TestSimpleQueryParser::parse(const wstring &text)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<SimpleQueryParser> parser =
      make_shared<SimpleQueryParser>(analyzer, L"field");
  parser->setDefaultOperator(Occur::MUST);
  return parser->parse(text);
}

shared_ptr<Query> TestSimpleQueryParser::parse(const wstring &text, int flags)
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<SimpleQueryParser> parser = make_shared<SimpleQueryParser>(
      analyzer, Collections::singletonMap(L"field", 1.0f), flags);
  parser->setDefaultOperator(Occur::MUST);
  return parser->parse(text);
}

void TestSimpleQueryParser::testTerm() 
{
  shared_ptr<Query> expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"foobar"));

  TestUtil::assertEquals(expected, parse(L"foobar"));
}

void TestSimpleQueryParser::testFuzzy() 
{
  shared_ptr<Query> regular =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"foobar"));
  shared_ptr<Query> expected =
      make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"foobar"), 2);

  TestUtil::assertEquals(expected, parse(L"foobar~2"));
  TestUtil::assertEquals(expected, parse(L"foobar~"));
  TestUtil::assertEquals(regular, parse(L"foobar~a"));
  TestUtil::assertEquals(regular, parse(L"foobar~1a"));

  shared_ptr<BooleanQuery::Builder> bool_ =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<FuzzyQuery> fuzzy =
      make_shared<FuzzyQuery>(make_shared<Term>(L"field", L"foo"),
                              LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE);
  bool_->add(fuzzy, Occur::MUST);
  bool_->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"bar")),
             Occur::MUST);

  TestUtil::assertEquals(
      bool_->build(),
      parse(L"foo~" +
            to_wstring(LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE) +
            to_wstring(1) + L" bar"));
}

void TestSimpleQueryParser::testPhrase() 
{
  shared_ptr<PhraseQuery> expected =
      make_shared<PhraseQuery>(L"field", L"foo", L"bar");

  TestUtil::assertEquals(expected, parse(L"\"foo bar\""));
}

void TestSimpleQueryParser::testPhraseWithSlop() 
{
  shared_ptr<PhraseQuery> expectedWithSlop =
      make_shared<PhraseQuery>(2, L"field", L"foo", L"bar");

  TestUtil::assertEquals(expectedWithSlop, parse(L"\"foo bar\"~2"));

  shared_ptr<PhraseQuery> expectedWithMultiDigitSlop =
      make_shared<PhraseQuery>(10, L"field", L"foo", L"bar");

  TestUtil::assertEquals(expectedWithMultiDigitSlop, parse(L"\"foo bar\"~10"));

  shared_ptr<PhraseQuery> expectedNoSlop =
      make_shared<PhraseQuery>(L"field", L"foo", L"bar");

  assertEquals(L"Ignore trailing tilde with no slop", expectedNoSlop,
               parse(L"\"foo bar\"~"));
  assertEquals(L"Ignore non-numeric trailing slop", expectedNoSlop,
               parse(L"\"foo bar\"~a"));
  assertEquals(L"Ignore non-numeric trailing slop", expectedNoSlop,
               parse(L"\"foo bar\"~1a"));
  assertEquals(L"Ignore negative trailing slop", expectedNoSlop,
               parse(L"\"foo bar\"~-1"));

  shared_ptr<PhraseQuery> pq =
      make_shared<PhraseQuery>(12, L"field", L"foo", L"bar");

  shared_ptr<BooleanQuery::Builder> expectedBoolean =
      make_shared<BooleanQuery::Builder>();
  expectedBoolean->add(pq, Occur::MUST);
  expectedBoolean->add(
      make_shared<TermQuery>(make_shared<Term>(L"field", L"baz")), Occur::MUST);

  TestUtil::assertEquals(expectedBoolean->build(),
                         parse(L"\"foo bar\"~12 baz"));
}

void TestSimpleQueryParser::testPrefix() 
{
  shared_ptr<PrefixQuery> expected =
      make_shared<PrefixQuery>(make_shared<Term>(L"field", L"foobar"));

  TestUtil::assertEquals(expected, parse(L"foobar*"));
}

void TestSimpleQueryParser::testAND() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"foo")),
                Occur::MUST);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"bar")),
                Occur::MUST);

  TestUtil::assertEquals(expected->build(), parse(L"foo+bar"));
}

void TestSimpleQueryParser::testANDPhrase() 
{
  shared_ptr<PhraseQuery> phrase1 =
      make_shared<PhraseQuery>(L"field", L"foo", L"bar");
  shared_ptr<PhraseQuery> phrase2 =
      make_shared<PhraseQuery>(L"field", L"star", L"wars");
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(phrase1, Occur::MUST);
  expected->add(phrase2, Occur::MUST);

  TestUtil::assertEquals(expected->build(),
                         parse(L"\"foo bar\"+\"star wars\""));
}

void TestSimpleQueryParser::testANDImplicit() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"foo")),
                Occur::MUST);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"bar")),
                Occur::MUST);

  TestUtil::assertEquals(expected->build(), parse(L"foo bar"));
}

void TestSimpleQueryParser::testOR() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"foo")),
                Occur::SHOULD);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"bar")),
                Occur::SHOULD);

  TestUtil::assertEquals(expected->build(), parse(L"foo|bar"));
  TestUtil::assertEquals(expected->build(), parse(L"foo||bar"));
}

void TestSimpleQueryParser::testORImplicit() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"foo")),
                Occur::SHOULD);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"bar")),
                Occur::SHOULD);

  shared_ptr<SimpleQueryParser> parser = make_shared<SimpleQueryParser>(
      make_shared<MockAnalyzer>(random()), L"field");
  TestUtil::assertEquals(expected->build(), parser->parse(L"foo bar"));
}

void TestSimpleQueryParser::testORPhrase() 
{
  shared_ptr<PhraseQuery> phrase1 =
      make_shared<PhraseQuery>(L"field", L"foo", L"bar");
  shared_ptr<PhraseQuery> phrase2 =
      make_shared<PhraseQuery>(L"field", L"star", L"wars");
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(phrase1, Occur::SHOULD);
  expected->add(phrase2, Occur::SHOULD);

  TestUtil::assertEquals(expected->build(),
                         parse(L"\"foo bar\"|\"star wars\""));
}

void TestSimpleQueryParser::testNOT() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"foo")),
                Occur::MUST_NOT);
  expected->add(make_shared<MatchAllDocsQuery>(), Occur::SHOULD);

  TestUtil::assertEquals(expected->build(), parse(L"-foo"));
  TestUtil::assertEquals(expected->build(), parse(L"-(foo)"));
  TestUtil::assertEquals(expected->build(), parse(L"---foo"));
}

void TestSimpleQueryParser::testCrazyPrefixes1() 
{
  shared_ptr<Query> expected =
      make_shared<PrefixQuery>(make_shared<Term>(L"field", L"st*ar"));

  TestUtil::assertEquals(expected, parse(L"st*ar*"));
}

void TestSimpleQueryParser::testCrazyPrefixes2() 
{
  shared_ptr<Query> expected =
      make_shared<PrefixQuery>(make_shared<Term>(L"field", L"st*ar\\*"));

  TestUtil::assertEquals(expected, parse(L"st*ar\\\\**"));
}

void TestSimpleQueryParser::testTermInDisguise() 
{
  shared_ptr<Query> expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"st*ar\\*"));

  TestUtil::assertEquals(expected, parse(L"sT*Ar\\\\\\*"));
}

void TestSimpleQueryParser::testGarbageTerm() 
{
  shared_ptr<Query> expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"star"));

  TestUtil::assertEquals(expected, parse(L"star"));
  TestUtil::assertEquals(expected, parse(L"star\n"));
  TestUtil::assertEquals(expected, parse(L"star\r"));
  TestUtil::assertEquals(expected, parse(L"star\t"));
  TestUtil::assertEquals(expected, parse(L"star("));
  TestUtil::assertEquals(expected, parse(L"star)"));
  TestUtil::assertEquals(expected, parse(L"star\""));
  TestUtil::assertEquals(expected, parse(L"\t \r\n\nstar   \n \r \t "));
  TestUtil::assertEquals(expected, parse(L"- + \"\" - star \\"));
}

void TestSimpleQueryParser::testGarbageEmpty() 
{
  shared_ptr<MatchNoDocsQuery> expected = make_shared<MatchNoDocsQuery>();

  TestUtil::assertEquals(expected, parse(L""));
  TestUtil::assertEquals(expected, parse(L"  "));
  TestUtil::assertEquals(expected, parse(L"  "));
  TestUtil::assertEquals(expected, parse(L"\\ "));
  TestUtil::assertEquals(expected, parse(L"\\ \\ "));
  TestUtil::assertEquals(expected, parse(L"\"\""));
  TestUtil::assertEquals(expected, parse(L"\" \""));
  TestUtil::assertEquals(expected, parse(L"\" \"|\" \""));
  TestUtil::assertEquals(expected, parse(L"(\" \"|\" \")"));
  TestUtil::assertEquals(expected, parse(L"\" \" \" \""));
  TestUtil::assertEquals(expected, parse(L"(\" \" \" \")"));
}

void TestSimpleQueryParser::testGarbageAND() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"star")),
                Occur::MUST);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wars")),
                Occur::MUST);

  TestUtil::assertEquals(expected->build(), parse(L"star wars"));
  TestUtil::assertEquals(expected->build(), parse(L"star+wars"));
  TestUtil::assertEquals(expected->build(), parse(L"     star     wars   "));
  TestUtil::assertEquals(expected->build(), parse(L"     star +    wars   "));
  TestUtil::assertEquals(expected->build(),
                         parse(L"  |     star + + |   wars   "));
  TestUtil::assertEquals(expected->build(),
                         parse(L"  |     star + + |   wars   \\"));
}

void TestSimpleQueryParser::testGarbageOR() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"star")),
                Occur::SHOULD);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wars")),
                Occur::SHOULD);

  TestUtil::assertEquals(expected->build(), parse(L"star|wars"));
  TestUtil::assertEquals(expected->build(), parse(L"     star |    wars   "));
  TestUtil::assertEquals(expected->build(),
                         parse(L"  |     star | + |   wars   "));
  TestUtil::assertEquals(expected->build(),
                         parse(L"  +     star | + +   wars   \\"));
}

void TestSimpleQueryParser::testGarbageNOT() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"star")),
                Occur::MUST_NOT);
  expected->add(make_shared<MatchAllDocsQuery>(), Occur::SHOULD);

  TestUtil::assertEquals(expected->build(), parse(L"-star"));
  TestUtil::assertEquals(expected->build(), parse(L"---star"));
  TestUtil::assertEquals(expected->build(), parse(L"- -star -"));
}

void TestSimpleQueryParser::testGarbagePhrase() 
{
  shared_ptr<PhraseQuery> expected =
      make_shared<PhraseQuery>(L"field", L"star", L"wars");

  TestUtil::assertEquals(expected, parse(L"\"star wars\""));
  TestUtil::assertEquals(expected, parse(L"\"star wars\\ \""));
  TestUtil::assertEquals(expected, parse(L"\"\" | \"star wars\""));
  TestUtil::assertEquals(expected,
                         parse(L"          \"star wars\"        \"\"\\"));
}

void TestSimpleQueryParser::testGarbageSubquery() 
{
  shared_ptr<Query> expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"star"));

  TestUtil::assertEquals(expected, parse(L"(star)"));
  TestUtil::assertEquals(expected, parse(L"(star))"));
  TestUtil::assertEquals(expected, parse(L"((star)"));
  TestUtil::assertEquals(expected, parse(L"     -()(star)        \n\n\r     "));
  TestUtil::assertEquals(expected,
                         parse(L"| + - ( + - |      star    \n      ) \n"));
}

void TestSimpleQueryParser::testCompoundAnd() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"star")),
                Occur::MUST);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wars")),
                Occur::MUST);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"empire")),
                Occur::MUST);

  TestUtil::assertEquals(expected->build(), parse(L"star wars empire"));
  TestUtil::assertEquals(expected->build(), parse(L"star+wars + empire"));
  TestUtil::assertEquals(expected->build(),
                         parse(L" | --star wars empire \n\\"));
}

void TestSimpleQueryParser::testCompoundOr() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"star")),
                Occur::SHOULD);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wars")),
                Occur::SHOULD);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"empire")),
                Occur::SHOULD);

  TestUtil::assertEquals(expected->build(), parse(L"star|wars|empire"));
  TestUtil::assertEquals(expected->build(), parse(L"star|wars | empire"));
  TestUtil::assertEquals(expected->build(),
                         parse(L" | --star|wars|empire \n\\"));
}

void TestSimpleQueryParser::testComplex00() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner =
      make_shared<BooleanQuery::Builder>();
  inner->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"star")),
             Occur::SHOULD);
  inner->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wars")),
             Occur::SHOULD);
  expected->add(inner->build(), Occur::MUST);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"empire")),
                Occur::MUST);

  TestUtil::assertEquals(expected->build(), parse(L"star|wars empire"));
  TestUtil::assertEquals(expected->build(), parse(L"star|wars + empire"));
  TestUtil::assertEquals(expected->build(),
                         parse(L"star| + wars + ----empire |"));
}

void TestSimpleQueryParser::testComplex01() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner =
      make_shared<BooleanQuery::Builder>();
  inner->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"star")),
             Occur::MUST);
  inner->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wars")),
             Occur::MUST);
  expected->add(inner->build(), Occur::SHOULD);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"empire")),
                Occur::SHOULD);

  TestUtil::assertEquals(expected->build(), parse(L"star wars | empire"));
  TestUtil::assertEquals(expected->build(), parse(L"star + wars|empire"));
  TestUtil::assertEquals(expected->build(),
                         parse(L"star + | wars | ----empire +"));
}

void TestSimpleQueryParser::testComplex02() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner =
      make_shared<BooleanQuery::Builder>();
  inner->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"star")),
             Occur::MUST);
  inner->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wars")),
             Occur::MUST);
  expected->add(inner->build(), Occur::SHOULD);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"empire")),
                Occur::SHOULD);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"strikes")),
                Occur::SHOULD);

  TestUtil::assertEquals(expected->build(),
                         parse(L"star wars | empire | strikes"));
  TestUtil::assertEquals(expected->build(),
                         parse(L"star + wars|empire | strikes"));
  TestUtil::assertEquals(expected->build(),
                         parse(L"star + | wars | ----empire | + --strikes \\"));
}

void TestSimpleQueryParser::testComplex03() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner2 =
      make_shared<BooleanQuery::Builder>();
  inner2->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"star")),
              Occur::MUST);
  inner2->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wars")),
              Occur::MUST);
  inner->add(inner2->build(), Occur::SHOULD);
  inner->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"empire")),
             Occur::SHOULD);
  inner->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"strikes")),
             Occur::SHOULD);
  expected->add(inner->build(), Occur::MUST);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"back")),
                Occur::MUST);

  TestUtil::assertEquals(expected->build(),
                         parse(L"star wars | empire | strikes back"));
  TestUtil::assertEquals(expected->build(),
                         parse(L"star + wars|empire | strikes + back"));
  TestUtil::assertEquals(
      expected->build(),
      parse(L"star + | wars | ----empire | + --strikes + | --back \\"));
}

void TestSimpleQueryParser::testComplex04() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner2 =
      make_shared<BooleanQuery::Builder>();
  inner->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"star")),
             Occur::MUST);
  inner->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wars")),
             Occur::MUST);
  inner2->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"strikes")),
              Occur::MUST);
  inner2->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"back")),
              Occur::MUST);
  expected->add(inner->build(), Occur::SHOULD);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"empire")),
                Occur::SHOULD);
  expected->add(inner2->build(), Occur::SHOULD);

  TestUtil::assertEquals(expected->build(),
                         parse(L"(star wars) | empire | (strikes back)"));
  TestUtil::assertEquals(expected->build(),
                         parse(L"(star + wars) |empire | (strikes + back)"));
  TestUtil::assertEquals(
      expected->build(),
      parse(L"(star + | wars |) | ----empire | + --(strikes + | --back) \\"));
}

void TestSimpleQueryParser::testComplex05() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner1 =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner2 =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner3 =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner4 =
      make_shared<BooleanQuery::Builder>();

  inner1->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"star")),
              Occur::MUST);
  inner1->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wars")),
              Occur::MUST);

  inner2->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"empire")),
              Occur::SHOULD);

  inner3->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"strikes")),
              Occur::MUST);
  inner3->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"back")),
              Occur::MUST);

  inner4->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"jarjar")),
              Occur::MUST_NOT);
  inner4->add(make_shared<MatchAllDocsQuery>(), Occur::SHOULD);

  inner3->add(inner4->build(), Occur::MUST);
  inner2->add(inner3->build(), Occur::SHOULD);

  expected->add(inner1->build(), Occur::SHOULD);
  expected->add(inner2->build(), Occur::SHOULD);

  TestUtil::assertEquals(
      expected->build(),
      parse(L"(star wars) | (empire | (strikes back -jarjar))"));
  TestUtil::assertEquals(
      expected->build(),
      parse(L"(star + wars) |(empire | (strikes + back -jarjar) () )"));
  TestUtil::assertEquals(expected->build(),
                         parse(L"(star + | wars |) | --(--empire | + "
                               L"--(strikes + | --back + -jarjar) \"\" ) \""));
}

void TestSimpleQueryParser::testComplex06() 
{
  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner1 =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner2 =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> inner3 =
      make_shared<BooleanQuery::Builder>();

  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"star")),
                Occur::MUST);

  inner1->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"wars")),
              Occur::SHOULD);

  inner3->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"empire")),
              Occur::SHOULD);
  inner3->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"strikes")),
              Occur::SHOULD);
  inner2->add(inner3->build(), Occur::MUST);

  inner2->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"back")),
              Occur::MUST);
  inner2->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"jar+|jar")),
              Occur::MUST);
  inner1->add(inner2->build(), Occur::SHOULD);

  expected->add(inner1->build(), Occur::MUST);

  TestUtil::assertEquals(
      expected->build(),
      parse(L"star (wars | (empire | strikes back jar\\+\\|jar))"));
  TestUtil::assertEquals(
      expected->build(),
      parse(L"star + (wars |(empire | strikes + back jar\\+\\|jar) () )"));
  TestUtil::assertEquals(expected->build(),
                         parse(L"star + (| wars | | --(--empire | + --strikes "
                               L"+ | --back + jar\\+\\|jar) \"\" ) \""));
}

void TestSimpleQueryParser::testWeightedTerm() 
{
  unordered_map<wstring, float> weights =
      make_shared<LinkedHashMap<wstring, float>>();
  weights.emplace(L"field0", 5.0f);
  weights.emplace(L"field1", 10.0f);

  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<Query> field0 =
      make_shared<TermQuery>(make_shared<Term>(L"field0", L"foo"));
  field0 = make_shared<BoostQuery>(field0, 5.0f);
  expected->add(field0, Occur::SHOULD);
  shared_ptr<Query> field1 =
      make_shared<TermQuery>(make_shared<Term>(L"field1", L"foo"));
  field1 = make_shared<BoostQuery>(field1, 10.0f);
  expected->add(field1, Occur::SHOULD);

  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<SimpleQueryParser> parser =
      make_shared<SimpleQueryParser>(analyzer, weights);
  TestUtil::assertEquals(expected->build(), parser->parse(L"foo"));
}

void TestSimpleQueryParser::testWeightedOR() 
{
  unordered_map<wstring, float> weights =
      make_shared<LinkedHashMap<wstring, float>>();
  weights.emplace(L"field0", 5.0f);
  weights.emplace(L"field1", 10.0f);

  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery::Builder> foo = make_shared<BooleanQuery::Builder>();
  shared_ptr<Query> field0 =
      make_shared<TermQuery>(make_shared<Term>(L"field0", L"foo"));
  field0 = make_shared<BoostQuery>(field0, 5.0f);
  foo->add(field0, Occur::SHOULD);
  shared_ptr<Query> field1 =
      make_shared<TermQuery>(make_shared<Term>(L"field1", L"foo"));
  field1 = make_shared<BoostQuery>(field1, 10.0f);
  foo->add(field1, Occur::SHOULD);
  expected->add(foo->build(), Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> bar = make_shared<BooleanQuery::Builder>();
  field0 = make_shared<TermQuery>(make_shared<Term>(L"field0", L"bar"));
  field0 = make_shared<BoostQuery>(field0, 5.0f);
  bar->add(field0, Occur::SHOULD);
  field1 = make_shared<TermQuery>(make_shared<Term>(L"field1", L"bar"));
  field1 = make_shared<BoostQuery>(field1, 10.0f);
  bar->add(field1, Occur::SHOULD);
  expected->add(bar->build(), Occur::SHOULD);

  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<SimpleQueryParser> parser =
      make_shared<SimpleQueryParser>(analyzer, weights);
  TestUtil::assertEquals(expected->build(), parser->parse(L"foo|bar"));
}

shared_ptr<Query> TestSimpleQueryParser::parseKeyword(const wstring &text,
                                                      int flags)
{
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false);
  shared_ptr<SimpleQueryParser> parser = make_shared<SimpleQueryParser>(
      analyzer, Collections::singletonMap(L"field", 1.0f), flags);
  return parser->parse(text);
}

void TestSimpleQueryParser::testDisablePhrase()
{
  shared_ptr<Query> expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"\"test\""));
  TestUtil::assertEquals(expected, parseKeyword(L"\"test\"", ~PHRASE_OPERATOR));
}

void TestSimpleQueryParser::testDisablePrefix()
{
  shared_ptr<Query> expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"test*"));
  TestUtil::assertEquals(expected, parseKeyword(L"test*", ~PREFIX_OPERATOR));
}

void TestSimpleQueryParser::testDisableAND()
{
  shared_ptr<Query> expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"foo+bar"));
  TestUtil::assertEquals(expected, parseKeyword(L"foo+bar", ~AND_OPERATOR));
  expected = make_shared<TermQuery>(make_shared<Term>(L"field", L"+foo+bar"));
  TestUtil::assertEquals(expected, parseKeyword(L"+foo+bar", ~AND_OPERATOR));
}

void TestSimpleQueryParser::testDisableOR()
{
  shared_ptr<Query> expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"foo|bar"));
  TestUtil::assertEquals(expected, parseKeyword(L"foo|bar", ~OR_OPERATOR));
  expected = make_shared<TermQuery>(make_shared<Term>(L"field", L"|foo|bar"));
  TestUtil::assertEquals(expected, parseKeyword(L"|foo|bar", ~OR_OPERATOR));
}

void TestSimpleQueryParser::testDisableNOT()
{
  shared_ptr<Query> expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"-foo"));
  TestUtil::assertEquals(expected, parseKeyword(L"-foo", ~NOT_OPERATOR));
}

void TestSimpleQueryParser::testDisablePrecedence()
{
  shared_ptr<Query> expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"(foo)"));
  TestUtil::assertEquals(expected,
                         parseKeyword(L"(foo)", ~PRECEDENCE_OPERATORS));
  expected = make_shared<TermQuery>(make_shared<Term>(L"field", L")foo("));
  TestUtil::assertEquals(expected,
                         parseKeyword(L")foo(", ~PRECEDENCE_OPERATORS));
}

void TestSimpleQueryParser::testDisableEscape()
{
  shared_ptr<Query> expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"foo\\bar"));
  TestUtil::assertEquals(expected, parseKeyword(L"foo\\bar", ~ESCAPE_OPERATOR));
  TestUtil::assertEquals(expected,
                         parseKeyword(L"(foo\\bar)", ~ESCAPE_OPERATOR));
  TestUtil::assertEquals(expected,
                         parseKeyword(L"\"foo\\bar\"", ~ESCAPE_OPERATOR));
}

void TestSimpleQueryParser::testDisableWhitespace()
{
  shared_ptr<Query> expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"foo foo"));
  TestUtil::assertEquals(expected,
                         parseKeyword(L"foo foo", ~WHITESPACE_OPERATOR));
  expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L" foo foo\n "));
  TestUtil::assertEquals(expected,
                         parseKeyword(L" foo foo\n ", ~WHITESPACE_OPERATOR));
  expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"\t\tfoo foo foo"));
  TestUtil::assertEquals(
      expected, parseKeyword(L"\t\tfoo foo foo", ~WHITESPACE_OPERATOR));
}

void TestSimpleQueryParser::testDisableFuzziness()
{
  shared_ptr<Query> expected =
      make_shared<TermQuery>(make_shared<Term>(L"field", L"foo~1"));
  TestUtil::assertEquals(expected, parseKeyword(L"foo~1", ~FUZZY_OPERATOR));
}

void TestSimpleQueryParser::testDisableSlop()
{
  shared_ptr<PhraseQuery> expectedPhrase =
      make_shared<PhraseQuery>(L"field", L"foo", L"bar");

  shared_ptr<BooleanQuery::Builder> expected =
      make_shared<BooleanQuery::Builder>();
  expected->add(expectedPhrase, Occur::MUST);
  expected->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"~2")),
                Occur::MUST);
  TestUtil::assertEquals(expected->build(),
                         parse(L"\"foo bar\"~2", ~NEAR_OPERATOR));
}

void TestSimpleQueryParser::testRandomQueries() 
{
  for (int i = 0; i < 1000; i++) {
    wstring query = TestUtil::randomUnicodeString(random());
    parse(query);                                              // no exception
    parseKeyword(query, TestUtil::nextInt(random(), 0, 1024)); // no exception
  }
}

void TestSimpleQueryParser::testRandomQueries2() 
{
  std::deque<wchar_t> chars = {L'a', L'1', L'|', L'&', L' ',
                                L'(', L')', L'"', L'-', L'~'};
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < 1000; i++) {
    sb->setLength(0);
    int queryLength = random()->nextInt(20);
    for (int j = 0; j < queryLength; j++) {
      sb->append(chars[random()->nextInt(chars.size())]);
    }
    parse(sb->toString()); // no exception
    parseKeyword(sb->toString(),
                 TestUtil::nextInt(random(), 0, 1024)); // no exception
  }
}

void TestSimpleQueryParser::testStarBecomesMatchAll() 
{
  shared_ptr<Query> q = parse(L"*");
  TestUtil::assertEquals(q, make_shared<MatchAllDocsQuery>());
  q = parse(L" *   ");
  TestUtil::assertEquals(q, make_shared<MatchAllDocsQuery>());
}
} // namespace org::apache::lucene::queryparser::simple