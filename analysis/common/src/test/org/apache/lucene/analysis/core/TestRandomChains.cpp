using namespace std;

#include "TestRandomChains.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CachingTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CrankyTokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/ValidatingTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/cjk/CJKBigramFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/commongrams/CommonGramsFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/commongrams/CommonGramsQueryFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/FlattenGraphFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/LowerCaseFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/StopFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/minhash/MinHashFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ConcatenateGraphFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ConditionalTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/DelimitedTermFrequencyTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/FingerprintFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/HyphenatedWordsFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/LimitTokenCountFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/LimitTokenOffsetFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/LimitTokenPositionFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/WordDelimiterFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/WordDelimiterGraphFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/path/PathHierarchyTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/path/ReversePathHierarchyTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/shingle/ShingleFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/wikipedia/WikipediaTokenizer.h"

namespace org::apache::lucene::analysis::core
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CachingTokenFilter = org::apache::lucene::analysis::CachingTokenFilter;
using CharArrayMap = org::apache::lucene::analysis::CharArrayMap;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using CharFilter = org::apache::lucene::analysis::CharFilter;
using CrankyTokenFilter = org::apache::lucene::analysis::CrankyTokenFilter;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using ValidatingTokenFilter =
    org::apache::lucene::analysis::ValidatingTokenFilter;
using NormalizeCharMap =
    org::apache::lucene::analysis::charfilter::NormalizeCharMap;
using CJKBigramFilter = org::apache::lucene::analysis::cjk::CJKBigramFilter;
using CommonGramsFilter =
    org::apache::lucene::analysis::commongrams::CommonGramsFilter;
using CommonGramsQueryFilter =
    org::apache::lucene::analysis::commongrams::CommonGramsQueryFilter;
using HyphenationCompoundWordTokenFilter =
    org::apache::lucene::analysis::compound::HyphenationCompoundWordTokenFilter;
using TestCompoundWordTokenFilter =
    org::apache::lucene::analysis::compound::TestCompoundWordTokenFilter;
using HyphenationTree =
    org::apache::lucene::analysis::compound::hyphenation::HyphenationTree;
using Dictionary = org::apache::lucene::analysis::hunspell::Dictionary;
using TestHunspellStemFilter =
    org::apache::lucene::analysis::hunspell::TestHunspellStemFilter;
using MinHashFilter = org::apache::lucene::analysis::minhash::MinHashFilter;
using ConcatenateGraphFilter =
    org::apache::lucene::analysis::miscellaneous::ConcatenateGraphFilter;
using ConditionalTokenFilter =
    org::apache::lucene::analysis::miscellaneous::ConditionalTokenFilter;
using DelimitedTermFrequencyTokenFilter = org::apache::lucene::analysis::
    miscellaneous::DelimitedTermFrequencyTokenFilter;
using FingerprintFilter =
    org::apache::lucene::analysis::miscellaneous::FingerprintFilter;
using HyphenatedWordsFilter =
    org::apache::lucene::analysis::miscellaneous::HyphenatedWordsFilter;
using LimitTokenCountFilter =
    org::apache::lucene::analysis::miscellaneous::LimitTokenCountFilter;
using LimitTokenOffsetFilter =
    org::apache::lucene::analysis::miscellaneous::LimitTokenOffsetFilter;
using LimitTokenPositionFilter =
    org::apache::lucene::analysis::miscellaneous::LimitTokenPositionFilter;
using StemmerOverrideFilter =
    org::apache::lucene::analysis::miscellaneous::StemmerOverrideFilter;
using StemmerOverrideMap = org::apache::lucene::analysis::miscellaneous::
    StemmerOverrideFilter::StemmerOverrideMap;
using WordDelimiterFilter =
    org::apache::lucene::analysis::miscellaneous::WordDelimiterFilter;
using WordDelimiterGraphFilter =
    org::apache::lucene::analysis::miscellaneous::WordDelimiterGraphFilter;
using PathHierarchyTokenizer =
    org::apache::lucene::analysis::path::PathHierarchyTokenizer;
using ReversePathHierarchyTokenizer =
    org::apache::lucene::analysis::path::ReversePathHierarchyTokenizer;
using IdentityEncoder =
    org::apache::lucene::analysis::payloads::IdentityEncoder;
using PayloadEncoder = org::apache::lucene::analysis::payloads::PayloadEncoder;
using ShingleFilter = org::apache::lucene::analysis::shingle::ShingleFilter;
using TestSnowball = org::apache::lucene::analysis::snowball::TestSnowball;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using SynonymMap = org::apache::lucene::analysis::synonym::SynonymMap;
using WikipediaTokenizer =
    org::apache::lucene::analysis::wikipedia::WikipediaTokenizer;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using CharsRef = org::apache::lucene::util::CharsRef;
using Rethrow = org::apache::lucene::util::Rethrow;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using AutomatonTestUtil =
    org::apache::lucene::util::automaton::AutomatonTestUtil;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using Operations = org::apache::lucene::util::automaton::Operations;
using RegExp = org::apache::lucene::util::automaton::RegExp;
using org::junit::AfterClass;
using org::junit::BeforeClass;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
using org::xml::sax::InputSource;
deque < Constructor < ? extends org.apache.lucene.analysis.Tokenizer >>
                             TestRandomChains::tokenizers;
deque < Constructor < ? extends org.apache.lucene.analysis.TokenFilter >>
                             TestRandomChains::tokenfilters;
deque < Constructor < ? extends org.apache.lucene.analysis.CharFilter >>
                             TestRandomChains::charfilters;
const function<bool(any[] *)> TestRandomChains::ALWAYS =
    ([&](any objects) { true; });
const shared_ptr<java::util::Set<type_info>>
    TestRandomChains::avoidConditionals = unordered_set<type_info>();

TestRandomChains::StaticConstructor::StaticConstructor()
{
  // These filters needs to consume the whole tokenstream, so conditionals don't
  // make sense here
  avoidConditionals->add(FingerprintFilter::typeid);
  avoidConditionals->add(MinHashFilter::typeid);
  avoidConditionals->add(ConcatenateGraphFilter::typeid);
  // ShingleFilter doesn't handle input graphs correctly, so wrapping it in a
  // condition can expose inconsistent offsets
  // https://issues.apache.org/jira/browse/LUCENE-4170
  avoidConditionals->add(ShingleFilter::typeid);
  // FlattenGraphFilter changes the output graph entirely, so wrapping it in a
  // condition can break position lengths
  avoidConditionals->add(FlattenGraphFilter::typeid);
  try {
    brokenConstructors.emplace(LimitTokenCountFilter::typeid->getConstructor(
                                   TokenStream::typeid, int ::typeid),
                               ALWAYS);
    brokenConstructors.emplace(
        LimitTokenCountFilter::typeid->getConstructor(
            TokenStream::typeid, int ::typeid, bool ::typeid),
        [&](any args) {
          assert(args->length == 3);
          return !(static_cast<optional<bool>>(args[2]));
        });
    brokenConstructors.emplace(LimitTokenOffsetFilter::typeid->getConstructor(
                                   TokenStream::typeid, int ::typeid),
                               ALWAYS);
    brokenConstructors.emplace(
        LimitTokenOffsetFilter::typeid->getConstructor(
            TokenStream::typeid, int ::typeid, bool ::typeid),
        [&](any args) {
          assert(args->length == 3);
          return !(static_cast<optional<bool>>(args[2]));
        });
    brokenConstructors.emplace(LimitTokenPositionFilter::typeid->getConstructor(
                                   TokenStream::typeid, int ::typeid),
                               ALWAYS);
    brokenConstructors.emplace(
        LimitTokenPositionFilter::typeid->getConstructor(
            TokenStream::typeid, int ::typeid, bool ::typeid),
        [&](any args) {
          assert(args->length == 3);
          return !(static_cast<optional<bool>>(args[2]));
        });
    for (type_info c : Arrays::asList<type_info>(
             CachingTokenFilter::typeid, CJKBigramFilter::typeid,
             CommonGramsFilter::typeid, CommonGramsQueryFilter::typeid,
             CrankyTokenFilter::typeid, HyphenatedWordsFilter::typeid,
             PathHierarchyTokenizer::typeid,
             ReversePathHierarchyTokenizer::typeid,
             ValidatingTokenFilter::typeid, WikipediaTokenizer::typeid,
             WordDelimiterFilter::typeid, WordDelimiterGraphFilter::typeid,
             DelimitedTermFrequencyTokenFilter::typeid,
             org::apache::lucene::analysis::core::StopFilter::typeid,
             org::apache::lucene::analysis::core::LowerCaseFilter::typeid)) {
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: for (Constructor<?> ctor : c.getConstructors())
      for (shared_ptr < Constructor < ? >> ctor : c.getConstructors()) {
        brokenConstructors.emplace(ctor, ALWAYS);
      }
    }
  } catch (const runtime_error &e) {
    throw make_shared<Error>(e);
  }
  allowedTokenizerArgs = Collections::newSetFromMap(
      make_shared<IdentityHashMap<type_info, bool>>());
  allowedTokenizerArgs->addAll(argProducers::keySet());
  allowedTokenizerArgs->add(Reader::typeid);
  allowedTokenizerArgs->add(AttributeFactory::typeid);
  allowedTokenizerArgs->add(AttributeSource::typeid);
  allowedTokenizerArgs->add(Automaton::typeid);

  allowedTokenFilterArgs = Collections::newSetFromMap(
      make_shared<IdentityHashMap<type_info, bool>>());
  allowedTokenFilterArgs->addAll(argProducers::keySet());
  allowedTokenFilterArgs->add(TokenStream::typeid);
  // TODO: fix this one, thats broken:
  allowedTokenFilterArgs->add(CommonGramsFilter::typeid);

  allowedCharFilterArgs = Collections::newSetFromMap(
      make_shared<IdentityHashMap<type_info, bool>>());
  allowedCharFilterArgs->addAll(argProducers::keySet());
  allowedCharFilterArgs->add(Reader::typeid);
}

TestRandomChains::StaticConstructor TestRandomChains::staticConstructor;
const unordered_map < Constructor < ? >,
    function<bool(any[] *)>>
        TestRandomChains::brokenConstructors = unordered_map < Constructor <
    ? >,
        function<bool(any[] *)>> ();

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestRandomChains::beforeClass() 
{
  deque<type_info> analysisClasses =
      getClassesForPackage(L"org.apache.lucene.analysis");
  tokenizers = deque<>();
  tokenfilters = deque<>();
  charfilters = deque<>();
  for (auto c : analysisClasses) {
    constexpr int modifiers = c.getModifiers();
    if (Modifier::isAbstract(modifiers) || !Modifier::isPublic(modifiers) ||
        c.isSynthetic() || c.isAnonymousClass() || c.isMemberClass() ||
        c.isInterface() || c.isAnnotationPresent(Deprecated::typeid) ||
        !(Tokenizer::typeid->isAssignableFrom(c) ||
          TokenFilter::typeid->isAssignableFrom(c) ||
          CharFilter::typeid->isAssignableFrom(c))) {
      continue;
    }

    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: for (final Constructor<?> ctor : c.getConstructors())
    for (shared_ptr < Constructor < ? >> ctor : c.getConstructors()) {
      // don't test synthetic or deprecated ctors, they likely have known bugs:
      if (ctor->isSynthetic() ||
          ctor->isAnnotationPresent(Deprecated::typeid) ||
          brokenConstructors[ctor] == ALWAYS) {
        continue;
      }
      // conditional filters are tested elsewhere
      if (ConditionalTokenFilter::typeid->isAssignableFrom(c)) {
        continue;
      }
      if (Tokenizer::typeid->isAssignableFrom(c)) {
        assertTrue(ctor->toGenericString() +
                       L" has unsupported parameter types",
                   allowedTokenizerArgs->containsAll(
                       Arrays::asList(ctor->getParameterTypes())));
        tokenizers.push_back(castConstructor(Tokenizer::typeid, ctor));
      } else if (TokenFilter::typeid->isAssignableFrom(c)) {
        assertTrue(ctor->toGenericString() +
                       L" has unsupported parameter types",
                   allowedTokenFilterArgs->containsAll(
                       Arrays::asList(ctor->getParameterTypes())));
        tokenfilters.push_back(castConstructor(TokenFilter::typeid, ctor));
      } else if (CharFilter::typeid->isAssignableFrom(c)) {
        assertTrue(ctor->toGenericString() +
                       L" has unsupported parameter types",
                   allowedCharFilterArgs->containsAll(
                       Arrays::asList(ctor->getParameterTypes())));
        charfilters.push_back(castConstructor(CharFilter::typeid, ctor));
      } else {
        fail(L"Cannot get here");
      }
    }
  }

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: final java.util.Comparator<Constructor<?>> ctorComp = (arg0,
  // arg1) -> arg0.toGenericString().compareTo(arg1.toGenericString());
  shared_ptr < Comparator < Constructor <
      ? >>> *const ctorComp = [&](arg0, arg1) {
          arg0::toGenericString()->compareTo(arg1::toGenericString());
        };
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(tokenizers, ctorComp);
  sort(tokenizers.begin(), tokenizers.end(), ctorComp);
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(tokenfilters, ctorComp);
  sort(tokenfilters.begin(), tokenfilters.end(), ctorComp);
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(charfilters, ctorComp);
  sort(charfilters.begin(), charfilters.end(), ctorComp);
  if (VERBOSE) {
    wcout << L"tokenizers = " << tokenizers << endl;
    wcout << L"tokenfilters = " << tokenfilters << endl;
    wcout << L"charfilters = " << charfilters << endl;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass()
void TestRandomChains::afterClass()
{
  tokenizers.clear();
  tokenfilters.clear();
  charfilters.clear();
}

template <typename T, typename T1>
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") private static <T>
// Constructor<T> castConstructor(Class<T> instanceClazz, Constructor<?> ctor)
shared_ptr<Constructor<T>>
TestRandomChains::castConstructor(type_info<T> &instanceClazz,
                                  shared_ptr<Constructor<T1>> ctor)
{
  return std::static_pointer_cast<Constructor<T>>(ctor);
}

deque<type_info> TestRandomChains::getClassesForPackage(
    const wstring &pckgname) 
{
  const deque<type_info> classes = deque<type_info>();
  collectClassesForPackage(pckgname, classes);
  assertFalse(L"No classes found in package '" + pckgname +
                  L"'; maybe your test classes are packaged as JAR file?",
              classes.empty());
  return classes;
}

void TestRandomChains::collectClassesForPackage(
    const wstring &pckgname, deque<type_info> &classes) 
{
  shared_ptr<ClassLoader> *const cld =
      TestRandomChains::typeid->getClassLoader();
  const wstring path = StringHelper::replace(pckgname, L'.', L'/');
  const java::util::Iterator<std::shared_ptr<URL>> resources =
      cld->getResources(path);
  while (resources->hasMoreElements()) {
    shared_ptr<URI> *const uri = resources->nextElement().toURI();
    // C++ TODO: The following Java case-insensitive std::wstring method call is not
    // converted:
    if (!(wstring(L"file")).equalsIgnoreCase(uri->getScheme())) {
      continue;
    }
    shared_ptr<Path> *const directory = Paths->get(uri);
    if (Files::exists(directory)) {
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try
      // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
      // java.nio.file.Files.newDirectoryStream(directory))
      {
        shared_ptr<java::nio::file::DirectoryStream<
            std::shared_ptr<java::nio::file::Path>>>
            stream = java::nio::file::Files::newDirectoryStream(directory);
        for (auto file : stream) {
          if (Files::isDirectory(file)) {
            // recurse
            // C++ TODO: There is no native C++ equivalent to 'toString':
            wstring subPackage =
                pckgname + L"." + file->getFileName()->toString();
            collectClassesForPackage(subPackage, classes);
          }
          // C++ TODO: There is no native C++ equivalent to 'toString':
          wstring fname = file->getFileName()->toString();
          if (StringHelper::endsWith(fname, L".class")) {
            wstring clazzName = fname.substr(0, fname.length() - 6);
            // exclude Test classes that happen to be in these packages.
            // class.ForName'ing some of them can cause trouble.
            if (!StringHelper::endsWith(clazzName, L"Test") &&
                !StringHelper::startsWith(clazzName, L"Test")) {
              // Don't run static initializers, as we won't use most of them.
              // Java will do that automatically once accessed/instantiated.
              classes.push_back(type_info::forName(
                  pckgname + StringHelper::toString(L'.') + clazzName, false,
                  cld));
            }
          }
        }
      }
    }
  }
}

const unordered_map<type_info, function<any *(java::util::Random *)>>
    TestRandomChains::argProducers =
        make_shared<IdentityHashMapAnonymousInnerClass>(java::util::Random *);

TestRandomChains::IdentityHashMapAnonymousInnerClass::
    IdentityHashMapAnonymousInnerClass(shared_ptr<UnknownType> java)
    : java->util
          .IdentityHashMap<Class, function<std::any(java->util.Random *)>>(
              Random *)
{
}
} // namespace org::apache::lucene::analysis::core