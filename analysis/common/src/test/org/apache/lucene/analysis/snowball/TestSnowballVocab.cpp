using namespace std;

#include "TestSnowballVocab.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/VocabularyAssert.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/snowball/SnowballFilter.h"

namespace org::apache::lucene::analysis::snowball
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::Slow;
//    import static org.apache.lucene.analysis.VocabularyAssert.*;

void TestSnowballVocab::testStemmers() 
{
  assertCorrectOutput(L"Danish", L"danish");
  assertCorrectOutput(L"Dutch", L"dutch");
  assertCorrectOutput(L"English", L"english");
  assertCorrectOutput(L"Finnish", L"finnish");
  assertCorrectOutput(L"French", L"french");
  assertCorrectOutput(L"German", L"german");
  assertCorrectOutput(L"German2", L"german2");
  assertCorrectOutput(L"Hungarian", L"hungarian");
  assertCorrectOutput(L"Italian", L"italian");
  assertCorrectOutput(L"Kp", L"kraaij_pohlmann");
  assertCorrectOutput(L"Lovins", L"lovins");
  assertCorrectOutput(L"Norwegian", L"norwegian");
  assertCorrectOutput(L"Porter", L"porter");
  assertCorrectOutput(L"Portuguese", L"portuguese");
  assertCorrectOutput(L"Romanian", L"romanian");
  assertCorrectOutput(L"Russian", L"russian");
  assertCorrectOutput(L"Spanish", L"spanish");
  assertCorrectOutput(L"Swedish", L"swedish");
  assertCorrectOutput(L"Turkish", L"turkish");
}

void TestSnowballVocab::assertCorrectOutput(
    const wstring &snowballLanguage,
    const wstring &dataDirectory) 
{
  if (VERBOSE) {
    wcout << L"checking snowball language: " << snowballLanguage << endl;
  }

  shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass>(
      shared_from_this(), snowballLanguage);

  VocabularyAssert::assertVocabulary(
      a, getDataPath(L"TestSnowballVocabData.zip"), dataDirectory + L"/voc.txt",
      dataDirectory + L"/output.txt");
  delete a;
}

TestSnowballVocab::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestSnowballVocab> outerInstance,
    const wstring &snowballLanguage)
{
  this->outerInstance = outerInstance;
  this->snowballLanguage = snowballLanguage;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSnowballVocab::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> t = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      t, make_shared<SnowballFilter>(t, snowballLanguage));
}
} // namespace org::apache::lucene::analysis::snowball