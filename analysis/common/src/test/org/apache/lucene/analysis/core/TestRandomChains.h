#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Tokenizer.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include  "core/src/java/org/apache/lucene/analysis/CharFilter.h"

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
namespace org::apache::lucene::analysis::core
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharFilter = org::apache::lucene::analysis::CharFilter;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

/** tests random analysis chains */
class TestRandomChains : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestRandomChains)

public:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: static java.util.List<Constructor<? extends
  // org.apache.lucene.analysis.Tokenizer>> tokenizers;
  static std::deque < Constructor < ? extends Tokenizer >> tokenizers;
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: static java.util.List<Constructor<? extends
  // org.apache.lucene.analysis.TokenFilter>> tokenfilters;
  static std::deque < Constructor < ? extends TokenFilter >> tokenfilters;
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: static java.util.List<Constructor<? extends
  // org.apache.lucene.analysis.CharFilter>> charfilters;
  static std::deque < Constructor < ? extends CharFilter >> charfilters;

private:
  static const std::function<bool(std::any[] *)> ALWAYS;

  static const std::shared_ptr<Set<std::type_info>> avoidConditionals;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static TestRandomChains::StaticConstructor staticConstructor;

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private static final
  // java.util.Map<Constructor<?>,java.util.function.Predicate<Object[]>>
  // brokenConstructors = new java.util.HashMap<>();
  static const std::unordered_map < Constructor < ? >,
      std::function<bool(std::any[] *)>> brokenConstructors;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass()
  static void afterClass();

  /** Hack to work around the stupidness of Oracle's strict Java backwards
   * compatibility.
   * {@code Class<T>#getConstructors()} should return unmodifiable {@code
   * List<Constructor<T>>} not array! */
private:
  template <typename T, typename T1>
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") private static <T>
  // Constructor<T> castConstructor(Class<T> instanceClazz, Constructor<?> ctor)
  static std::shared_ptr<Constructor<T>>
  castConstructor(std::type_info<T> &instanceClazz,
                  std::shared_ptr<Constructor<T1>> ctor);

public:
  static std::deque<std::type_info>
  getClassesForPackage(const std::wstring &pckgname) ;

private:
  static void collectClassesForPackage(
      const std::wstring &pckgname,
      std::deque<std::type_info> &classes) ;

  static const std::unordered_map<std::type_info,
                                  std::function<std::any *(Random *)>>
      argProducers;

private:
  class IdentityHashMapAnonymousInnerClass
      : public IdentityHashMap<std::type_info,
                               std::function<std::any *(Random *)>>
  {
    GET_CLASS_NAME(IdentityHashMapAnonymousInnerClass)
  public:
    IdentityHashMapAnonymousInnerClass(std::shared_ptr<UnknownType> java);

    //      {
    //    put(int.class, random ->
    //    {
    GET_CLASS_NAME(, )
    //        return Integer.valueOf(TestUtil.nextInt(random, -50, 50));
    //    }
    //   );
    //    put(char.class, random ->
    //    {
    GET_CLASS_NAME(, )
    //        while(true)
    //        {
    //            char c = (char)random.nextInt(65536);
    //            if (c < '\uD800' || c > '\uDFFF')
    //            {
    //                return Character.valueOf(c);
    //            }
    //        }
    //    }
    //   );
    //    put(float.class, Random::nextFloat);
    //    put(bool.class, Random::nextBoolean);
    //    put(byte.class, random -> (byte) random.nextInt(256));
    //    put(byte[].class, random ->
    //    {
    //        byte bytes[] = new byte[random.nextInt(256)];
    //        random.nextBytes(bytes);
    //        return bytes;
    //    }
    //   );
    //    put(Random.class, random -> new Random(random.nextLong()));
    //    put(Version.class, random -> Version.LATEST);
    //    put(AttributeFactory.class,
    //    BaseTokenStreamTestCase::newAttributeFactory); put(Set.class,random ->
    //    {
    //        Set<std::wstring> set = new HashSet<>();
    //        int num = random.nextInt(5);
    //        for (int i = 0; i < num; i++)
    //        {
    //            set.add(StandardTokenizer.TOKEN_TYPES[random.nextInt(StandardTokenizer.TOKEN_TYPES.length)]);
    //        }
    //        return set;
    //    }
    //   );
    //    put(std::deque.class, random ->
    //    {
    GET_CLASS_NAME(, )
    //        std::deque<char[]> col = new ArrayList<>();
    //        int num = random.nextInt(5);
    //        for (int i = 0; i < num; i++)
    //        {
    //            col.add(TestUtil.randomSimpleString(random).toCharArray());
    //        }
    //        return col;
    //    }
    //   );
    //    put(CharArraySet.class, random ->
    //    {
    GET_CLASS_NAME(, )
    //        int num = random.nextInt(10);
    //        CharArraySet set = new CharArraySet(num, random.nextBoolean());
    //        for (int i = 0; i < num; i++)
    //        {
    //            set.add(TestUtil.randomSimpleString(random));
    //        }
    //        return set;
    //    }
    //   );
    //    // TODO: don't want to make the exponentially slow ones Dawid
    //    documents
    //    // in TestPatternReplaceFilter, so dont use truly random patterns (for
    //    now) put(Pattern.class, random -> Pattern.compile("a"));
    //    put(Pattern[].class, random -> new Pattern[]
    //    {Pattern.compile("([a-z]+)"), Pattern.compile("([0-9]+)")});
    GET_CLASS_NAME(, )
    //    put(PayloadEncoder.class, random -> new IdentityEncoder()); // the
    //    other encoders will throw exceptions if tokens arent numbers?
    //    put(Dictionary.class, random ->
    //    {
    //        InputStream affixStream =
    //        TestHunspellStemFilter.class.getResourceAsStream("simple.aff");
    //        InputStream dictStream =
    //        TestHunspellStemFilter.class.getResourceAsStream("simple.dic");
    //        try
    //        {
    //            return new Dictionary(new RAMDirectory(), "dictionary",
    //            affixStream, dictStream);
    //        }
    //        catch (Exception ex)
    //        {
    //            Rethrow.rethrow(ex);
    //            return nullptr;
    //        }
    //    }
    //   );
    //    put(HyphenationTree.class, random ->
    //    {
    GET_CLASS_NAME(, )
    //        try
    //        {
    //            InputSource is = new
    //            InputSource(TestCompoundWordTokenFilter.class.getResource("da_UTF8.xml").toExternalForm());
    //            HyphenationTree hyphenator =
    //            HyphenationCompoundWordTokenFilter.getHyphenationTree(is);
    //            return hyphenator;
    //        }
    //        catch (Exception ex)
    //        {
    //            Rethrow.rethrow(ex);
    //            return nullptr;
    //        }
    //    }
    //   );
    //    put(SnowballProgram.class, random ->
    //    {
    GET_CLASS_NAME(, )
    //        try
    //        {
    //            std::wstring lang =
    //            TestSnowball.SNOWBALL_LANGS[random.nextInt(TestSnowball.SNOWBALL_LANGS.length)];
    //            Class clazz = Class.forName("org.tartarus.snowball.ext." +
    //            lang + "Stemmer").asSubclass(SnowballProgram.class); return
    //            clazz.newInstance();
    //        }
    //        catch (Exception ex)
    //        {
    //            Rethrow.rethrow(ex);
    //            return nullptr;
    //        }
    //    }
    //   );
    //    put(std::wstring.class, random ->
    //    {
    GET_CLASS_NAME(, )
    //        if (random.nextBoolean())
    //        {
    //            return
    //            StandardTokenizer.TOKEN_TYPES[random.nextInt(StandardTokenizer.TOKEN_TYPES.length)];
    //        }
    //        else
    //        {
    //            return TestUtil.randomSimpleString(random);
    //        }
    //    }
    //   );
    //    put(NormalizeCharMap.class, random ->
    //    {
    GET_CLASS_NAME(, )
    //        NormalizeCharMap.Builder builder = new NormalizeCharMap.Builder();
    //        Set<std::wstring> keys = new HashSet<>();
    //        int num = random.nextInt(5);
    //        for (int i = 0; i < num; i++)
    //        {
    //            std::wstring key = TestUtil.randomSimpleString(random);
    //            if (!keys.contains(key) && key.length() > 0)
    //            {
    //                std::wstring value = TestUtil.randomSimpleString(random);
    //                builder.add(key, value);
    //                keys.add(key);
    //            }
    //        }
    //        return builder.build();
    //    }
    //   );
    //    put(CharacterRunAutomaton.class, random ->
    //    {
    GET_CLASS_NAME(, )
    //        switch(random.nextInt(5))
    //        {
    //            case 0:
    //                return MockTokenizer.KEYWORD;
    //            case 1:
    //                return MockTokenizer.SIMPLE;
    //            case 2:
    //                return MockTokenizer.WHITESPACE;
    //            case 3:
    //                return MockTokenFilter.EMPTY_STOPSET;
    //            default:
    //                return MockTokenFilter.ENGLISH_STOPSET;
    //        }
    //    }
    //   );
    //    put(CharArrayMap.class, random ->
    //    {
    GET_CLASS_NAME(, )
    //        int num = random.nextInt(10);
    //        CharArrayMap<std::wstring> map_obj = new CharArrayMap<>(num,
    //        random.nextBoolean()); for (int i = 0; i < num; i++)
    //        {
    //            map_obj.put(TestUtil.randomSimpleString(random),
    //            TestUtil.randomSimpleString(random));
    //        }
    //        return map_obj;
    //    }
    //   );
    //    put(StemmerOverrideMap.class, random ->
    //    {
    GET_CLASS_NAME(, )
    //        int num = random.nextInt(10);
    //        StemmerOverrideFilter.Builder builder = new
    //        StemmerOverrideFilter.Builder(random.nextBoolean()); for (int i =
    //        0; i < num; i++)
    //        {
    //            std::wstring input = "";
    //            do
    //            {
    //                input = TestUtil.randomRealisticUnicodeString(random);
    //            } while(input.isEmpty());
    //            std::wstring out = "";
    //            TestUtil.randomSimpleString(random);
    //            do
    //            {
    //                out = TestUtil.randomRealisticUnicodeString(random);
    //            } while(out.isEmpty());
    //            builder.add(input, out);
    //        }
    //        try
    //        {
    //            return builder.build();
    //        }
    //        catch (Exception ex)
    //        {
    //            Rethrow.rethrow(ex);
    //            return nullptr;
    //        }
    //    }
    //   );
    //    put(SynonymMap.class, new Function<Random, Object>()
    //    {
    GET_CLASS_NAME(, )
    //      @@Override public Object apply(Random random)
    //      {
    //        SynonymMap.Builder b = new
    //        SynonymMap.Builder(random.nextBoolean()); final int numEntries =
    //        atLeast(10); for (int j = 0; j < numEntries; j++)
    //        {
    //          addSyn(b, randomNonEmptyString(random),
    //          randomNonEmptyString(random), random.nextBoolean());
    //        }
    //        try
    //        {
    //          return b.build();
    //        }
    //        catch (Exception ex)
    //        {
    //          Rethrow.rethrow(ex);
    //          return nullptr; // unreachable code
    //        }
    //      }
    //
    //      private void addSyn(SynonymMap.Builder b, std::wstring input, std::wstring
    //      output, bool keepOrig)
    //      {
    //        b.add(new CharsRef(input.replaceAll(" +", "\u0000")), new
    //        CharsRef(output.replaceAll(" +", "\u0000")), keepOrig);
    //      }
    //
    //      private std::wstring randomNonEmptyString(Random random)
    //      {
    //        while(true)
    //        {
    //          final std::wstring s = TestUtil.randomUnicodeString(random).trim();
    //          if (s.length() != 0 && s.indexOf('\u0000') == -1)
    //          {
    //            return s;
    //          }
    //        }
    //      }
    //    }
    //   );
    //    put(DateFormat.class, random ->
    //    {
    GET_CLASS_NAME(, )
    //        if (random.nextBoolean())
    //            return nullptr;
    //        return DateFormat.getDateInstance(DateFormat.DEFAULT,
    //        randomLocale(random));
    //    }
    //   );
    //    put(Automaton.class, random ->
    //    {
    GET_CLASS_NAME(, )
    //        return Operations.determinize(new
    //        RegExp(AutomatonTestUtil.randomRegexp(random),
    //        RegExp.NONE).toAutomaton(),
    //        Operations.DEFAULT_MAX_DETERMINIZED_STATES);
    //    }
    //   );
    //  }
    //  }
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    //
    //  static final Set<Class> allowedTokenizerArgs, allowedTokenFilterArgs,
    //  allowedCharFilterArgs;
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    // ignore
    //
    //  @@SuppressWarnings("unchecked") static <T> T newRandomArg(Random random,
    //  Class<T> paramType)
    //  {
    //    final Function<Random,Object> producer = argProducers.get(paramType);
    //    assertNotNull("No producer for arguments of type " +
    //    paramType.getName() + " found", producer); return (T)
    //    producer.apply(random);
    //  }
    //
    //  static Object[] newTokenizerArgs(Random random, Class[] paramTypes)
    //  {
    //    Object[] args = new Object[paramTypes.length];
    //    for (int i = 0; i < args.length; i++)
    //    {
    //      Class paramType = paramTypes[i];
    //      if (paramType == AttributeSource.class)
    //      {
GET_CLASS_NAME())
    //        // TODO: args[i] = new AttributeSource();
    //        // this is currently too scary to deal with!
    //        args[i] = nullptr; // force IAE
    //      }
    //      else
    //      {
    //        args[i] = newRandomArg(random, paramType);
    //      }
    //    }
    //    return args;
    //  }
    //
    //  static Object[] newCharFilterArgs(Random random, Reader reader, Class[] paramTypes)
    //  {
    //    Object[] args = new Object[paramTypes.length];
    //    for (int i = 0; i < args.length; i++)
    //    {
    //      Class paramType = paramTypes[i];
    //      if (paramType == Reader.class)
    //      {
GET_CLASS_NAME())
    //        args[i] = reader;
    //      }
    //      else
    //      {
    //        args[i] = newRandomArg(random, paramType);
    //      }
    //    }
    //    return args;
    //  }
    //
    //  static Object[] newFilterArgs(Random random, TokenStream stream, Class[] paramTypes)
    //  {
    //    Object[] args = new Object[paramTypes.length];
    //    for (int i = 0; i < args.length; i++)
    //    {
    //      Class paramType = paramTypes[i];
    //      if (paramType == TokenStream.class)
    //      {
GET_CLASS_NAME())
    //        args[i] = stream;
    //      }
    //      else if (paramType == CommonGramsFilter.class)
    //      {
GET_CLASS_NAME())
    //        // TODO: fix this one, thats broken: CommonGramsQueryFilter takes this one explicitly
    //        args[i] = new CommonGramsFilter(stream, newRandomArg(random, CharArraySet.class));
    //      }
    //      else
    //      {
    //        args[i] = newRandomArg(random, paramType);
    //      }
    //    }
    //    return args;
    //  }
    //
    //  static class MockRandomAnalyzer extends Analyzer
    //  {
GET_CLASS_NAME(MockRandomAnalyzer)
    //    final long seed;
    //
    //    MockRandomAnalyzer(long seed)
    //    {
    //      this.seed = seed;
    //    }
    //
    //    @@Override protected TokenStreamComponents createComponents(std::wstring fieldName)
    //    {
    //      Random random = new Random(seed);
    //      TokenizerSpec tokenizerSpec = newTokenizer(random);
    //      //System.out.println("seed=" + seed + ",create tokenizer=" + tokenizerSpec.toString);
    //      TokenFilterSpec filterSpec = newFilterChain(random, tokenizerSpec.tokenizer);
    //      //System.out.println("seed=" + seed + ",create filter=" + filterSpec.toString);
    //      return new TokenStreamComponents(tokenizerSpec.tokenizer, filterSpec.stream);
    //    }
    //
    //    @@Override protected Reader initReader(std::wstring fieldName, Reader reader)
    //    {
    //      Random random = new Random(seed);
    //      CharFilterSpec charfilterspec = newCharFilterChain(random, reader);
    //      return charfilterspec.reader;
    //    }
    //
    //    @@Override public std::wstring toString()
    //    {
    //      Random random = new Random(seed);
    //      StringBuilder sb = new StringBuilder();
    //      CharFilterSpec charFilterSpec = newCharFilterChain(random, new StringReader(""));
    //      sb.append("\ncharfilters=");
    //      sb.append(charFilterSpec.toString);
    //      // intentional: initReader gets its own separate random
    //      random = new Random(seed);
    //      TokenizerSpec tokenizerSpec = newTokenizer(random);
    //      sb.append("\n");
    //      sb.append("tokenizer=");
    //      sb.append(tokenizerSpec.toString);
    //      TokenFilterSpec tokenFilterSpec = newFilterChain(random, tokenizerSpec.tokenizer);
    //      sb.append("\n");
    //      sb.append("filters=");
    //      sb.append(tokenFilterSpec.toString);
    //      return sb.toString();
    //    }
    //
    //    private <T> T createComponent(Constructor<T> ctor, Object[] args, StringBuilder descr, bool isConditional)
    //    {
    //      try
    //      {
    //        final T instance = ctor.newInstance(args);
    // /*
    // if (descr.length() > 0) {
    //   descr.append(",");
    // }
    // */
    //        descr.append("\n  ");
    //        if (isConditional)
    //        {
    //          descr.append("Conditional:");
    //        }
    //        descr.append(ctor.getDeclaringClass().getName());
    //        std::wstring params = Arrays.deepToString(args);
    //        params = params.substring(1, (params.length()-1) - 1);
    //        descr.append("(").append(params).append(")");
    //        return instance;
    //      }
    //      catch (InvocationTargetException ite)
    //      {
    //        final Throwable cause = ite.getCause();
    //        if (cause instanceof IllegalArgumentException || cause instanceof UnsupportedOperationException)
    //    {
    //          // thats ok, ignore
    //          if (VERBOSE)
    //          {
    //            System.err.println("Ignoring IAE/UOE from ctor:");
    //            cause.printStackTrace(System.err);
    //          }
    //        }
    //        else
    //        {
    //          Rethrow.rethrow(cause);
    //        }
    //      }
    //      catch (IllegalAccessException | InstantiationException iae)
    //      {
    //        Rethrow.rethrow(iae);
    //      }
    //      return nullptr; // no success
    //    }
    //
    //    private bool broken(Constructor<?> ctor, Object[] args)
    //    {
    //      final Predicate<Object[]> pred = brokenConstructors.get(ctor);
    //      return pred != nullptr && pred.test(args);
    //    }
    //
    //    // create a new random tokenizer from classpath
    //    private TokenizerSpec newTokenizer(Random random)
    //    {
    //      TokenizerSpec spec = new TokenizerSpec();
    //      while (spec.tokenizer == nullptr)
    //      {
    //        final Constructor<? extends Tokenizer> ctor = tokenizers.get(random.nextInt(tokenizers.size()));
    //        final StringBuilder descr = new StringBuilder();
    //        final Object args[] = newTokenizerArgs(random, ctor.getParameterTypes());
    //        if (broken(ctor, args))
    //        {
    //          continue;
    //        }
    //        spec.tokenizer = createComponent(ctor, args, descr, false);
    //        if (spec.tokenizer != nullptr)
    //        {
    //          spec.toString = descr.toString();
    //        }
    //      }
    //      return spec;
    //    }
    //
    //    private CharFilterSpec newCharFilterChain(Random random, Reader reader)
    //    {
    //      CharFilterSpec spec = new CharFilterSpec();
    //      spec.reader = reader;
    //      StringBuilder descr = new StringBuilder();
    //      int numFilters = random.nextInt(3);
    //      for (int i = 0; i < numFilters; i++)
    //      {
    //        while (true)
    //        {
    //          final Constructor<? extends CharFilter> ctor = charfilters.get(random.nextInt(charfilters.size()));
    //          final Object args[] = newCharFilterArgs(random, spec.reader, ctor.getParameterTypes());
    //          if (broken(ctor, args))
    //          {
    //            continue;
    //          }
    //          reader = createComponent(ctor, args, descr, false);
    //          if (reader != nullptr)
    //          {
    //            spec.reader = reader;
    //            break;
    //          }
    //        }
    //      }
    //      spec.toString = descr.toString();
    //      return spec;
    //    }
    //
    //    private TokenFilterSpec newFilterChain(Random random, Tokenizer tokenizer)
    //    {
    //      TokenFilterSpec spec = new TokenFilterSpec();
    //      spec.stream = tokenizer;
    //      StringBuilder descr = new StringBuilder();
    //      int numFilters = random.nextInt(5);
    //      for (int i = 0; i < numFilters; i++)
    //      {
    //
    //        // Insert ValidatingTF after each stage so we can
    //        // catch problems right after the TF that "caused"
    //        // them:
    //        spec.stream = new ValidatingTokenFilter(spec.stream, "stage " + i);
    //
    //        while (true)
    //        {
    //          final Constructor<? extends TokenFilter> ctor = tokenfilters.get(random.nextInt(tokenfilters.size()));
    //          if (random.nextBoolean() && avoidConditionals.contains(ctor.getDeclaringClass()) == false)
    //          {
    //            long seed = random.nextLong();
    //            spec.stream = new ConditionalTokenFilter(spec.stream, @in ->
    //            {
    //                final Object args[] = newFilterArgs(random, @in, ctor.getParameterTypes());
    //                if (broken(ctor, args))
    //                {
    //                    return @in;
    //                }
    //                TokenStream ts = createComponent(ctor, args, descr, true);
    //                if (ts == nullptr)
    //                {
    //                    return @in;
    //                }
    //                return ts;
    //            }
    //           )
    //            {
    //              Random random = new Random(seed);
    //
    //              @@Override public void reset() throws IOException
    //              {
    //                super.reset();
    //                random = new Random(seed);
    //              }
    //
    //              @@Override protected bool shouldFilter() throws IOException
    //              {
    //                return random.nextBoolean();
    //              }
    //            };
    //            break;
    //          }
    //          else
    //          {
    //            final Object args[] = newFilterArgs(random, spec.stream, ctor.getParameterTypes());
    //            if (broken(ctor, args))
    //            {
    //              continue;
    //            }
    //            final TokenFilter flt = createComponent(ctor, args, descr, false);
    //            if (flt != nullptr)
    //            {
    //              spec.stream = flt;
    //              break;
    //            }
    //          }
    //        }
    //      }
    //
    //      // Insert ValidatingTF after each stage so we can
    //      // catch problems right after the TF that "caused"
    //      // them:
    //      spec.stream = new ValidatingTokenFilter(spec.stream, "last stage");
    //
    //      spec.toString = descr.toString();
    //      return spec;
    //    }
    //  }
    //
    //  static class CheckThatYouDidntReadAnythingReaderWrapper extends CharFilter
    //  {
GET_CLASS_NAME(CheckThatYouDidntReadAnythingReaderWrapper)
    //    bool readSomething;
    //
    //    CheckThatYouDidntReadAnythingReaderWrapper(Reader @in)
    //    {
    //      super(@in);
    //    }
    //
    //    @@Override public int correct(int currentOff)
    //    {
    //      return currentOff; // we don't change any offsets
    //    }
    //
    //    @@Override public int read(char[] cbuf, int off, int len) throws IOException
    //    {
    //      readSomething = true;
    //      return input.read(cbuf, off, len);
    //    }
    //
    //    @@Override public int read() throws IOException
    //    {
    //      readSomething = true;
    //      return input.read();
    //    }
    //
    //    @@Override public int read(CharBuffer target) throws IOException
    //    {
    //      readSomething = true;
    //      return input.read(target);
    //    }
    //
    //    @@Override public int read(char[] cbuf) throws IOException
    //    {
    //      readSomething = true;
    //      return input.read(cbuf);
    //    }
    //
    //    @@Override public long skip(long n) throws IOException
    //    {
    //      readSomething = true;
    //      return input.skip(n);
    //    }
    //
    //    @@Override public void mark(int readAheadLimit) throws IOException
    //    {
    //      input.mark(readAheadLimit);
    //    }
    //
    //    @@Override public bool markSupported()
    //    {
    //      return input.markSupported();
    //    }
    //
    //    @@Override public bool ready() throws IOException
    //    {
    //      return input.ready();
    //    }
    //
    //    @@Override public void reset() throws IOException
    //    {
    //      input.reset();
    //    }
    //  }
    //
    //  static class TokenizerSpec
    //  {
GET_CLASS_NAME(TokenizerSpec)
    //    Tokenizer tokenizer;
    //    std::wstring toString;
    //  }
    //
    //  static class TokenFilterSpec
    //  {
GET_CLASS_NAME(TokenFilterSpec)
    //    TokenStream stream;
    //    std::wstring toString;
    //  }
    //
    //  static class CharFilterSpec
    //  {
GET_CLASS_NAME(CharFilterSpec)
//    Reader reader;
//    std::wstring toString;
//  }
//
//  public void testRandomChains() throws Throwable
//  {
//    int numIterations = TEST_NIGHTLY ? atLeast(20) : 3;
//    Random random = random();
//    for (int i = 0; i < numIterations; i++)
//    {
//      try (MockRandomAnalyzer a = new MockRandomAnalyzer(random.nextLong()))
//      {
//        if (VERBOSE)
//        {
//          System.out.println("Creating random analyzer:" + a);
//        }
//        try
//        {
//          checkNormalize(a);
//          checkRandomData(random, a, 500*RANDOM_MULTIPLIER, 20, false, false);
//        }
//        catch (Throwable e)
//        {
//          System.err.println("Exception from random analyzer: " + a);
//          throw e;
//        }
//      }
//    }
//  }
//
//  public void checkNormalize(Analyzer a)
//  {
//    // normalization should not modify characters that may be used for
//    wildcards
//    // or regular expressions
//    std::wstring s = "([0-9]+)?*";
//    assertEquals(s, a.normalize("dummy", s).utf8ToString());
//  }
//
//  // we might regret this decision...
//  public void testRandomChainsWithLargeStrings() throws Throwable
//  {
//    int numIterations = TEST_NIGHTLY ? atLeast(20) : 3;
//    Random random = random();
//    for (int i = 0; i < numIterations; i++)
//    {
//      try (MockRandomAnalyzer a = new MockRandomAnalyzer(random.nextLong()))
//      {
//        if (VERBOSE)
//        {
//          System.out.println("Creating random analyzer:" + a);
//        }
//        try
//        {
//          checkRandomData(random, a, 50*RANDOM_MULTIPLIER, 80, false, false);
//        }
//        catch (Throwable e)
//        {
//          System.err.println("Exception from random analyzer: " + a);
//          throw e;
//        }
//      }
//    }
//  }
//}
//
  }
