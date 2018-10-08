using namespace std;

#include "TestBoolValOfNumericDVs.h"

namespace org::apache::lucene::queries::function::docvalues
{
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestBoolValOfNumericDVs::test() 
{
  check(true);
  check(false);
}

void TestBoolValOfNumericDVs::check(bool const expected) 
{

  // create "constant" based instances of each superclass that should returned
  // the expected value based on the constant used
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: final org.apache.lucene.queries.function.FunctionValues[]
  // values = new org.apache.lucene.queries.function.FunctionValues[] { new
  // FloatDocValues(null) { @Override public float floatVal(int doc) throws
  // java.io.IOException { return expected ? Float.MIN_VALUE : 0.0F; } }, new
  // DoubleDocValues(null) { @Override public double doubleVal(int doc) throws
  // java.io.IOException { return expected ? Double.MIN_VALUE : 0.0D; } }, new
  // IntDocValues(null) { @Override public int intVal(int doc) throws
  // java.io.IOException { return expected ? 1 : 0; } }, new LongDocValues(null)
  // { @Override public long longVal(int doc) throws java.io.IOException { return
  // expected ? 1L : 0L; } }};
  std::deque<std::shared_ptr<FunctionValues>> values = {
      make_shared<FloatDocValues>(nullptr){float floatVal(
          int doc) throws IOException{return expected ? Float::MIN_VALUE : 0.0F;
}
} // namespace org::apache::lucene::queries::function::docvalues
, make_shared<DoubleDocValues>(nullptr){
      public double doubleVal(int doc)
          throws IOException{return expected ? Double::MIN_VALUE : 0.0;
}
}
, make_shared<IntDocValues>(nullptr){
      public int intVal(int doc) throws IOException{return expected ? 1 : 0;
}
}
, make_shared<LongDocValues>(nullptr)
{
public
  int64_t longVal(int doc) throws IOException { return expected ? 1LL : 0LL; }
}
}
;

for (auto fv : values) {
  // docId is irrelevant since all of our FunctionValues return a constant
  // value.
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(fv->getClass().getSuperclass()->toString(), expected,
               fv->boolVal(123));
}
}
}