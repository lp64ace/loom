#include "CppUnitTest.h"
#include "CppUnitTestAssert.h"

#include "loomlib/loomlib_ghash.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <map>

TEST_CLASS ( LoomLibUnitTest ) {
public:
	TEST_METHOD ( GHashUnitTest_simple ) {
		GHash *ghash = GLU_ghash_str_new ( __func__ );

		GLU_ghash_insert ( ghash , ( void * ) "peach" , ( void * ) "2016" );
		GLU_ghash_insert ( ghash , ( void * ) "genesis" , ( void * ) "2020" );
		GLU_ghash_insert ( ghash , ( void * ) "cronos" , ( void * ) "2021" );
		GLU_ghash_insert ( ghash , ( void * ) "rose" , ( void * ) "2021" );
		GLU_ghash_insert ( ghash , ( void * ) "loom" , ( void * ) "2023" );

		Assert::AreEqual ( GLU_ghash_len ( ghash ) , 5u , L"The length of the ghash is invalid, insert failed" );

		Assert::AreEqual ( ( const char * ) GLU_ghash_lookup ( ghash , ( void * ) "peach" ) , "2016" );
		Assert::AreEqual ( ( const char * ) GLU_ghash_lookup ( ghash , ( void * ) "genesis" ) , "2020" );
		Assert::AreEqual ( ( const char * ) GLU_ghash_lookup ( ghash , ( void * ) "cronos" ) , "2021" );
		Assert::AreEqual ( ( const char * ) GLU_ghash_lookup ( ghash , ( void * ) "rose" ) , "2021" );
		Assert::AreEqual ( ( const char * ) GLU_ghash_lookup ( ghash , ( void * ) "loom" ) , "2023" );

		GLU_ghash_remove ( ghash , ( void * ) "peach" , NULL , NULL );
		GLU_ghash_remove ( ghash , ( void * ) "genesis" , NULL , NULL );
		GLU_ghash_remove ( ghash , ( void * ) "cronos" , NULL , NULL );

		Assert::AreEqual ( GLU_ghash_len ( ghash ) , 2u , L"The length of the ghash is invalid, remove failed" );

		Assert::IsNotNull ( GLU_ghash_lookup ( ghash , ( void * ) "loom" ) );
		Assert::IsNull ( GLU_ghash_lookup ( ghash , ( void * ) "peach" ) );
		Assert::IsNotNull ( GLU_ghash_lookup ( ghash , ( void * ) "rose" ) );
	}
	TEST_METHOD ( GHashUnitTest_excessive ) {
		GHash *shash = GLU_ghash_str_new ( __func__ );

		std::map<std::string , std::string> smap;

		const char *selection [ ] = {
			"random_string1" , "random_string2" , "random_string3" , "random_string4" ,
			"specific_string1" , "specific_string2" , "specific_string3" , "specific_string4"
		};

		for ( int i = 0; i < 65536; i++ ) {
			const char *key = selection [ rand ( ) % ARRAY_SIZE ( selection ) ];
			const char *val = selection [ rand ( ) % ARRAY_SIZE ( selection ) ];
			smap [ key ] = val;
			GLU_ghash_reinsert ( shash , ( void * ) key , ( void * ) val , NULL , NULL );
		}

		for ( std::map<std::string , std::string>::const_iterator itr = smap.begin ( ); itr != smap.end ( ); itr++ ) {
			Assert::AreEqual ( ( const char * ) GLU_ghash_lookup ( shash , ( void * ) itr->first.c_str ( ) ) , itr->second.c_str ( ) );
		}
	}
};
