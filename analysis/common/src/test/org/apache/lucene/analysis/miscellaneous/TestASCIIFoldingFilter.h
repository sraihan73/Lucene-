#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/miscellaneous/ASCIIFoldingFilter.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"

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
namespace org::apache::lucene::analysis::miscellaneous
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

class TestASCIIFoldingFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestASCIIFoldingFilter)
  /**
   * Pop one input token's worth of tokens off the filter and verify that they
   * are as expected.
   */
public:
  virtual void assertNextTerms(
      const std::wstring &expectedUnfolded, const std::wstring &expectedFolded,
      std::shared_ptr<ASCIIFoldingFilter> filter,
      std::shared_ptr<CharTermAttribute> termAtt) ;

  // testLain1Accents() is a copy of TestLatin1AccentFilter.testU().
  virtual void testLatin1Accents() ;

  // Test that we do not emit duplicated tokens when preserve original is on
  virtual void testUnmodifiedLetters() ;

  // The following Perl script generated the foldings[] array automatically
  // from ASCIIFoldingFilter.java:
  //
  //    ============== begin get.test.cases.pl ==============
  //
  //    use strict;
  //    use warnings;
  //
  //    my $file = "ASCIIFoldingFilter.java";
  //    my $output = "testcases.txt";
  //    my %codes = ();
  //    my $folded = '';
  //
  //    open IN, "<:utf8", $file || die "Error opening input file '$file': $!";
  //    open OUT, ">:utf8", $output || die "Error opening output file '$output':
  //    $!";
  //
  //    while (my $line = <IN>) {
  //      chomp($line);
  //      # case '\u0133': // <char> <maybe URL> [ description ]
  //      if ($line =~ /case\s+'\\u(....)':.*\[([^\]]+)\]/) {
  //        my $code = $1;
  //        my $desc = $2;
  //        $codes{$code} = $desc;
  //      }
  //      # output[outputPos++] = 'A';
  //      elsif ($line =~ /output\[outputPos\+\+\] = '(.+)';/) {
  //        my $output_char = $1;
  //        $folded .= $output_char;
  //      }
  //      elsif ($line =~ /break;/ && length($folded) > 0) {
  //        my $first = 1;
  //        for my $code (sort { hex($a) <=> hex($b) } keys %codes) {
  //          my $desc = $codes{$code};
  //          print OUT '      ';
  //          print OUT '+ ' if (not $first);
  //          $first = 0;
  //          print OUT '"', chr(hex($code)), qq!"  // U+$code: $desc\n!;
  //        }
  //        print OUT qq!      ,"$folded", // Folded result\n\n!;
  //        %codes = ();
  //        $folded = '';
  //      }
  //    }
  //    close OUT;
  //
  //    ============== end get.test.cases.pl ==============
  //
  virtual void testAllFoldings() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestASCIIFoldingFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestASCIIFoldingFilter> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testEmptyTerm() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestASCIIFoldingFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestASCIIFoldingFilter> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestASCIIFoldingFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestASCIIFoldingFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
