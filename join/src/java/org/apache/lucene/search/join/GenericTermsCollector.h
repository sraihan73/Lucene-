#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRefHash;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::search::join
{

using Collector = org::apache::lucene::search::Collector;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;

class GenericTermsCollector : public Collector
{
  GET_CLASS_NAME(GenericTermsCollector)

public:
  virtual std::shared_ptr<BytesRefHash> getCollectedTerms() = 0;

  virtual std::deque<float> getScoresPerTerm() = 0;

  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static GenericTermsCollector
  //      createCollectorMV(org.apache.lucene.search.join.DocValuesTermsCollector.Function<org.apache.lucene.index.SortedSetDocValues>
  //      mvFunction, ScoreMode mode)
  //  {
  //
  //    switch (mode)
  //    {
  //      case None:
  //        return wrap(new TermsCollector.MV(mvFunction));
  //      case Avg:
  //        return new MV.Avg(mvFunction);
  //      default:
  //        return new MV(mvFunction, mode);
  //    }
  //  }

  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static
  //      org.apache.lucene.search.join.DocValuesTermsCollector.Function<org.apache.lucene.index.SortedSetDocValues>
  //      verbose(java.io.PrintStream out,
  //      org.apache.lucene.search.join.DocValuesTermsCollector.Function<org.apache.lucene.index.SortedSetDocValues>
  //      mvFunction)
  //  {
  //    return (ctx) ->
  //    {
  //      final SortedSetDocValues target = mvFunction.apply(ctx);
  //      return new SortedSetDocValues()
  //      {
  //
  //        @@Override public int docID()
  //        {
  //          return target.docID();
  //        }
  //
  //        @@Override public int nextDoc() throws IOException
  //        {
  //          int docID = target.nextDoc();
  //          out.println("\nnextDoc doc# "+docID);
  //          return docID;
  //        }
  //
  //        @@Override public int advance(int dest) throws IOException
  //        {
  //          int docID = target.advance(dest);
  //          out.println("\nadvance(" + dest + ") -> doc# "+docID);
  //          return docID;
  //        }
  //
  //        @@Override public bool advanceExact(int dest) throws IOException
  //        {
  //          bool exists = target.advanceExact(dest);
  //          out.println("\nadvanceExact(" + dest + ") -> exists# "+exists);
  //          return exists;
  //        }
  //
  //        @@Override public long cost()
  //        {
  //          return target.cost();
  //        }
  //
  //        @@Override public long nextOrd() throws IOException
  //        {
  //          return target.nextOrd();
  //        }
  //
  //        @@Override public BytesRef lookupOrd(long ord) throws IOException
  //        {
  //          final BytesRef val = target.lookupOrd(ord);
  //          out.println(val.toString()+", ");
  //          return val;
  //        }
  //
  //        @@Override public long getValueCount()
  //        {
  //          return target.getValueCount();
  //        }
  //      };
  //
  //    };
  //  }

  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static GenericTermsCollector
  //      createCollectorSV(org.apache.lucene.search.join.DocValuesTermsCollector.Function<org.apache.lucene.index.BinaryDocValues>
  //      svFunction, ScoreMode mode)
  //  {
  //
  //    switch (mode)
  //    {
  //      case None:
  //        return wrap(new TermsCollector.SV(svFunction));
  //      case Avg:
  //        return new SV.Avg(svFunction);
  //      default:
  //        return new SV(svFunction, mode);
  //    }
  //  }

  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  //      static GenericTermsCollector wrap(final TermsCollector<?> collector)
  //  {
  //    return new GenericTermsCollector()
  //    {
  //
  //
  //      @@Override public LeafCollector getLeafCollector(LeafReaderContext
  //      context) throws IOException
  //      {
  //        return collector.getLeafCollector(context);
  //      }
  //
  //      @@Override public bool needsScores()
  //      {
  //        return collector.needsScores();
  //      }
  //
  //      @@Override public BytesRefHash getCollectedTerms()
  //      {
  //        return collector.getCollectorTerms();
  //      }
  //
  //      @@Override public float[] getScoresPerTerm()
  //      {
  //        throw new UnsupportedOperationException("scores are not available
  //        for "+collector);
  //      }
  //    };
  //  }
};

} // namespace org::apache::lucene::search::join
