using namespace std;

#include "TestDictionary.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/CharSequenceOutputs.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Outputs.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"
#include "../../../../../../java/org/apache/lucene/analysis/hunspell/Dictionary.h"

namespace org::apache::lucene::analysis::hunspell
{
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRef = org::apache::lucene::util::CharsRef;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Builder = org::apache::lucene::util::fst::Builder;
using CharSequenceOutputs = org::apache::lucene::util::fst::CharSequenceOutputs;
using FST = org::apache::lucene::util::fst::FST;
using Outputs = org::apache::lucene::util::fst::Outputs;
using Util = org::apache::lucene::util::fst::Util;

void TestDictionary::testSimpleDictionary() 
{
  shared_ptr<InputStream> affixStream =
      getClass().getResourceAsStream(L"simple.aff");
  shared_ptr<InputStream> dictStream =
      getClass().getResourceAsStream(L"simple.dic");
  shared_ptr<Directory> tempDir = getDirectory();

  shared_ptr<Dictionary> dictionary =
      make_shared<Dictionary>(tempDir, L"dictionary", affixStream, dictStream);
  assertEquals(
      3, dictionary->lookupSuffix(std::deque<wchar_t>{L'e'}, 0, 1)->length);
  assertEquals(
      1, dictionary->lookupPrefix(std::deque<wchar_t>{L's'}, 0, 1)->length);
  shared_ptr<IntsRef> ordList =
      dictionary->lookupWord(std::deque<wchar_t>{L'o', L'l', L'r'}, 0, 3);
  assertNotNull(ordList);
  assertEquals(1, ordList->length);

  shared_ptr<BytesRef> ref = make_shared<BytesRef>();
  dictionary->flagLookup->get(ordList->ints[0], ref);
  std::deque<wchar_t> flags = Dictionary::decodeFlags(ref);
  assertEquals(1, flags.size());

  ordList = dictionary->lookupWord(
      std::deque<wchar_t>{L'l', L'u', L'c', L'e', L'n'}, 0, 5);
  assertNotNull(ordList);
  assertEquals(1, ordList->length);
  dictionary->flagLookup->get(ordList->ints[0], ref);
  flags = Dictionary::decodeFlags(ref);
  assertEquals(1, flags.size());

  affixStream->close();
  dictStream->close();
  delete tempDir;
}

void TestDictionary::testCompressedDictionary() 
{
  shared_ptr<InputStream> affixStream =
      getClass().getResourceAsStream(L"compressed.aff");
  shared_ptr<InputStream> dictStream =
      getClass().getResourceAsStream(L"compressed.dic");

  shared_ptr<Directory> tempDir = getDirectory();
  shared_ptr<Dictionary> dictionary =
      make_shared<Dictionary>(tempDir, L"dictionary", affixStream, dictStream);
  assertEquals(
      3, dictionary->lookupSuffix(std::deque<wchar_t>{L'e'}, 0, 1)->length);
  assertEquals(
      1, dictionary->lookupPrefix(std::deque<wchar_t>{L's'}, 0, 1)->length);
  shared_ptr<IntsRef> ordList =
      dictionary->lookupWord(std::deque<wchar_t>{L'o', L'l', L'r'}, 0, 3);
  shared_ptr<BytesRef> ref = make_shared<BytesRef>();
  dictionary->flagLookup->get(ordList->ints[0], ref);
  std::deque<wchar_t> flags = Dictionary::decodeFlags(ref);
  assertEquals(1, flags.size());

  affixStream->close();
  dictStream->close();
  delete tempDir;
}

void TestDictionary::testCompressedBeforeSetDictionary() 
{
  shared_ptr<InputStream> affixStream =
      getClass().getResourceAsStream(L"compressed-before-set.aff");
  shared_ptr<InputStream> dictStream =
      getClass().getResourceAsStream(L"compressed.dic");
  shared_ptr<Directory> tempDir = getDirectory();

  shared_ptr<Dictionary> dictionary =
      make_shared<Dictionary>(tempDir, L"dictionary", affixStream, dictStream);
  assertEquals(
      3, dictionary->lookupSuffix(std::deque<wchar_t>{L'e'}, 0, 1)->length);
  assertEquals(
      1, dictionary->lookupPrefix(std::deque<wchar_t>{L's'}, 0, 1)->length);
  shared_ptr<IntsRef> ordList =
      dictionary->lookupWord(std::deque<wchar_t>{L'o', L'l', L'r'}, 0, 3);
  shared_ptr<BytesRef> ref = make_shared<BytesRef>();
  dictionary->flagLookup->get(ordList->ints[0], ref);
  std::deque<wchar_t> flags = Dictionary::decodeFlags(ref);
  assertEquals(1, flags.size());

  affixStream->close();
  dictStream->close();
  delete tempDir;
}

void TestDictionary::testCompressedEmptyAliasDictionary() 
{
  shared_ptr<InputStream> affixStream =
      getClass().getResourceAsStream(L"compressed-empty-alias.aff");
  shared_ptr<InputStream> dictStream =
      getClass().getResourceAsStream(L"compressed.dic");
  shared_ptr<Directory> tempDir = getDirectory();

  shared_ptr<Dictionary> dictionary =
      make_shared<Dictionary>(tempDir, L"dictionary", affixStream, dictStream);
  assertEquals(
      3, dictionary->lookupSuffix(std::deque<wchar_t>{L'e'}, 0, 1)->length);
  assertEquals(
      1, dictionary->lookupPrefix(std::deque<wchar_t>{L's'}, 0, 1)->length);
  shared_ptr<IntsRef> ordList =
      dictionary->lookupWord(std::deque<wchar_t>{L'o', L'l', L'r'}, 0, 3);
  shared_ptr<BytesRef> ref = make_shared<BytesRef>();
  dictionary->flagLookup->get(ordList->ints[0], ref);
  std::deque<wchar_t> flags = Dictionary::decodeFlags(ref);
  assertEquals(1, flags.size());

  affixStream->close();
  dictStream->close();
  delete tempDir;
}

void TestDictionary::testInvalidData() 
{
  shared_ptr<InputStream> affixStream =
      getClass().getResourceAsStream(L"broken.aff");
  shared_ptr<InputStream> dictStream =
      getClass().getResourceAsStream(L"simple.dic");
  shared_ptr<Directory> tempDir = getDirectory();

  shared_ptr<ParseException> expected =
      expectThrows(ParseException::typeid, [&]() {
        make_shared<Dictionary>(tempDir, L"dictionary", affixStream,
                                dictStream);
      });
  assertTrue(expected->getMessage()->startsWith(
      L"The affix file contains a rule with less than four elements"));
  assertEquals(24, expected->getErrorOffset());

  affixStream->close();
  dictStream->close();
  delete tempDir;
}

void TestDictionary::testInvalidFlags() 
{
  shared_ptr<InputStream> affixStream =
      getClass().getResourceAsStream(L"broken-flags.aff");
  shared_ptr<InputStream> dictStream =
      getClass().getResourceAsStream(L"simple.dic");
  shared_ptr<Directory> tempDir = getDirectory();

  runtime_error expected = expectThrows(runtime_error::typeid, [&]() {
    make_shared<Dictionary>(tempDir, L"dictionary", affixStream, dictStream);
  });
  assertTrue(expected.what()->startsWith(L"expected only one flag"));

  affixStream->close();
  dictStream->close();
  delete tempDir;
}

TestDictionary::CloseCheckInputStream::CloseCheckInputStream(
    shared_ptr<InputStream> delegate_)
    : java::io::FilterInputStream(delegate_)
{
}

TestDictionary::CloseCheckInputStream::~CloseCheckInputStream()
{
  this->closed = true;
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
}

bool TestDictionary::CloseCheckInputStream::isClosed() { return this->closed; }

void TestDictionary::testResourceCleanup() 
{
  shared_ptr<CloseCheckInputStream> affixStream =
      make_shared<CloseCheckInputStream>(
          getClass().getResourceAsStream(L"compressed.aff"));
  shared_ptr<CloseCheckInputStream> dictStream =
      make_shared<CloseCheckInputStream>(
          getClass().getResourceAsStream(L"compressed.dic"));
  shared_ptr<Directory> tempDir = getDirectory();

  make_shared<Dictionary>(tempDir, L"dictionary", affixStream, dictStream);

  assertFalse(affixStream->isClosed());
  assertFalse(dictStream->isClosed());

  affixStream->close();
  dictStream->close();
  delete tempDir;

  assertTrue(affixStream->isClosed());
  assertTrue(dictStream->isClosed());
}

void TestDictionary::testReplacements() 
{
  shared_ptr<Outputs<std::shared_ptr<CharsRef>>> outputs =
      CharSequenceOutputs::getSingleton();
  shared_ptr<Builder<std::shared_ptr<CharsRef>>> builder =
      make_shared<Builder<std::shared_ptr<CharsRef>>>(FST::INPUT_TYPE::BYTE2,
                                                      outputs);
  shared_ptr<IntsRefBuilder> scratchInts = make_shared<IntsRefBuilder>();

  // a -> b
  Util::toUTF16(L"a", scratchInts);
  builder->add(scratchInts->get(), make_shared<CharsRef>(L"b"));

  // ab -> c
  Util::toUTF16(L"ab", scratchInts);
  builder->add(scratchInts->get(), make_shared<CharsRef>(L"c"));

  // c -> de
  Util::toUTF16(L"c", scratchInts);
  builder->add(scratchInts->get(), make_shared<CharsRef>(L"de"));

  // def -> gh
  Util::toUTF16(L"def", scratchInts);
  builder->add(scratchInts->get(), make_shared<CharsRef>(L"gh"));

  shared_ptr<FST<std::shared_ptr<CharsRef>>> fst = builder->finish();

  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>(L"atestanother");
  Dictionary::applyMappings(fst, sb);
  assertEquals(L"btestbnother", sb->toString());

  sb = make_shared<StringBuilder>(L"abtestanother");
  Dictionary::applyMappings(fst, sb);
  assertEquals(L"ctestbnother", sb->toString());

  sb = make_shared<StringBuilder>(L"atestabnother");
  Dictionary::applyMappings(fst, sb);
  assertEquals(L"btestcnother", sb->toString());

  sb = make_shared<StringBuilder>(L"abtestabnother");
  Dictionary::applyMappings(fst, sb);
  assertEquals(L"ctestcnother", sb->toString());

  sb = make_shared<StringBuilder>(L"abtestabcnother");
  Dictionary::applyMappings(fst, sb);
  assertEquals(L"ctestcdenother", sb->toString());

  sb = make_shared<StringBuilder>(L"defdefdefc");
  Dictionary::applyMappings(fst, sb);
  assertEquals(L"ghghghde", sb->toString());
}

void TestDictionary::testSetWithCrazyWhitespaceAndBOMs() 
{
  assertEquals(
      L"UTF-8",
      Dictionary::getDictionaryEncoding(make_shared<ByteArrayInputStream>(
          (wstring(L"SET\tUTF-8\n")).getBytes(StandardCharsets::UTF_8))));
  assertEquals(
      L"UTF-8",
      Dictionary::getDictionaryEncoding(make_shared<ByteArrayInputStream>(
          (wstring(L"SET\t UTF-8\n")).getBytes(StandardCharsets::UTF_8))));
  assertEquals(
      L"UTF-8",
      Dictionary::getDictionaryEncoding(make_shared<ByteArrayInputStream>(
          (wstring(L"\uFEFFSET\tUTF-8\n")).getBytes(StandardCharsets::UTF_8))));
  assertEquals(L"UTF-8", Dictionary::getDictionaryEncoding(
                             make_shared<ByteArrayInputStream>(
                                 (wstring(L"\uFEFFSET\tUTF-8\r\n"))
                                     .getBytes(StandardCharsets::UTF_8))));
}

void TestDictionary::testFlagWithCrazyWhitespace() 
{
  assertNotNull(Dictionary::getFlagParsingStrategy(L"FLAG\tUTF-8"));
  assertNotNull(Dictionary::getFlagParsingStrategy(L"FLAG    UTF-8"));
}

shared_ptr<Directory> TestDictionary::getDirectory() { return newDirectory(); }
} // namespace org::apache::lucene::analysis::hunspell