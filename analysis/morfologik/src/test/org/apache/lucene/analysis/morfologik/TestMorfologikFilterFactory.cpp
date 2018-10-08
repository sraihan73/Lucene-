using namespace std;

#include "TestMorfologikFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ClasspathResourceLoader.h"
#include "../../../../../../java/org/apache/lucene/analysis/morfologik/MorfologikFilterFactory.h"

namespace org::apache::lucene::analysis::morfologik
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ClasspathResourceLoader =
    org::apache::lucene::analysis::util::ClasspathResourceLoader;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;

shared_ptr<InputStream>
TestMorfologikFilterFactory::ForbidResourcesLoader::openResource(
    const wstring &resource) 
{
  throw make_shared<UnsupportedOperationException>();
}

template <typename T>
type_info TestMorfologikFilterFactory::ForbidResourcesLoader::findClass(
    const wstring &cname, type_info<T> &expectedType)
{
  throw make_shared<UnsupportedOperationException>();
}

template <typename T>
T TestMorfologikFilterFactory::ForbidResourcesLoader::newInstance(
    const wstring &cname, type_info<T> &expectedType)
{
  throw make_shared<UnsupportedOperationException>();
}

void TestMorfologikFilterFactory::testDefaultDictionary() 
{
  shared_ptr<StringReader> reader = make_shared<StringReader>(L"rowery bilety");
  shared_ptr<MorfologikFilterFactory> factory =
      make_shared<MorfologikFilterFactory>(
          Collections::emptyMap<wstring, wstring>());
  factory->inform(make_shared<ForbidResourcesLoader>());
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = factory->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"rower", L"bilet"});
}

void TestMorfologikFilterFactory::testExplicitDictionary() 
{
  shared_ptr<ResourceLoader> *const loader =
      make_shared<ClasspathResourceLoader>(TestMorfologikFilterFactory::typeid);

  shared_ptr<StringReader> reader =
      make_shared<StringReader>(L"inflected1 inflected2");
  unordered_map<wstring, wstring> params = unordered_map<wstring, wstring>();
  params.emplace(MorfologikFilterFactory::DICTIONARY_ATTRIBUTE,
                 L"custom-dictionary.dict");
  shared_ptr<MorfologikFilterFactory> factory =
      make_shared<MorfologikFilterFactory>(params);
  factory->inform(loader);
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = factory->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"lemma1", L"lemma2"});
}

void TestMorfologikFilterFactory::testMissingDictionary() 
{
  shared_ptr<ResourceLoader> *const loader =
      make_shared<ClasspathResourceLoader>(TestMorfologikFilterFactory::typeid);

  shared_ptr<IOException> expected = expectThrows(IOException::typeid, [&]() {
    unordered_map<wstring, wstring> params = unordered_map<wstring, wstring>();
    params.put(MorfologikFilterFactory::DICTIONARY_ATTRIBUTE,
               L"missing-dictionary-resource.dict");
    shared_ptr<MorfologikFilterFactory> factory =
        make_shared<MorfologikFilterFactory>(params);
    factory->inform(loader);
  });
  assertTrue(expected->getMessage()->contains(L"Resource not found"));
}

void TestMorfologikFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    unordered_map<wstring, wstring> params = unordered_map<wstring, wstring>();
    params.put(L"bogusArg", L"bogusValue");
    make_shared<MorfologikFilterFactory>(params);
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::morfologik