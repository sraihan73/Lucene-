using namespace std;

#include "TestPhoneticFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ClasspathResourceLoader.h"
#include "../../../../../../java/org/apache/lucene/analysis/phonetic/PhoneticFilterFactory.h"

namespace org::apache::lucene::analysis::phonetic
{
using org::apache::commons::codec::language::Caverphone2;
using org::apache::commons::codec::language::Metaphone;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using ClasspathResourceLoader =
    org::apache::lucene::analysis::util::ClasspathResourceLoader;

void TestPhoneticFilterFactory::testFactoryDefaults() 
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(PhoneticFilterFactory::ENCODER, L"Metaphone");
  shared_ptr<PhoneticFilterFactory> factory =
      make_shared<PhoneticFilterFactory>(args);
  factory->inform(make_shared<ClasspathResourceLoader>(factory->getClass()));
  assertTrue(std::dynamic_pointer_cast<Metaphone>(factory->getEncoder()) !=
             nullptr);
  assertTrue(factory->inject); // default
}

void TestPhoneticFilterFactory::testInjectFalse() 
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(PhoneticFilterFactory::ENCODER, L"Metaphone");
  args.emplace(PhoneticFilterFactory::INJECT, L"false");
  shared_ptr<PhoneticFilterFactory> factory =
      make_shared<PhoneticFilterFactory>(args);
  factory->inform(make_shared<ClasspathResourceLoader>(factory->getClass()));
  assertFalse(factory->inject);
}

void TestPhoneticFilterFactory::testMaxCodeLength() 
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(PhoneticFilterFactory::ENCODER, L"Metaphone");
  args.emplace(PhoneticFilterFactory::MAX_CODE_LENGTH, L"2");
  shared_ptr<PhoneticFilterFactory> factory =
      make_shared<PhoneticFilterFactory>(args);
  factory->inform(make_shared<ClasspathResourceLoader>(factory->getClass()));
  assertEquals(2, (std::static_pointer_cast<Metaphone>(factory->getEncoder()))
                      ->getMaxCodeLen());
}

void TestPhoneticFilterFactory::testMissingEncoder() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<PhoneticFilterFactory>(unordered_map<wstring, wstring>());
  });
  assertTrue(expected.what()->contains(
      L"Configuration Error: missing parameter 'encoder'"));
}

void TestPhoneticFilterFactory::testUnknownEncoder() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
    args.put(L"encoder", L"XXX");
    shared_ptr<PhoneticFilterFactory> factory =
        make_shared<PhoneticFilterFactory>(args);
    factory->inform(make_shared<ClasspathResourceLoader>(factory->getClass()));
  });
  assertTrue(expected.what()->contains(L"Error loading encoder"));
}

void TestPhoneticFilterFactory::testUnknownEncoderReflection() throw(
    IOException)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
    args.put(L"encoder", L"org.apache.commons.codec.language.NonExistence");
    shared_ptr<PhoneticFilterFactory> factory =
        make_shared<PhoneticFilterFactory>(args);
    factory->inform(make_shared<ClasspathResourceLoader>(factory->getClass()));
  });
  assertTrue(expected.what()->contains(L"Error loading encoder"));
}

void TestPhoneticFilterFactory::testFactoryReflection() 
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(PhoneticFilterFactory::ENCODER,
               L"org.apache.commons.codec.language.Metaphone");
  shared_ptr<PhoneticFilterFactory> factory =
      make_shared<PhoneticFilterFactory>(args);
  factory->inform(make_shared<ClasspathResourceLoader>(factory->getClass()));
  assertTrue(std::dynamic_pointer_cast<Metaphone>(factory->getEncoder()) !=
             nullptr);
  assertTrue(factory->inject); // default
}

void TestPhoneticFilterFactory::testFactoryReflectionCaverphone2() throw(
    IOException)
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(PhoneticFilterFactory::ENCODER, L"Caverphone2");
  shared_ptr<PhoneticFilterFactory> factory =
      make_shared<PhoneticFilterFactory>(args);
  factory->inform(make_shared<ClasspathResourceLoader>(factory->getClass()));
  assertTrue(std::dynamic_pointer_cast<Caverphone2>(factory->getEncoder()) !=
             nullptr);
  assertTrue(factory->inject); // default
}

void TestPhoneticFilterFactory::testFactoryReflectionCaverphone() throw(
    IOException)
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(PhoneticFilterFactory::ENCODER, L"Caverphone");
  shared_ptr<PhoneticFilterFactory> factory =
      make_shared<PhoneticFilterFactory>(args);
  factory->inform(make_shared<ClasspathResourceLoader>(factory->getClass()));
  assertTrue(std::dynamic_pointer_cast<Caverphone2>(factory->getEncoder()) !=
             nullptr);
  assertTrue(factory->inject); // default
}

void TestPhoneticFilterFactory::testAlgorithms() 
{
  assertAlgorithm(L"Metaphone", L"true", L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A", L"aaa", L"B", L"bbb", L"KKK",
                                       L"ccc", L"ESKS", L"easgasg"});
  assertAlgorithm(L"Metaphone", L"false", L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A", L"B", L"KKK", L"ESKS"});

  assertAlgorithm(L"DoubleMetaphone", L"true", L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A", L"aaa", L"PP", L"bbb", L"KK",
                                       L"ccc", L"ASKS", L"easgasg"});
  assertAlgorithm(L"DoubleMetaphone", L"false", L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A", L"PP", L"KK", L"ASKS"});

  assertAlgorithm(L"Soundex", L"true", L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A000", L"aaa", L"B000", L"bbb",
                                       L"C000", L"ccc", L"E220", L"easgasg"});
  assertAlgorithm(L"Soundex", L"false", L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A000", L"B000", L"C000", L"E220"});

  assertAlgorithm(L"RefinedSoundex", L"true", L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A0", L"aaa", L"B1", L"bbb", L"C3",
                                       L"ccc", L"E034034", L"easgasg"});
  assertAlgorithm(L"RefinedSoundex", L"false", L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"A0", L"B1", L"C3", L"E034034"});

  assertAlgorithm(L"Caverphone", L"true", L"Darda Karleen Datha Carlene",
                  std::deque<wstring>{L"TTA1111111", L"Darda", L"KLN1111111",
                                       L"Karleen", L"TTA1111111", L"Datha",
                                       L"KLN1111111", L"Carlene"});
  assertAlgorithm(L"Caverphone", L"false", L"Darda Karleen Datha Carlene",
                  std::deque<wstring>{L"TTA1111111", L"KLN1111111",
                                       L"TTA1111111", L"KLN1111111"});

  assertAlgorithm(L"ColognePhonetic", L"true", L"Meier Schmitt Meir Schmidt",
                  std::deque<wstring>{L"67", L"Meier", L"862", L"Schmitt",
                                       L"67", L"Meir", L"862", L"Schmidt"});
  assertAlgorithm(L"ColognePhonetic", L"false", L"Meier Schmitt Meir Schmidt",
                  std::deque<wstring>{L"67", L"862", L"67", L"862"});

  assertAlgorithm(L"Nysiis", L"true", L"Macintosh Knuth Bart Hurd",
                  std::deque<wstring>{L"MCANT", L"Macintosh", L"NAT", L"Knuth",
                                       L"BAD", L"Bart", L"HAD", L"Hurd"});
  assertAlgorithm(L"Nysiis", L"false", L"Macintosh Knuth Bart Hurd",
                  std::deque<wstring>{L"MCANT", L"NAT", L"BAD", L"HAD"});
}

void TestPhoneticFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<PhoneticFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestPhoneticFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestPhoneticFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"encoder", L"Metaphone");
  this->put(L"bogusArg", L"bogusValue");
}

void TestPhoneticFilterFactory::assertAlgorithm(
    const wstring &algName, const wstring &inject, const wstring &input,
    std::deque<wstring> &expected) 
{
  shared_ptr<Tokenizer> tokenizer = whitespaceMockTokenizer(input);
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"encoder", algName);
  args.emplace(L"inject", inject);
  shared_ptr<PhoneticFilterFactory> factory =
      make_shared<PhoneticFilterFactory>(args);
  factory->inform(make_shared<ClasspathResourceLoader>(factory->getClass()));
  shared_ptr<TokenStream> stream = factory->create(tokenizer);
  assertTokenStreamContents(stream, expected);
}
} // namespace org::apache::lucene::analysis::phonetic