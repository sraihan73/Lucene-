using namespace std;

#include "TestDiversifiedTopDocsCollector.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FloatDocValuesField = org::apache::lucene::document::FloatDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValues = org::apache::lucene::index::DocValues;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestDiversifiedTopDocsCollector::testNonDiversifiedResults() throw(
    runtime_error)
{
  int numberOfTracksOnCompilation = 10;
  int expectedMinNumOfBeatlesHits = 5;
  shared_ptr<TopDocs> res =
      searcher->search(getTestQuery(), numberOfTracksOnCompilation);
  assertEquals(numberOfTracksOnCompilation, res->scoreDocs.size());
  // due to randomization of segment merging in tests the exact number of
  // Beatles hits selected varies between 5 and 6 but we prove the point they
  // are over-represented in our result set using a standard search.
  assertTrue(getMaxNumRecordsPerArtist(res->scoreDocs) >=
             expectedMinNumOfBeatlesHits);
}

void TestDiversifiedTopDocsCollector::testFirstPageDiversifiedResults() throw(
    runtime_error)
{
  // Using a diversified collector we can limit the results from
  // any one artist.
  int requiredMaxHitsPerArtist = 2;
  int numberOfTracksOnCompilation = 10;
  shared_ptr<DiversifiedTopDocsCollector> tdc = doDiversifiedSearch(
      numberOfTracksOnCompilation, requiredMaxHitsPerArtist);
  std::deque<std::shared_ptr<ScoreDoc>> sd = tdc->topDocs(0)->scoreDocs;
  assertEquals(numberOfTracksOnCompilation, sd.size());
  assertTrue(getMaxNumRecordsPerArtist(sd) <= requiredMaxHitsPerArtist);
}

void TestDiversifiedTopDocsCollector::testSecondPageResults() throw(
    runtime_error)
{
  int numberOfTracksPerCompilation = 10;
  int numberOfCompilations = 2;
  int requiredMaxHitsPerArtist = 1;

  // Volume 2 of our hits compilation - start at position 10
  shared_ptr<DiversifiedTopDocsCollector> tdc =
      doDiversifiedSearch(numberOfTracksPerCompilation * numberOfCompilations,
                          requiredMaxHitsPerArtist);
  std::deque<std::shared_ptr<ScoreDoc>> volume2 =
      tdc->topDocs(numberOfTracksPerCompilation, numberOfTracksPerCompilation)
          ->scoreDocs;
  assertEquals(numberOfTracksPerCompilation, volume2.size());
  assertTrue(getMaxNumRecordsPerArtist(volume2) <= requiredMaxHitsPerArtist);
}

void TestDiversifiedTopDocsCollector::testInvalidArguments() throw(
    runtime_error)
{
  int numResults = 5;
  shared_ptr<DiversifiedTopDocsCollector> tdc =
      doDiversifiedSearch(numResults, 15);

  // start < 0
  assertEquals(0, tdc->topDocs(-1)->scoreDocs.size());

  // start > pq.size()
  assertEquals(0, tdc->topDocs(numResults + 1)->scoreDocs.size());

  // start == pq.size()
  assertEquals(0, tdc->topDocs(numResults)->scoreDocs.size());

  // howMany < 0
  assertEquals(0, tdc->topDocs(0, -1)->scoreDocs.size());

  // howMany == 0
  assertEquals(0, tdc->topDocs(0, 0)->scoreDocs.size());
}

TestDiversifiedTopDocsCollector::DocValuesDiversifiedCollector::
    DocValuesDiversifiedCollector(int size, int maxHitsPerKey,
                                  shared_ptr<SortedDocValues> sdv)
    : DiversifiedTopDocsCollector(size, maxHitsPerKey), sdv(sdv)
{
}

shared_ptr<NumericDocValues>
TestDiversifiedTopDocsCollector::DocValuesDiversifiedCollector::getKeys(
    shared_ptr<LeafReaderContext> context)
{

  return make_shared<NumericDocValuesAnonymousInnerClass>(shared_from_this(),
                                                          context);
}

TestDiversifiedTopDocsCollector::DocValuesDiversifiedCollector::
    NumericDocValuesAnonymousInnerClass::NumericDocValuesAnonymousInnerClass(
        shared_ptr<DocValuesDiversifiedCollector> outerInstance,
        shared_ptr<LeafReaderContext> context)
{
  this->outerInstance = outerInstance;
  this->context = context;
}

int TestDiversifiedTopDocsCollector::DocValuesDiversifiedCollector::
    NumericDocValuesAnonymousInnerClass::docID()
{
  return outerInstance->sdv->docID() - context->docBase;
}

int TestDiversifiedTopDocsCollector::DocValuesDiversifiedCollector::
    NumericDocValuesAnonymousInnerClass::nextDoc() 
{
  return outerInstance->sdv->nextDoc() - context->docBase;
}

int TestDiversifiedTopDocsCollector::DocValuesDiversifiedCollector::
    NumericDocValuesAnonymousInnerClass::advance(int target) 
{
  return outerInstance->sdv->advance(target + context->docBase);
}

bool TestDiversifiedTopDocsCollector::DocValuesDiversifiedCollector::
    NumericDocValuesAnonymousInnerClass::advanceExact(int target) throw(
        IOException)
{
  return outerInstance->sdv->advanceExact(target + context->docBase);
}

int64_t TestDiversifiedTopDocsCollector::DocValuesDiversifiedCollector::
    NumericDocValuesAnonymousInnerClass::cost()
{
  return 0;
}

int64_t TestDiversifiedTopDocsCollector::DocValuesDiversifiedCollector::
    NumericDocValuesAnonymousInnerClass::longValue() 
{
  // Keys are always expressed as a long so we obtain the
  // ordinal for our std::wstring-based artist name here
  return outerInstance->sdv->ordValue();
}

TestDiversifiedTopDocsCollector::HashedDocValuesDiversifiedCollector::
    HashedDocValuesDiversifiedCollector(int size, int maxHitsPerKey,
                                        const wstring &field)
    : DiversifiedTopDocsCollector(size, maxHitsPerKey), field(field)
{
}

shared_ptr<NumericDocValues>
TestDiversifiedTopDocsCollector::HashedDocValuesDiversifiedCollector::getKeys(
    shared_ptr<LeafReaderContext> context)
{
  return make_shared<NumericDocValuesAnonymousInnerClass>(shared_from_this());
}

TestDiversifiedTopDocsCollector::HashedDocValuesDiversifiedCollector::
    NumericDocValuesAnonymousInnerClass::NumericDocValuesAnonymousInnerClass(
        shared_ptr<HashedDocValuesDiversifiedCollector> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TestDiversifiedTopDocsCollector::HashedDocValuesDiversifiedCollector::
    NumericDocValuesAnonymousInnerClass::docID()
{
  return outerInstance->vals->docID();
}

int TestDiversifiedTopDocsCollector::HashedDocValuesDiversifiedCollector::
    NumericDocValuesAnonymousInnerClass::nextDoc() 
{
  return outerInstance->vals->nextDoc();
}

int TestDiversifiedTopDocsCollector::HashedDocValuesDiversifiedCollector::
    NumericDocValuesAnonymousInnerClass::advance(int target) 
{
  return outerInstance->vals->advance(target);
}

bool TestDiversifiedTopDocsCollector::HashedDocValuesDiversifiedCollector::
    NumericDocValuesAnonymousInnerClass::advanceExact(int target) throw(
        IOException)
{
  return outerInstance->vals->advanceExact(target);
}

int64_t TestDiversifiedTopDocsCollector::HashedDocValuesDiversifiedCollector::
    NumericDocValuesAnonymousInnerClass::cost()
{
  return outerInstance->vals->cost();
}

int64_t TestDiversifiedTopDocsCollector::HashedDocValuesDiversifiedCollector::
    NumericDocValuesAnonymousInnerClass::longValue() 
{
  return outerInstance->vals == nullptr
             ? -1
             : outerInstance->vals->binaryValue()->hashCode();
}

shared_ptr<LeafCollector>
TestDiversifiedTopDocsCollector::HashedDocValuesDiversifiedCollector::
    getLeafCollector(shared_ptr<LeafReaderContext> context) 
{
  this->vals = DocValues::getBinary(context->reader(), field);
  return DiversifiedTopDocsCollector::getLeafCollector(context);
}

std::deque<wstring> TestDiversifiedTopDocsCollector::hitsOfThe60s = {
    L"1966\tSPENCER DAVIS GROUP\tKEEP ON RUNNING\t1",
    L"1966\tOVERLANDERS\tMICHELLE\t3",
    L"1966\tNANCY SINATRA\tTHESE BOOTS ARE MADE FOR WALKIN'\t4",
    L"1966\tWALKER BROTHERS\tTHE SUN AIN'T GONNA SHINE ANYMORE\t4",
    L"1966\tSPENCER DAVIS GROUP\tSOMEBODY HELP ME\t2",
    L"1966\tDUSTY SPRINGFIELD\tYOU DON'T HAVE TO SAY YOU LOVE ME\t1",
    L"1966\tMANFRED MANN\tPRETTY FLAMINGO\t3",
    L"1966\tROLLING STONES\tPAINT IT, BLACK\t1",
    L"1966\tFRANK SINATRA\tSTRANGERS IN THE NIGHT\t3",
    L"1966\tBEATLES\tPAPERBACK WRITER\t5",
    L"1966\tKINKS\tSUNNY AFTERNOON\t2",
    L"1966\tGEORGIE FAME AND THE BLUE FLAMES\tGETAWAY\t1",
    L"1966\tCHRIS FARLOWE\tOUT OF TIME\t1",
    L"1966\tTROGGS\tWITH A GIRL LIKE YOU\t2",
    L"1966\tBEATLES\tYELLOW SUBMARINE/ELEANOR RIGBY\t4",
    L"1966\tSMALL FACES\tALL OR NOTHING\t1",
    L"1966\tJIM REEVES\tDISTANT DRUMS\t5",
    L"1966\tFOUR TOPS\tREACH OUT I'LL BE THERE\t3",
    L"1966\tBEACH BOYS\tGOOD VIBRATIONS\t2",
    L"1966\tTOM JONES\tGREEN GREEN GRASS OF HOME\t4",
    L"1967\tMONKEES\tI'M A BELIEVER\t4",
    L"1967\tPETULA CLARK\tTHIS IS MY SONG\t2",
    L"1967\tENGELBERT HUMPERDINCK\tRELEASE ME\t4",
    L"1967\tNANCY SINATRA AND FRANK SINATRA\tSOMETHIN' STUPID\t2",
    L"1967\tSANDIE SHAW\tPUPPET ON A STRING\t3",
    L"1967\tTREMELOES\tSILENCE IS GOLDEN\t3",
    L"1967\tPROCOL HARUM\tA WHITER SHADE OF PALE\t4",
    L"1967\tBEATLES\tALL YOU NEED IS LOVE\t7",
    L"1967\tSCOTT MCKENZIE\tSAN FRANCISCO (BE SURE TO WEAR SOME FLOWERS INYOUR "
    L"HAIR)\t4",
    L"1967\tENGELBERT HUMPERDINCK\tTHE LAST WALTZ\t5",
    L"1967\tBEE GEES\tMASSACHUSETTS (THE LIGHTS WENT OUT IN)\t4",
    L"1967\tFOUNDATIONS\tBABY NOW THAT I'VE FOUND YOU\t2",
    L"1967\tLONG JOHN BALDRY\tLET THE HEARTACHES BEGIN\t2",
    L"1967\tBEATLES\tHELLO GOODBYE\t5",
    L"1968\tGEORGIE FAME\tTHE BALLAD OF BONNIE AND CLYDE\t1",
    L"1968\tLOVE AFFAIR\tEVERLASTING LOVE\t2",
    L"1968\tMANFRED MANN\tMIGHTY QUINN\t2",
    L"1968\tESTHER AND ABI OFARIM\tCINDERELLA ROCKEFELLA\t3",
    L"1968\tDAVE DEE, DOZY, BEAKY, MICK AND TICH\tTHE LEGEND OF XANADU\t1",
    L"1968\tBEATLES\tLADY MADONNA\t2",
    L"1968\tCLIFF RICHARD\tCONGRATULATIONS\t2",
    L"1968\tLOUIS ARMSTRONG\tWHAT A WONDERFUL WORLD/CABARET\t4",
    L"1968\tGARRY PUCKETT AND THE UNION GAP\tYOUNG GIRL\t4",
    L"1968\tROLLING STONES\tJUMPING JACK FLASH\t2",
    L"1968\tEQUALS\tBABY COME BACK\t3",
    L"1968\tDES O'CONNOR\tI PRETEND\t1",
    L"1968\tTOMMY JAMES AND THE SHONDELLS\tMONY MONY\t2",
    L"1968\tCRAZY WORLD OF ARTHUR BROWN\tFIRE!\t1",
    L"1968\tTOMMY JAMES AND THE SHONDELLS\tMONY MONY\t1",
    L"1968\tBEACH BOYS\tDO IT AGAIN\t1",
    L"1968\tBEE GEES\tI'VE GOTTA GET A MESSAGE TO YOU\t1",
    L"1968\tBEATLES\tHEY JUDE\t8",
    L"1968\tMARY HOPKIN\tTHOSE WERE THE DAYS\t6",
    L"1968\tJOE COCKER\tWITH A LITTLE HELP FROM MY FRIENDS\t1",
    L"1968\tHUGO MONTENEGRO\tTHE GOOD THE BAD AND THE UGLY\t4",
    L"1968\tSCAFFOLD\tLILY THE PINK\t3",
    L"1969\tMARMALADE\tOB-LA-DI, OB-LA-DA\t1",
    L"1969\tSCAFFOLD\tLILY THE PINK\t1",
    L"1969\tMARMALADE\tOB-LA-DI, OB-LA-DA\t2",
    L"1969\tFLEETWOOD MAC\tALBATROSS\t1",
    L"1969\tMOVE\tBLACKBERRY WAY\t1",
    L"1969\tAMEN CORNER\t(IF PARADISE IS) HALF AS NICE\t2",
    L"1969\tPETER SARSTEDT\tWHERE DO YOU GO TO (MY LOVELY)\t4",
    L"1969\tMARVIN GAYE\tI HEARD IT THROUGH THE GRAPEVINE\t3",
    L"1969\tDESMOND DEKKER AND THE ACES\tTHE ISRAELITES\t1",
    L"1969\tBEATLES\tGET BACK\t6",
    L"1969\tTOMMY ROE\tDIZZY\t1",
    L"1969\tBEATLES\tTHE BALLAD OF JOHN AND YOKO\t3",
    L"1969\tTHUNDERCLAP NEWMAN\tSOMETHING IN THE AIR\t3",
    L"1969\tROLLING STONES\tHONKY TONK WOMEN\t5",
    L"1969\tZAGER AND EVANS\tIN THE YEAR 2525 (EXORDIUM AND TERMINUS)\t3",
    L"1969\tCREEDENCE CLEARWATER REVIVAL\tBAD MOON RISING\t3",
    L"1969\tJANE BIRKIN AND SERGE GAINSBOURG\tJE T'AIME... MOI NON PLUS\t1",
    L"1969\tBOBBIE GENTRY\tI'LL NEVER FALL IN LOVE AGAIN\t1",
    L"1969\tARCHIES\tSUGAR, SUGAR\t4"};
const unordered_map<wstring, std::shared_ptr<Record>>
    TestDiversifiedTopDocsCollector::parsedRecords =
        unordered_map<wstring, std::shared_ptr<Record>>();

// C++ TODO: No base class can be determined:
TestDiversifiedTopDocsCollector::Record::Record(const wstring &id,
                                                const wstring &year,
                                                const wstring &artist,
                                                const wstring &song,
                                                float weeks)
{
  this->id = id;
  this->year = year;
  this->artist = artist;
  this->song = song;
  this->weeks = weeks;
}

wstring TestDiversifiedTopDocsCollector::Record::toString()
{
  return L"Record [id=" + id + L", artist=" + artist + L", weeks=" +
         to_wstring(weeks) + L", year=" + year + L", song=" + song + L"]";
}

shared_ptr<DiversifiedTopDocsCollector>
TestDiversifiedTopDocsCollector::doDiversifiedSearch(
    int numResults, int maxResultsPerArtist) 
{
  // Alternate between implementations used for key lookups
  if (random()->nextBoolean()) {
    // Faster key lookup but with potential for collisions on larger datasets
    return doFuzzyDiversifiedSearch(numResults, maxResultsPerArtist);
  } else {
    // Slower key lookup but 100% accurate
    return doAccurateDiversifiedSearch(numResults, maxResultsPerArtist);
  }
}

shared_ptr<DiversifiedTopDocsCollector>
TestDiversifiedTopDocsCollector::doFuzzyDiversifiedSearch(
    int numResults, int maxResultsPerArtist) 
{
  shared_ptr<DiversifiedTopDocsCollector> tdc =
      make_shared<HashedDocValuesDiversifiedCollector>(
          numResults, maxResultsPerArtist, L"artist");
  searcher->search(getTestQuery(), tdc);
  return tdc;
}

shared_ptr<DiversifiedTopDocsCollector>
TestDiversifiedTopDocsCollector::doAccurateDiversifiedSearch(
    int numResults, int maxResultsPerArtist) 
{
  shared_ptr<DiversifiedTopDocsCollector> tdc =
      make_shared<DocValuesDiversifiedCollector>(
          numResults, maxResultsPerArtist, artistDocValues);
  searcher->search(getTestQuery(), tdc);
  return tdc;
}

shared_ptr<Query> TestDiversifiedTopDocsCollector::getTestQuery()
{
  shared_ptr<BooleanQuery::Builder> testQuery =
      make_shared<BooleanQuery::Builder>();
  testQuery->add(make_shared<BooleanClause>(
      make_shared<TermQuery>(make_shared<Term>(L"year", L"1966")),
      Occur::SHOULD));
  testQuery->add(make_shared<BooleanClause>(
      make_shared<TermQuery>(make_shared<Term>(L"year", L"1967")),
      Occur::SHOULD));
  testQuery->add(make_shared<BooleanClause>(
      make_shared<TermQuery>(make_shared<Term>(L"year", L"1968")),
      Occur::SHOULD));
  testQuery->add(make_shared<BooleanClause>(
      make_shared<TermQuery>(make_shared<Term>(L"year", L"1969")),
      Occur::SHOULD));
  return testQuery->build();
}

void TestDiversifiedTopDocsCollector::setUp() 
{
  LuceneTestCase::setUp();

  // populate an index with documents - artist, song and weeksAtNumberOne
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<Field> yearField = newTextField(L"year", L"", Field::Store::NO);
  shared_ptr<SortedDocValuesField> artistField =
      make_shared<SortedDocValuesField>(L"artist", make_shared<BytesRef>(L""));
  shared_ptr<Field> weeksAtNumberOneField =
      make_shared<FloatDocValuesField>(L"weeksAtNumberOne", 0.0F);
  shared_ptr<Field> weeksStoredField = make_shared<StoredField>(L"weeks", 0.0F);
  shared_ptr<Field> idField = newStringField(L"id", L"", Field::Store::YES);
  shared_ptr<Field> songField = newTextField(L"song", L"", Field::Store::NO);
  shared_ptr<Field> storedArtistField =
      newTextField(L"artistName", L"", Field::Store::NO);

  doc->push_back(idField);
  doc->push_back(weeksAtNumberOneField);
  doc->push_back(storedArtistField);
  doc->push_back(songField);
  doc->push_back(weeksStoredField);
  doc->push_back(yearField);
  doc->push_back(artistField);

  parsedRecords.clear();
  for (int i = 0; i < hitsOfThe60s.size(); i++) {
    std::deque<wstring> cols = hitsOfThe60s[i].split(L"\t");
    shared_ptr<Record> record = make_shared<Record>(
        to_wstring(i), cols[0], cols[1], cols[2], stof(cols[3]));
    parsedRecords.emplace(record->id, record);
    idField->setStringValue(record->id);
    yearField->setStringValue(record->year);
    storedArtistField->setStringValue(record->artist);
    artistField->setBytesValue(make_shared<BytesRef>(record->artist));
    songField->setStringValue(record->song);
    weeksStoredField->setFloatValue(record->weeks);
    weeksAtNumberOneField->setFloatValue(record->weeks);
    writer->addDocument(doc);
    if (i % 10 == 0) {
      // Causes the creation of multiple segments for our test
      writer->commit();
    }
  }
  reader = writer->getReader();
  delete writer;
  searcher = newSearcher(reader);
  artistDocValues = MultiDocValues::getSortedValues(reader, L"artist");

  // All searches sort by song popularity
  shared_ptr<Similarity> *const base = searcher->getSimilarity(true);
  searcher->setSimilarity(
      make_shared<DocValueSimilarity>(base, L"weeksAtNumberOne"));
}

void TestDiversifiedTopDocsCollector::tearDown() 
{
  delete reader;
  delete dir;
  dir.reset();
  LuceneTestCase::tearDown();
}

int TestDiversifiedTopDocsCollector::getMaxNumRecordsPerArtist(
    std::deque<std::shared_ptr<ScoreDoc>> &sd) 
{
  int result = 0;
  unordered_map<wstring, int> artistCounts = unordered_map<wstring, int>();
  for (int i = 0; i < sd.size(); i++) {
    shared_ptr<Document> doc = reader->document(sd[i]->doc);
    shared_ptr<Record> record = parsedRecords[doc[L"id"]];
    optional<int> count = artistCounts[record->artist];
    int newCount = 1;
    if (count) {
      newCount = count.value() + 1;
    }
    result = max(result, newCount);
    artistCounts.emplace(record->artist, newCount);
  }
  return result;
}

TestDiversifiedTopDocsCollector::DocValueSimilarity::DocValueSimilarity(
    shared_ptr<Similarity> sim, const wstring &scoreValueField)
    : sim(sim), scoreValueField(scoreValueField)
{
}

int64_t TestDiversifiedTopDocsCollector::DocValueSimilarity::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  return sim->computeNorm(state);
}

shared_ptr<Similarity::SimWeight>
TestDiversifiedTopDocsCollector::DocValueSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  return sim->computeWeight(boost, collectionStats, {termStats});
}

shared_ptr<Similarity::SimScorer>
TestDiversifiedTopDocsCollector::DocValueSimilarity::simScorer(
    shared_ptr<Similarity::SimWeight> stats,
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Similarity::SimScorer> *const sub = sim->simScorer(stats, context);
  shared_ptr<NumericDocValues> *const values =
      DocValues::getNumeric(context->reader(), scoreValueField);

  return make_shared<SimScorerAnonymousInnerClass>(shared_from_this(), sub,
                                                   values);
}

TestDiversifiedTopDocsCollector::DocValueSimilarity::
    SimScorerAnonymousInnerClass::SimScorerAnonymousInnerClass(
        shared_ptr<DocValueSimilarity> outerInstance,
        shared_ptr<Similarity::SimScorer> sub,
        shared_ptr<NumericDocValues> values)
{
  this->outerInstance = outerInstance;
  this->sub = sub;
  this->values = values;
}

float TestDiversifiedTopDocsCollector::DocValueSimilarity::
    SimScorerAnonymousInnerClass::score(int doc, float freq) 
{
  if (doc != values->docID()) {
    values->advance(doc);
  }
  if (doc == values->docID()) {
    return Float::intBitsToFloat(static_cast<int>(values->longValue()));
  } else {
    return 0.0f;
  }
}

float TestDiversifiedTopDocsCollector::DocValueSimilarity::
    SimScorerAnonymousInnerClass::computeSlopFactor(int distance)
{
  return sub->computeSlopFactor(distance);
}

float TestDiversifiedTopDocsCollector::DocValueSimilarity::
    SimScorerAnonymousInnerClass::computePayloadFactor(
        int doc, int start, int end, shared_ptr<BytesRef> payload)
{
  return sub->computePayloadFactor(doc, start, end, payload);
}

shared_ptr<Explanation> TestDiversifiedTopDocsCollector::DocValueSimilarity::
    SimScorerAnonymousInnerClass::explain(
        int doc, shared_ptr<Explanation> freq) 
{
  return Explanation::match(score(doc, 0.0f),
                            L"indexDocValue(" + outerInstance->scoreValueField +
                                L")");
}
} // namespace org::apache::lucene::search