using namespace std;

#include "TestEmptyTokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TermToBytesRefAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/StringField.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/TextField.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/ValidatingTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/EmptyTokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/KeywordRepeatFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ProtectedTermFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Directory = org::apache::lucene::store::Directory;

void TestEmptyTokenStream::testConsume() 
{
  shared_ptr<TokenStream> ts = make_shared<EmptyTokenStream>();
  ts->reset();
  assertFalse(ts->incrementToken());
  ts->end();
  delete ts;
  // try again with reuse:
  ts->reset();
  assertFalse(ts->incrementToken());
  ts->end();
  delete ts;
}

TestEmptyTokenStream::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<MissingClass> outerInstance,
    shared_ptr<org::apache::lucene::analysis::CharArraySet> protectedTerms)
{
  this->outerInstance = outerInstance;
  this->protectedTerms = protectedTerms;
}

shared_ptr<TokenStreamComponents>
TestEmptyTokenStream::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source = make_shared<ClassicTokenizer>();
  shared_ptr<TokenStream> sink =
      make_shared<ProtectedTermFilter>(protectedTerms, source, [&](any in_) {
        make_shared<ShingleFilter>(in_, 2);
      });
  sink = make_shared<ValidatingTokenFilter>(sink, L"1");
  return make_shared<TokenStreamComponents>(source, sink);
}

TestEmptyTokenStream::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<MissingClass> outerInstance,
        shared_ptr<org::apache::lucene::analysis::CharArraySet> protectedTerms)
{
  this->outerInstance = outerInstance;
  this->protectedTerms = protectedTerms;
}

shared_ptr<TokenStreamComponents>
TestEmptyTokenStream::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> sink = make_shared<ShingleFilter>(source, 3);
  sink = make_shared<ProtectedTermFilter>(protectedTerms, sink, [&](any in_) {
    make_shared<TypeTokenFilter>(in_, Collections::singleton(L"ALL"), true);
  });
  return make_shared<TokenStreamComponents>(source, sink);
}

TestEmptyTokenStream::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(shared_ptr<MissingClass> outerInstance,
                                 int64_t seed)
{
  this->outerInstance = outerInstance;
  this->seed = seed;
}

shared_ptr<TokenStreamComponents>
TestEmptyTokenStream::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source = make_shared<NGramTokenizer>();
  shared_ptr<TokenStream> sink = make_shared<ValidatingTokenFilter>(
      make_shared<KeywordRepeatFilter>(source), L"stage 0");
  sink = make_shared<ValidatingTokenFilter>(sink, L"stage 1");
  sink = make_shared<RandomSkippingFilter>(sink, seed, [&](any in_) {
    make_shared<TypeTokenFilter>(in_, Collections::singleton(L"word"));
  });
  sink = make_shared<ValidatingTokenFilter>(sink, L"last stage");
  return make_shared<TokenStreamComponents>(source, sink);
}

void TestEmptyTokenStream::testConsume2() 
{
  BaseTokenStreamTestCase::assertTokenStreamContents(
      make_shared<EmptyTokenStream>(), std::deque<wstring>(0));
}

void TestEmptyTokenStream::testIndexWriter_LUCENE4656() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(directory, newIndexWriterConfig(nullptr));

  shared_ptr<TokenStream> ts = make_shared<EmptyTokenStream>();
  assertFalse(ts->hasAttribute(TermToBytesRefAttribute::typeid));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  doc->push_back(make_shared<TextField>(L"description", ts));

  // this should not fail because we have no TermToBytesRefAttribute
  writer->addDocument(doc);

  assertEquals(1, writer->numDocs());

  delete writer;
  delete directory;
}
} // namespace org::apache::lucene::analysis::miscellaneous