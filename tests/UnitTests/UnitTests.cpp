#include "CppUnitTest.h"
#include "CppUnitTestAssert.h"

#include "loomlib/loomlib_ghash.h"
#include "loomlib/loomlib_string.h"

#include "makesdna/dna_types_c.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <map>

TEST_CLASS(LoomLibUnitTest){public : TEST_METHOD(GHashUnitTest_simple){
	GHash *ghash = GLU_ghash_str_new(__func__);

GLU_ghash_insert(ghash, (void *)"peach", (void *)"2016");
GLU_ghash_insert(ghash, (void *)"genesis", (void *)"2020");
GLU_ghash_insert(ghash, (void *)"cronos", (void *)"2021");
GLU_ghash_insert(ghash, (void *)"rose", (void *)"2021");
GLU_ghash_insert(ghash, (void *)"loom", (void *)"2023");

Assert::AreEqual(GLU_ghash_len(ghash),
				 5u,
				 L"The length of the ghash is invalid, insert failed");

Assert::AreEqual((const char *)GLU_ghash_lookup(ghash, (void *)"peach"),
				 "2016");
Assert::AreEqual((const char *)GLU_ghash_lookup(ghash, (void *)"genesis"),
				 "2020");
Assert::AreEqual((const char *)GLU_ghash_lookup(ghash, (void *)"cronos"),
				 "2021");
Assert::AreEqual((const char *)GLU_ghash_lookup(ghash, (void *)"rose"), "2021");
Assert::AreEqual((const char *)GLU_ghash_lookup(ghash, (void *)"loom"), "2023");

GLU_ghash_remove(ghash, (void *)"peach", NULL, NULL);
GLU_ghash_remove(ghash, (void *)"genesis", NULL, NULL);
GLU_ghash_remove(ghash, (void *)"cronos", NULL, NULL);

Assert::AreEqual(GLU_ghash_len(ghash),
				 2u,
				 L"The length of the ghash is invalid, remove failed");

Assert::IsNotNull(GLU_ghash_lookup(ghash, (void *)"loom"));
Assert::IsNull(GLU_ghash_lookup(ghash, (void *)"peach"));
Assert::IsNotNull(GLU_ghash_lookup(ghash, (void *)"rose"));
}

TEST_METHOD(GHashUnitTest_excessive)
{
	for (int t = 0; t < 64; t++) {
		GHash *shash = GLU_ghash_str_new(__func__);

		std::map<std::string, std::string> smap;

		const char *selection[] = {"random_string1",
								   "random_string2",
								   "random_string3",
								   "random_string4",
								   "specific_string1",
								   "specific_string2",
								   "specific_string3",
								   "specific_string4"};

		for (int i = 0; i < 16384; i++) {
			const char *key = selection[rand() % ARRAY_SIZE(selection)];
			const char *val = selection[rand() % ARRAY_SIZE(selection)];
			smap[key] = val;
			GLU_ghash_reinsert(shash, (void *)key, (void *)val, NULL, NULL);
		}

		for (std::map<std::string, std::string>::const_iterator itr =
				 smap.begin();
			 itr != smap.end();
			 itr++) {
			const char *const key = itr->first.c_str();
			const char *const val = itr->second.c_str();
			Assert::AreEqual((const char *)GLU_ghash_lookup(shash, (void *)key),
							 val);
		}
	}
}

TEST_METHOD(StringUnitTest_simple)
{
	{
		const char *sample =
			"This is a string that will be split in order to have a null "
			"terminator in the middle. "
			"Trying to catch an error in GLU_strlen where a single zero byte "
			"is not accounted for.";

		Assert::AreEqual(GLU_strlen(sample), strlen(sample));
		char *dup = GLU_strdup(sample);
		Assert::AreEqual(GLU_strlen(sample), strlen(sample));
		dup[48] = '\0';
		Assert::AreEqual(GLU_strlen(sample), strlen(sample));

		MEM_freeN(dup);
	}

	{
		const char *sample =
			"This is a string that will be split in order to have a null "
			"terminator in the middle. "
			"Trying to catch an error in GLU_strlen where a single zero byte "
			"is not accounted for.";

		char *dup = GLU_strdup(sample);

		std::string rev_expected = sample;
		std::reverse(rev_expected.begin(), rev_expected.end());
		char *rev1_expected = GLU_str_reverse(dup);
		char *rev2_expected = GLU_str_reverseN(sample);

		Assert::AreEqual(
			rev_expected.c_str(), rev1_expected, "reverse in-place failed");
		Assert::AreEqual(rev_expected.c_str(), rev2_expected, "reverse failed");

		MEM_freeN(rev2_expected);
		MEM_freeN(dup);
	}

	{
		const char *sample =
			"This is a long string in which we will replace portions of it "
			"using the #GLU_str_replaceN implementing "
			"the Z algorithm fast string search. We shall replace of "
			"occurrences of the word 'string' with the words 'byte array' and "
			"then we shall compare the resulting string with the correct "
			"output string";
		const char *expected =
			"This is a long byte array in which we will replace portions of it "
			"using the #GLU_str_replaceN implementing "
			"the Z algorithm fast byte array search. We shall replace of "
			"occurrences of the word 'byte array' with the words 'byte array' "
			"and "
			"then we shall compare the resulting byte array with the correct "
			"output byte array";

		char *res = GLU_str_replaceN(sample, "string", "byte array");

		Assert::AreEqual(expected, res);

		MEM_freeN(res);
	}

	{
		char *res = GLU_sprintfN(
			"This is a sample integer placing onto string that forces "
			"#GLU_sprintfN to reallocate more than the default output "
			"buffer length, random integers to test : %d.%d.%d",
			1,
			2,
			0);

		Assert::AreEqual(
			"This is a sample integer placing onto string that forces "
			"#GLU_sprintfN to reallocate more than the default output "
			"buffer length, random integers to test : 1.2.0",
			res);

		MEM_freeN(res);
	}
}

TEST_METHOD(StringUnitTest_excessive)
{
	for (int t = 0; t < 64; t++) {
		const char *words[] = {
			"random", "words", "to",	"choose", "from", "in",		"order",
			"to",	  "make",  "a",		"string", "with", "common", "words",
			"we",	  "can",   "later", "choose", "to",	  "replace"};

		std::string generated;
		std::string expected;

		for (int i = 0; i < 16384; i++) {
			generated.append(words[rand() % ARRAY_SIZE(words)]);
			generated.append(" ");
		}

		std::string str_old = words[rand() % ARRAY_SIZE(words)];
		std::string str_new = words[rand() % ARRAY_SIZE(words)];

		/** Brute force to find the correct and expected output from the
		 * source generated string, and then see if it matches out optimized
		 * approach. */
		for (int i = 0; i <= generated.size(); i++) {
			if (i + str_old.size() <= generated.size() &&
				generated.substr(i, str_old.size()) == str_old) {
				expected += str_new;
				i += str_old.size() - 1;
			}
			else {
				expected += generated[i];
			}
		}

		const char *in = generated.c_str();
		const char *old_substr = str_old.c_str();
		const char *new_substr = str_new.c_str();
		char *out = GLU_str_replaceN(in, old_substr, new_substr);

		Assert::AreEqual(expected.c_str(), out);
	}
}

TEST_METHOD(LoomType_simple)
{
	const unsigned int matf3x3 = LOOM_MAKETYPE(LOOM_32F, 3, 3);
	Assert::AreEqual(8 * sizeof(float), LOOM_DEPTH_BITS(matf3x3));
	Assert::AreEqual(9, LOOM_CHANNELS(matf3x3));
	Assert::AreEqual(9 * sizeof(float), LOOM_SIZEOF(matf3x3));

	const unsigned int mati3x3 = LOOM_MAKETYPE(LOOM_32S, 3, 3);
	Assert::AreEqual(8 * sizeof(int), LOOM_DEPTH_BITS(mati3x3));
	Assert::AreEqual(9, LOOM_CHANNELS(mati3x3));
	Assert::AreEqual(9 * sizeof(int), LOOM_SIZEOF(mati3x3));

	const unsigned int mats3x3 = LOOM_MAKETYPE(LOOM_16U, 3, 3);
	Assert::AreEqual(8 * sizeof(unsigned short), LOOM_DEPTH_BITS(mats3x3));
	Assert::AreEqual(9, LOOM_CHANNELS(mats3x3));
	Assert::AreEqual(9 * sizeof(unsigned short), LOOM_SIZEOF(mats3x3));

	Assert::AreEqual(8, LOOM_MAGIC(LOOM_MAKETYPE_EX(LOOM_32S, 3, 3, 8)));
	Assert::AreEqual(15, LOOM_MAGIC(LOOM_MAKETYPE_EX(LOOM_32S, 3, 3, 15)));
}
}
;
