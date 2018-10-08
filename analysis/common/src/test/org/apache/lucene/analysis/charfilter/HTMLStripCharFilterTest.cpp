using namespace std;

#include "HTMLStripCharFilterTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/HTMLStripCharFilter.h"

namespace org::apache::lucene::analysis::charfilter
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Analyzer> HTMLStripCharFilterTest::newTestAnalyzer()
{
  return make_shared<AnalyzerAnonymousInnerClass>();
}

HTMLStripCharFilterTest::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass()
{
}

shared_ptr<Analyzer::TokenStreamComponents>
HTMLStripCharFilterTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

shared_ptr<Reader>
HTMLStripCharFilterTest::AnalyzerAnonymousInnerClass::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<HTMLStripCharFilter>(reader);
}

void HTMLStripCharFilterTest::test() 
{
  wstring html = wstring(L"<div class=\"foo\">this is some text</div> here is "
                         L"a <a href=\"#bar\">link</a> and ") +
                 L"another <a href=\"http://lucene.apache.org/\">link</a>. " +
                 L"This is an entity: &amp; plus a &lt;.  Here is an &. <!-- "
                 L"is a comment -->";
  wstring gold = wstring(L"\nthis is some text\n here is a link and ") +
                 L"another link. " +
                 L"This is an entity: & plus a <.  Here is an &. ";
  assertHTMLStripsTo(html, gold, nullptr);
}

void HTMLStripCharFilterTest::testHTML() 
{
  shared_ptr<InputStream> stream =
      getClass().getResourceAsStream(L"htmlStripReaderTest.html");
  shared_ptr<HTMLStripCharFilter> reader = make_shared<HTMLStripCharFilter>(
      make_shared<InputStreamReader>(stream, StandardCharsets::UTF_8));
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  int ch = -1;
  while ((ch = reader->read()) != -1) {
    builder->append(static_cast<wchar_t>(ch));
  }
  wstring str = builder->toString();
  assertTrue(L"Entity not properly escaped",
             str.find(L"&lt;") == wstring::npos); // there is one > in the text
  assertTrue(L"Forrest should have been stripped out",
             str.find(L"forrest") == wstring::npos &&
                 str.find(L"Forrest") == wstring::npos);
  assertTrue(L"File should start with 'Welcome to Solr' after trimming",
             StringHelper::trim(str)->startsWith(L"Welcome to Solr"));

  assertTrue(L"File should start with 'Foundation.' after trimming",
             StringHelper::trim(str)->endsWith(L"Foundation."));
}

void HTMLStripCharFilterTest::testMSWord14GeneratedHTML() 
{
  shared_ptr<InputStream> stream =
      getClass().getResourceAsStream(L"MS-Word 14 generated.htm");
  shared_ptr<HTMLStripCharFilter> reader = make_shared<HTMLStripCharFilter>(
      make_shared<InputStreamReader>(stream, StandardCharsets::UTF_8));
  wstring gold = L"This is a test";
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  int ch = 0;
  while ((ch = reader->read()) != -1) {
    builder->append(static_cast<wchar_t>(ch));
  }
  // Compare trim()'d output to gold
  assertEquals(L"'" + builder->toString()->trim() + L"' is not equal to '" +
                   gold + L"'",
               gold, builder->toString()->trim());
}

void HTMLStripCharFilterTest::testGamma() 
{
  assertHTMLStripsTo(L"&Gamma;", L"\u0393",
                     unordered_set<>(Arrays::asList(L"reserved")));
}

void HTMLStripCharFilterTest::testEntities() 
{
  wstring test =
      L"&nbsp; &lt;foo&gt; &Uuml;bermensch &#61; &Gamma; bar &#x393;";
  wstring gold = L"  <foo> \u00DCbermensch = \u0393 bar \u0393";
  assertHTMLStripsTo(test, gold, unordered_set<>(Arrays::asList(L"reserved")));
}

void HTMLStripCharFilterTest::testMoreEntities() 
{
  wstring test = L"&nbsp; &lt;junk/&gt; &nbsp; &#33; &#64; and &#8217;";
  wstring gold = L"  <junk/>   ! @ and ’";
  assertHTMLStripsTo(test, gold, unordered_set<>(Arrays::asList(L"reserved")));
}

void HTMLStripCharFilterTest::testReserved() 
{
  wstring test = L"aaa bbb <reserved ccc=\"ddddd\"> eeee </reserved> ffff "
                 L"<reserved ggg=\"hhhh\"/> <other/>";
  shared_ptr<Set<wstring>> set = unordered_set<wstring>();
  set->add(L"reserved");
  shared_ptr<Reader> reader =
      make_shared<HTMLStripCharFilter>(make_shared<StringReader>(test), set);
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  int ch = 0;
  while ((ch = reader->read()) != -1) {
    builder->append(static_cast<wchar_t>(ch));
  }
  wstring result = builder->toString();
  // System.out.println("Result: " + result);
  assertTrue(L"Escaped tag not preserved: " +
                 to_wstring((int)result.find(L"reserved")),
             (int)result.find(L"reserved") == 9);
  assertTrue(L"Escaped tag not preserved: " +
                 to_wstring((int)result.find(L"reserved", 15)),
             (int)result.find(L"reserved", 15) == 38);
  assertTrue(L"Escaped tag not preserved: " +
                 to_wstring((int)result.find(L"reserved", 41)),
             (int)result.find(L"reserved", 41) == 54);
  assertTrue(L"Other tag should be removed",
             result.find(L"other") == wstring::npos);
}

void HTMLStripCharFilterTest::testMalformedHTML() 
{
  std::deque<wstring> testGold = {
      L"a <a hr<ef=aa<a>> </close</a>",
      L"a <a hr<ef=aa> </close",
      L"<a href=http://dmoz.org/cgi-bin/add.cgi?where=/arts/\" class=lu "
      L"style=\"font-size: 9px\" target=dmoz>Submit a Site</a>",
      L"Submit a Site",
      L"<a href=javascript:ioSwitch('p8','http://www.csmonitor.com/') "
      L"title=expand id=e8 class=expanded "
      L"rel=http://www.csmonitor.com/>Christian Science",
      L"Christian Science",
      L"<link rel=\"alternate\" type=\"application/rss+xml\" title=\"San "
      L"Francisco \" 2008 RSS Feed\" "
      L"href=\"http://2008.sf.wordcamp.org/feed/\" />",
      L"\n",
      L"<a href=\" "
      L"http://www.surgery4was.happyhost.org/"
      L"video-of-arthroscopic-knee-surgery symptoms.html, heat congestive "
      L"heart failure <a href=\" http://www.symptoms1bad.happyhost.org/canine",
      L"<a href=\" "
      L"http://www.surgery4was.happyhost.org/"
      L"video-of-arthroscopic-knee-surgery symptoms.html, heat congestive "
      L"heart failure <a href=\" http://www.symptoms1bad.happyhost.org/canine",
      L"<a "
      L"href=\"http://ucblibraries.colorado.edu/how/"
      L"index.htm\"class=\"pageNavAreaText\">",
      L"",
      L"<link title=\"^\\\" 21Sta's Blog\" rel=\"search\"  "
      L"type=\"application/opensearchdescription+xml\"  "
      L"href=\"http://21sta.com/blog/inc/opensearch.php\" />",
      L"\n",
      L"<a href=\"#postcomment\" title=\"\"Leave a comment\";\">?",
      L"?",
      L"<a href='/modern-furniture'   ' id='21txt' class='offtab'   "
      L"onMouseout=\"this.className='offtab';  return true;\" "
      L"onMouseover=\"this.className='ontab';  return true;\">",
      L"",
      L"<a href='http://alievi.wordpress.com/category/01-todos-posts/' "
      L"style='font-size: 275%; padding: 1px; margin: 1px;' title='01 - Todos "
      L"Post's (83)'>",
      L"",
      L"The <a href=<a "
      L"href=\"http://www.advancedmd.com>medical\">http://"
      L"www.advancedmd.com>medical</a> practice software</a>",
      L"The <a href=medical\">http://www.advancedmd.com>medical practice "
      L"software",
      L"<a href=\"node/21426\" class=\"clipTitle2\" title=\"Levi.com/BMX 2008 "
      L"Clip of the Week 29 \"Morgan Wade Leftover Clips\"\">Levi.com/BMX 2008 "
      L"Clip of the Week 29...",
      L"Levi.com/BMX 2008 Clip of the Week 29...",
      L"<a "
      L"href=\"printer_friendly.php?branch=&year=&submit=go&screen=\";\">"
      L"Printer Friendly",
      L"Printer Friendly",
      L"<a href=#\" ondragstart=\"return false\" "
      L"onclick=\"window.external.AddFavorite('http://"
      L"www.amazingtextures.com', 'Amazing Textures');return false\" "
      L"onmouseover=\"window.status='Add to Favorites';return true\">Add to "
      L"Favorites",
      L"Add to Favorites",
      L"<a "
      L"href=\"../at_home/at_home_search.html\"../_home/"
      L"at_home_search.html\">At",
      L"At",
      L"E-mail: <a href=\"\"mailto:XXXXXX@example.com\" \">XXXXXX@example.com "
      L"</a>",
      L"E-mail: XXXXXX@example.com ",
      L"<li class=\"farsi\"><a title=\"A'13?\" alt=\"A'13?\" "
      L"href=\"http://www.america.gov/persian\" alt=\"\" name=\"A'13?\"A'13? "
      L"title=\"A'13?\">A'13?</a></li>",
      L"\nA'13?\n",
      L"<li><a href=\"#28\" title=\"Hubert \"Geese\" Ausby\">Hubert \"Geese\" "
      L"Ausby</a></li>",
      L"\nHubert \"Geese\" Ausby\n",
      L"<href=\"http://anbportal.com/mms/login.asp\">",
      L"\n",
      L"<a href=\"",
      L"<a href=\"",
      L"<a href=\">",
      L"",
      L"<a rel=\"nofollow\" "
      L"href=\"http://anissanina31.skyrock.com/"
      L"1895039493-Hi-tout-le-monde.html\" title=\" Hi, tout le monde !>#</a>",
      L"#",
      L"<a href=\"http://annunciharleydavidsonusate.myblog.it/\" "
      L"title=\"Annunci Moto e Accessori Harley Davidson\" "
      L"target=\"_blank\"><img "
      L"src=\"http://annunciharleydavidsonusate.myblog.it/images/"
      L"Antipixel.gif\" /></a>",
      L"",
      L"<a href=\"video/addvideo&v=120838887181\" onClick=\"return "
      L"confirm('Are you sure you want  add this video to your profile? If it "
      L"exists some video in your profile will be overlapped by this "
      L"video!!')\" \" onmouseover=\"this.className='border2'\" "
      L"onmouseout=\"this.className=''\">",
      L"",
      L"<a href=#Services & Support>",
      L"",
      L"<input type=\"image\" "
      L"src=\"http://apologyindex.com/ThemeFiles/83401-72905/images/"
      L"btn_search.gif\"value=\"Search\" name=\"Search\" alt=\"Search\" "
      L"class=\"searchimage\" onclick=\"incom ='&sc=' + "
      L"document.getElementById('sel').value ; var dt ='&dt=' + "
      L"document.getElementById('dt').value; var searchKeyword = "
      L"document.getElementById('q').value ; searchKeyword = "
      L"searchKeyword.replace(/\\s/g,''); if (searchKeyword.length < "
      L"3){alert('Nothing to search. Search keyword should contain atleast 3 "
      L"chars.'); return false; } var al='&al=' +  "
      L"document.getElementById('advancedlink').style.display ;  "
      L"document.location.href='http://apologyindex.com/search.aspx?q=' + "
      L"document.getElementById('q').value + incom + dt + al;\" />",
      L"",
      L"<input type=\"image\" src=\"images/afbe.gif\" width=\"22\" "
      L"height=\"22\"  hspace=\"4\" title=\"Add to Favorite\" alt=\"Add to "
      L"Favorite\"onClick=\" if(window.sidebar){ "
      L"window.sidebar.addPanel(document.title,location.href,''); }else "
      L"if(window.external){ "
      L"window.external.AddFavorite(location.href,document.title); }else "
      L"if(window.opera&&window.print) { return true; }\">",
      L"",
      L"<area shape=\"rect\" coords=\"12,153,115,305\" "
      L"href=\"http://statenislandtalk.com/v-web/gallery/"
      L"Osmundsen-family\"Art's Norwegian Roots in Rogaland\">",
      L"\n",
      L"<a rel=\"nofollow\" "
      L"href=\"http://arth26.skyrock.com/660188240-bonzai.html\" "
      L"title=\"bonza>#",
      L"#",
      L"<a href=  >",
      L"",
      L"<ahref=http:..",
      L"<ahref=http:..",
      L"<ahref=http:..>",
      L"\n",
      L"<ahref=\"http://aseigo.bddf.ca/cms/1025\">A",
      L"\nA",
      L"<a "
      L"href=\"javascript:calendar_window=window.open('/"
      L"calendar.aspx?formname=frmCalendar.txtDate','calendar_window','width="
      L"154,height=188');calendar_window.focus()\">",
      L"",
      L"<a href=\"/applications/defenseaerospace/19+rackmounts\" title=\"19\" "
      L"Rackmounts\">",
      L"",
      L"<a "
      L"href=http://www.azimprimerie.fr/flash/backup/lewes-zip-code/"
      L"savage-model-110-manual.html title=savage model 110 manual "
      L"rel=dofollow>",
      L"",
      L"<a class=\"at\" name=\"Lamborghini  "
      L"href=\"http://lamborghini.coolbegin.com\">Lamborghini /a>",
      L"Lamborghini /a>",
      L"<A "
      L"href='newslink.php?news_link=http%3A%2F%2Fwww.worldnetdaily.com%"
      L"2Findex.php%3Ffa%3DPAGE.view%26pageId%3D85729&news_title=Florida QB "
      L"makes 'John 3:16' hottest Google search Tebow inscribed Bible "
      L"reference on eye black for championship game' TARGET=_blank>",
      L"",
      L"<a href=/myspace !style='color:#993333'>",
      L"",
      L"<meta name=3DProgId content=3DExcel.Sheet>",
      L"\n",
      L"<link id=3D\"shLink\" "
      L"href=3D\"PSABrKelly-BADMINTONCupResults08FINAL2008_09_19=_files/"
      L"sheet004.htm\">",
      L"\n",
      L"<td bgcolor=3D\"#FFFFFF\" nowrap>",
      L"\n",
      L"<a "
      L"href=\"http://basnect.info/usersearch/"
      L"\"predicciones-mundiales-2009\".html\">\"predicciones mundiales "
      L"2009\"</a>",
      L"\"predicciones mundiales 2009\"",
      L"<a class=\"comment-link\" "
      L"href=\"https://www.blogger.com/"
      L"comment.g?blogID=19402125&postID=114070605958684588\"location.href="
      L"https://www.blogger.com/"
      L"comment.g?blogID=19402125&postID=114070605958684588;>",
      L"",
      L"<a href = \"/videos/Bishop\"/\" title = \"click to see more Bishop\" "
      L"videos\">Bishop\"</a>",
      L"Bishop\"",
      L"<a "
      L"href=\"http://bhaa.ie/calendar/"
      L"event.php?eid=20081203150127531\"\">BHAA Eircom 2 &amp; 5 miles CC "
      L"combined start</a>",
      L"BHAA Eircom 2 & 5 miles CC combined start",
      L"<a href=\"http://people.tribe.net/wolfmana\" "
      L"onClick='setClick(\"Application[tribe].Person[bb7df210-9dc0-478c-917f-"
      L"436b896bcb79]\")'\" title=\"Mana\">",
      L"",
      L"<a  "
      L"href=\"http://blog.edu-cyberpg.com/"
      L"ct.ashx?id=6143c528-080c-4bb2-b765-5ec56c8256d3&url=http%3a%2f%2fwww."
      L"gsa.ac.uk%2fmackintoshsketchbook%2f\"\" eudora=\"autourl\">",
      L"",
      L"<input type=\"text\" value=\"<search here>\">",
      L"<input type=\"text\" value=\"\n\">",
      L"<input type=\"text\" value=\"<search here\">",
      L"<input type=\"text\" value=\"\n",
      L"<input type=\"text\" value=\"search here>\">",
      L"\">",
      L"<input type=\"text\" value=\"&lt;search here&gt;\" "
      L"onFocus=\"this.value='<search here>'\">",
      L"",
      L"<![if ! IE]>\n<link href=\"http://i.deviantart.com/icons/favicon.png\" "
      L"rel=\"shortcut icon\"/>\n<![endif]>",
      L"\n\n\n",
      L"<![if supportMisalignedColumns]>\n<tr height=0 "
      L"style='display:none'>\n<td width=64 "
      L"style='width:48pt'></td>\n</tr>\n<![endif]>",
      L"\n\n\n\n\n\n\n\n"};
  for (int i = 0; i < testGold.size(); i += 2) {
    assertHTMLStripsTo(testGold[i], testGold[i + 1], nullptr);
  }
}

void HTMLStripCharFilterTest::testBufferOverflow() 
{
  shared_ptr<StringBuilder> testBuilder = make_shared<StringBuilder>(
      HTMLStripCharFilter::getInitialBufferSize() + 50);
  testBuilder->append(L"ah<?> ??????");
  appendChars(testBuilder, HTMLStripCharFilter::getInitialBufferSize() + 500);
  shared_ptr<Reader> reader = make_shared<HTMLStripCharFilter>(
      make_shared<BufferedReader>(make_shared<StringReader>(
          testBuilder->toString()))); // force the use of BufferedReader
  assertHTMLStripsTo(reader, testBuilder->toString(), nullptr);

  testBuilder->setLength(0);
  testBuilder->append(L"<!--"); // comments
  appendChars(testBuilder, 3 * HTMLStripCharFilter::getInitialBufferSize() +
                               500); // comments have two lookaheads

  testBuilder->append(L"-->foo");
  wstring gold = L"foo";
  assertHTMLStripsTo(testBuilder->toString(), gold, nullptr);

  testBuilder->setLength(0);
  testBuilder->append(L"<?");
  appendChars(testBuilder, HTMLStripCharFilter::getInitialBufferSize() + 500);
  testBuilder->append(L"?>");
  gold = L"";
  assertHTMLStripsTo(testBuilder->toString(), gold, nullptr);

  testBuilder->setLength(0);
  testBuilder->append(L"<b ");
  appendChars(testBuilder, HTMLStripCharFilter::getInitialBufferSize() + 500);
  testBuilder->append(L"/>");
  gold = L"";
  assertHTMLStripsTo(testBuilder->toString(), gold, nullptr);
}

void HTMLStripCharFilterTest::appendChars(shared_ptr<StringBuilder> testBuilder,
                                          int numChars)
{
  int i1 = numChars / 2;
  for (int i = 0; i < i1; i++) {
    testBuilder->append(L'a')->append(
        L' '); // tack on enough to go beyond the mark readahead limit, since
               // <?> makes HTMLStripCharFilter think it is a processing
               // instruction
  }
}

void HTMLStripCharFilterTest::testComment() 
{
  wstring test = L"<!--- three dashes, still a valid comment ---> ";
  wstring gold = L" ";
  assertHTMLStripsTo(test, gold, nullptr);

  test = L"<! -- blah > "; // should not be recognized as a comment
  gold = L" ";
  assertHTMLStripsTo(test, gold, nullptr);

  shared_ptr<StringBuilder> testBuilder = make_shared<StringBuilder>(L"<!--");
  appendChars(testBuilder, TestUtil::nextInt(random(), 0, 1000));
  gold = L"";
  assertHTMLStripsTo(testBuilder->toString(), gold, nullptr);
}

void HTMLStripCharFilterTest::doTestOffsets(const wstring &in_) throw(
    runtime_error)
{
  shared_ptr<HTMLStripCharFilter> reader = make_shared<HTMLStripCharFilter>(
      make_shared<BufferedReader>(make_shared<StringReader>(in_)));
  int ch = 0;
  int off = 0;     // offset in the reader
  int strOff = -1; // offset in the original string
  while ((ch = reader->read()) != -1) {
    int correctedOff = reader->correctOffset(off);

    if (ch == L'X') {
      strOff = (int)in_.find(L'X', strOff + 1);
      TestUtil::assertEquals(strOff, correctedOff);
    }

    off++;
  }
}

void HTMLStripCharFilterTest::testOffsets() 
{
  //    doTestOffsets("hello X how X are you");
  doTestOffsets(L"hello <p> X<p> how <p>X are you");
  doTestOffsets(L"X &amp; X &#40; X &lt; &gt; X");

  // test backtracking
  doTestOffsets(L"X < &zz >X &# < X > < &l > &g < X");
}

void HTMLStripCharFilterTest::assertLegalOffsets(const wstring &in_) throw(
    runtime_error)
{
  int length = in_.length();
  shared_ptr<HTMLStripCharFilter> reader = make_shared<HTMLStripCharFilter>(
      make_shared<BufferedReader>(make_shared<StringReader>(in_)));
  int ch = 0;
  int off = 0;
  while ((ch = reader->read()) != -1) {
    int correction = reader->correctOffset(off);
    assertTrue(L"invalid offset correction: " + to_wstring(off) + L"->" +
                   to_wstring(correction) + L" for doc of length: " +
                   to_wstring(length),
               correction <= length);
    off++;
  }
}

void HTMLStripCharFilterTest::testLegalOffsets() 
{
  assertLegalOffsets(L"hello world");
  assertLegalOffsets(L"hello &#x world");
}

void HTMLStripCharFilterTest::testRandom() 
{
  int numRounds = RANDOM_MULTIPLIER * 1000;
  shared_ptr<Analyzer> a = newTestAnalyzer();
  checkRandomData(random(), a, numRounds);
  delete a;
}

void HTMLStripCharFilterTest::testRandomHugeStrings() 
{
  int numRounds = RANDOM_MULTIPLIER * 100;
  shared_ptr<Analyzer> a = newTestAnalyzer();
  checkRandomData(random(), a, numRounds, 8192);
  delete a;
}

void HTMLStripCharFilterTest::testCloseBR() 
{
  shared_ptr<Analyzer> a = newTestAnalyzer();
  checkAnalysisConsistency(random(), a, random()->nextBoolean(),
                           L" Secretary)</br> [[M");
  delete a;
}

void HTMLStripCharFilterTest::testServerSideIncludes() 
{
  wstring test =
      wstring(L"one<img src=\"image.png\"\n") +
      L" alt =  \"Alt: <!--#echo "
      L"var='${IMAGE_CAPTION:<!--comment-->\\'Comment\\'}'  -->\"\n\n" +
      L" title=\"Title: <!--#echo var=\"IMAGE_CAPTION\"-->\">two";
  wstring gold = L"onetwo";
  assertHTMLStripsTo(test, gold, nullptr);

  test = L"one<script><!-- <!--#config comment=\"<!-- \\\"comment\\\"-->\"--> "
         L"--></script>two";
  gold = L"one\ntwo";
  assertHTMLStripsTo(test, gold, nullptr);
}

void HTMLStripCharFilterTest::testScriptQuotes() 
{
  wstring test = L"one<script attr= bare><!-- action('<!-- comment -->', "
                 L"\"\\\"-->\\\"\"); --></script>two";
  wstring gold = L"one\ntwo";
  assertHTMLStripsTo(test, gold, nullptr);

  test = L"hello<script><!-- f('<!--internal--></script>'); --></script>";
  gold = L"hello\n";
  assertHTMLStripsTo(test, gold, nullptr);
}

void HTMLStripCharFilterTest::testEscapeScript() 
{
  wstring test = L"one<script no-value-attr>callSomeMethod();</script>two";
  wstring gold = L"one<script no-value-attr></script>two";
  shared_ptr<Set<wstring>> escapedTags =
      unordered_set<wstring>(Arrays::asList(L"SCRIPT"));
  assertHTMLStripsTo(test, gold, escapedTags);
}

void HTMLStripCharFilterTest::testStyle() 
{
  wstring test =
      wstring(L"one<style type=\"text/css\">\n") + L"<!--\n" +
      L"@import url('http://www.lasletrasdecanciones.com/css.css');\n" +
      L"-->\n" + L"</style>two";
  wstring gold = L"one\ntwo";
  assertHTMLStripsTo(test, gold, nullptr);
}

void HTMLStripCharFilterTest::testEscapeStyle() 
{
  wstring test = L"one<style type=\"text/css\"> body,font,a { "
                 L"font-family:arial; } </style>two";
  wstring gold = L"one<style type=\"text/css\"></style>two";
  shared_ptr<Set<wstring>> escapedTags =
      unordered_set<wstring>(Arrays::asList(L"STYLE"));
  assertHTMLStripsTo(test, gold, escapedTags);
}

void HTMLStripCharFilterTest::testBR() 
{
  std::deque<wstring> testGold = {L"one<BR />two<br>three", L"one\ntwo\nthree",
                                   L"one<BR some stuff here too>two</BR>",
                                   L"one\ntwo\n"};
  for (int i = 0; i < testGold.size(); i += 2) {
    assertHTMLStripsTo(testGold[i], testGold[i + 1], nullptr);
  }
}

void HTMLStripCharFilterTest::testEscapeBR() 
{
  wstring test = L"one<BR class='whatever'>two</\nBR\n>";
  wstring gold = L"one<BR class='whatever'>two</\nBR\n>";
  shared_ptr<Set<wstring>> escapedTags =
      unordered_set<wstring>(Arrays::asList(L"BR"));
  assertHTMLStripsTo(test, gold, escapedTags);
}

void HTMLStripCharFilterTest::testInlineTagsNoSpace() 
{
  wstring test =
      L"one<sPAn class=\"invisible\">two<sup>2<sup>e</sup></sup>.</SpaN>three";
  wstring gold = L"onetwo2e.three";
  assertHTMLStripsTo(test, gold, nullptr);
}

void HTMLStripCharFilterTest::testCDATA() 
{
  int maxNumElems = 100;
  wstring randomHtmlishString1 =
      TestUtil::randomHtmlishString(random(), maxNumElems)
          .replaceAll(L">", L" ")
          .replaceFirst(L"^--", L"__");
  wstring closedAngleBangNonCDATA =
      L"<!" + randomHtmlishString1 + L"-[CDATA[&]]>";

  wstring randomHtmlishString2 =
      TestUtil::randomHtmlishString(random(), maxNumElems)
          .replaceAll(L">", L" ")
          .replaceFirst(L"^--", L"__");
  wstring unclosedAngleBangNonCDATA =
      L"<!" + randomHtmlishString1 + L"-[CDATA[";

  std::deque<wstring> testGold = {
      L"one<![CDATA[<one><two>three<four></four></two></one>]]>two",
      L"one<one><two>three<four></four></two></one>two",
      L"one<![CDATA[two<![CDATA[three]]]]><![CDATA[>four]]>five",
      L"onetwo<![CDATA[three]]>fourfive",
      L"<! [CDATA[&]]>",
      L"",
      L"<! [CDATA[&] ] >",
      L"",
      L"<! [CDATA[&]]",
      L"<! [CDATA[&]]",
      L"<!\u2009[CDATA[&]]>",
      L"",
      L"<!\u2009[CDATA[&]\u2009]\u2009>",
      L"",
      L"<!\u2009[CDATA[&]\u2009]\u2009",
      L"<!\u2009[CDATA[&]\u2009]\u2009",
      closedAngleBangNonCDATA,
      L"",
      L"<![CDATA[",
      L"",
      L"<![CDATA[<br>",
      L"<br>",
      L"<![CDATA[<br>]]",
      L"<br>]]",
      L"<![CDATA[<br>]]>",
      L"<br>",
      L"<![CDATA[<br>] ] >",
      L"<br>] ] >",
      L"<![CDATA[<br>]\u2009]\u2009>",
      L"<br>]\u2009]\u2009>",
      L"<!\u2009[CDATA[",
      L"<!\u2009[CDATA[",
      unclosedAngleBangNonCDATA,
      unclosedAngleBangNonCDATA};
  for (int i = 0; i < testGold.size(); i += 2) {
    assertHTMLStripsTo(testGold[i], testGold[i + 1], nullptr);
  }
}

void HTMLStripCharFilterTest::testUnclosedAngleBang() 
{
  assertHTMLStripsTo(L"<![endif]", L"<![endif]", nullptr);
}

void HTMLStripCharFilterTest::testUppercaseCharacterEntityVariants() throw(
    runtime_error)
{
  wstring test = L" &QUOT;-&COPY;&GT;>&LT;<&REG;&AMP;";
  wstring gold = L" \"-\u00A9>><<\u00AE&";
  assertHTMLStripsTo(test, gold, nullptr);
}

void HTMLStripCharFilterTest::testMSWordMalformedProcessingInstruction() throw(
    runtime_error)
{
  wstring test = L"one<?xml:namespace prefix = o ns = "
                 L"\"urn:schemas-microsoft-com:office:office\" />two";
  wstring gold = L"onetwo";
  assertHTMLStripsTo(test, gold, nullptr);
}

void HTMLStripCharFilterTest::testSupplementaryCharsInTags() throw(
    runtime_error)
{
  wstring test = L"one<𩬅艱鍟䇹愯瀛>two<瀛愯𩬅>three "
                 L"瀛愯𩬅</瀛愯𩬅>four</𩬅艱鍟䇹愯瀛>five<𠀀𠀀>six<𠀀𠀀/>seven";
  wstring gold = L"one\ntwo\nthree 瀛愯𩬅\nfour\nfive\nsix\nseven";
  assertHTMLStripsTo(test, gold, nullptr);
}

void HTMLStripCharFilterTest::testRandomBrokenHTML() 
{
  int maxNumElements = 10000;
  wstring text = TestUtil::randomHtmlishString(random(), maxNumElements);
  shared_ptr<Analyzer> a = newTestAnalyzer();
  checkAnalysisConsistency(random(), a, random()->nextBoolean(), text);
  delete a;
}

void HTMLStripCharFilterTest::testRandomText() 
{
  shared_ptr<StringBuilder> text = make_shared<StringBuilder>();
  int minNumWords = 10;
  int maxNumWords = 10000;
  int minWordLength = 3;
  int maxWordLength = 20;
  int numWords = TestUtil::nextInt(random(), minNumWords, maxNumWords);
  switch (TestUtil::nextInt(random(), 0, 4)) {
  case 0: {
    for (int wordNum = 0; wordNum < numWords; ++wordNum) {
      text->append(TestUtil::randomUnicodeString(random(), maxWordLength));
      text->append(L' ');
    }
    break;
  }
  case 1: {
    for (int wordNum = 0; wordNum < numWords; ++wordNum) {
      text->append(TestUtil::randomRealisticUnicodeString(
          random(), minWordLength, maxWordLength));
      text->append(L' ');
    }
    break;
  }
  default: { // ASCII 50% of the time
    for (int wordNum = 0; wordNum < numWords; ++wordNum) {
      text->append(TestUtil::randomSimpleString(random()));
      text->append(L' ');
    }
  }
  }
  shared_ptr<Reader> reader = make_shared<HTMLStripCharFilter>(
      make_shared<StringReader>(text->toString()));
  while (reader->read() != -1) {
    ;
  }
}

void HTMLStripCharFilterTest::testUTF16Surrogates() 
{
  shared_ptr<Analyzer> analyzer = newTestAnalyzer();
  // Paired surrogates
  assertAnalyzesTo(analyzer, L" one two &#xD86C;&#XdC01;three",
                   std::deque<wstring>{L"one", L"two", L"\uD86C\uDC01three"});
  assertAnalyzesTo(analyzer, L" &#55404;&#XdC01;",
                   std::deque<wstring>{L"\uD86C\uDC01"});
  assertAnalyzesTo(analyzer, L" &#xD86C;&#56321;",
                   std::deque<wstring>{L"\uD86C\uDC01"});
  assertAnalyzesTo(analyzer, L" &#55404;&#56321;",
                   std::deque<wstring>{L"\uD86C\uDC01"});

  // Improperly paired surrogates
  assertAnalyzesTo(analyzer, L" &#55404;&#57999;",
                   std::deque<wstring>{L"\uFFFD\uE28F"});
  assertAnalyzesTo(analyzer, L" &#xD86C;&#57999;",
                   std::deque<wstring>{L"\uFFFD\uE28F"});
  assertAnalyzesTo(analyzer, L" &#55002;&#XdC01;",
                   std::deque<wstring>{L"\uD6DA\uFFFD"});
  assertAnalyzesTo(analyzer, L" &#55002;&#56321;",
                   std::deque<wstring>{L"\uD6DA\uFFFD"});

  // Unpaired high surrogates
  assertAnalyzesTo(analyzer, L" &#Xd921;", std::deque<wstring>{L"\uFFFD"});
  assertAnalyzesTo(analyzer, L" &#Xd921", std::deque<wstring>{L"\uFFFD"});
  assertAnalyzesTo(analyzer, L" &#Xd921<br>", std::deque<wstring>{L"&#Xd921"});
  assertAnalyzesTo(analyzer, L" &#55528;", std::deque<wstring>{L"\uFFFD"});
  assertAnalyzesTo(analyzer, L" &#55528", std::deque<wstring>{L"\uFFFD"});
  assertAnalyzesTo(analyzer, L" &#55528<br>", std::deque<wstring>{L"&#55528"});

  // Unpaired low surrogates
  assertAnalyzesTo(analyzer, L" &#xdfdb;", std::deque<wstring>{L"\uFFFD"});
  assertAnalyzesTo(analyzer, L" &#xdfdb", std::deque<wstring>{L"\uFFFD"});
  assertAnalyzesTo(analyzer, L" &#xdfdb<br>", std::deque<wstring>{L"&#xdfdb"});
  assertAnalyzesTo(analyzer, L" &#57209;", std::deque<wstring>{L"\uFFFD"});
  assertAnalyzesTo(analyzer, L" &#57209", std::deque<wstring>{L"\uFFFD"});
  assertAnalyzesTo(analyzer, L" &#57209<br>", std::deque<wstring>{L"&#57209"});
  delete analyzer;
}

void HTMLStripCharFilterTest::assertHTMLStripsTo(
    const wstring &input, const wstring &gold,
    shared_ptr<Set<wstring>> escapedTags) 
{
  assertHTMLStripsTo(make_shared<StringReader>(input), gold, escapedTags);
}

void HTMLStripCharFilterTest::assertHTMLStripsTo(
    shared_ptr<Reader> input, const wstring &gold,
    shared_ptr<Set<wstring>> escapedTags) 
{
  shared_ptr<HTMLStripCharFilter> reader;
  if (nullptr == escapedTags) {
    reader = make_shared<HTMLStripCharFilter>(input);
  } else {
    reader = make_shared<HTMLStripCharFilter>(input, escapedTags);
  }
  int ch = 0;
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  try {
    while ((ch = reader->read()) != -1) {
      builder->append(static_cast<wchar_t>(ch));
    }
  } catch (const runtime_error &e) {
    if (gold == builder->toString()) {
      throw e;
    }
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new Exception("('" + builder.toString() + "' is not
    // equal to '" + gold + "').  " + e.getMessage(), e);
    throw runtime_error(L"('" + builder->toString() + L"' is not equal to '" +
                        gold + L"').  " + e.what());
  }
  assertEquals(L"'" + builder->toString() + L"' is not equal to '" + gold +
                   L"'",
               gold, builder->toString());
}
} // namespace org::apache::lucene::analysis::charfilter