using namespace std;

#include "StemmerTestBase.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/RAMDirectory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../java/org/apache/lucene/analysis/hunspell/Dictionary.h"
#include "../../../../../../java/org/apache/lucene/analysis/hunspell/Stemmer.h"

namespace org::apache::lucene::analysis::hunspell
{
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using CharsRef = org::apache::lucene::util::CharsRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
shared_ptr<Stemmer> StemmerTestBase::stemmer;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass()
void StemmerTestBase::afterClass() { stemmer.reset(); }

void StemmerTestBase::init(const wstring &affix,
                           const wstring &dictionary) throw(IOException,
                                                            ParseException)
{
  init(false, affix, dictionary);
}

void StemmerTestBase::init(bool ignoreCase, const wstring &affix,
                           deque<wstring> &dictionaries) throw(IOException,
                                                                ParseException)
{
  if (dictionaries->length == 0) {
    throw invalid_argument(L"there must be at least one dictionary");
  }

  shared_ptr<InputStream> affixStream =
      StemmerTestBase::typeid->getResourceAsStream(affix);
  if (affixStream == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.io.FileNotFoundException(L"file not found: " + affix);
  }

  std::deque<std::shared_ptr<InputStream>> dictStreams(dictionaries->length);
  for (int i = 0; i < dictionaries->length; i++) {
    dictStreams[i] =
        StemmerTestBase::typeid->getResourceAsStream(dictionaries[i]);
    if (dictStreams[i] == nullptr) {
      // C++ TODO: The following line could not be converted:
      throw java.io.FileNotFoundException(L"file not found: " + dictStreams[i]);
    }
  }

  try {
    shared_ptr<Dictionary> dictionary = make_shared<Dictionary>(
        make_shared<RAMDirectory>(), L"dictionary", affixStream,
        Arrays::asList(dictStreams), ignoreCase);
    stemmer = make_shared<Stemmer>(dictionary);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::closeWhileHandlingException({affixStream});
    IOUtils::closeWhileHandlingException(dictStreams);
  }
}

void StemmerTestBase::assertStemsTo(const wstring &s, deque<wstring> &expected)
{
  assertNotNull(stemmer);
  Arrays::sort(expected);

  deque<std::shared_ptr<CharsRef>> stems = stemmer->stem(s);
  std::deque<wstring> actual(stems.size());
  for (int i = 0; i < actual.size(); i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    actual[i] = stems[i]->toString();
  }
  Arrays::sort(actual);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertArrayEquals(L"expected=" + Arrays->toString(expected) + L",actual=" +
                        Arrays->toString(actual),
                    expected, actual);
}
} // namespace org::apache::lucene::analysis::hunspell