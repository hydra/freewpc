#include <freewpc.h>
#include "seatest.h"

extern void io_init(void);

#include "unittests.h"
//
// create a test...
//
void test_hello_world( void )
{
        char *s = "hello world!";
        assert_string_equal("hello world!", s);
        assert_string_contains("hello", s);
        assert_string_doesnt_contain("goodbye", s);
        assert_string_ends_with("!", s);
        assert_string_starts_with("hell", s);
}

//
// put the test into a fixture...
//
void test_fixture_hello( void )
{
        test_fixture_start();
        run_test(test_hello_world);
        test_fixture_end();
}

//
// put the fixture into a suite...
//
void all_tests( void )
{
        test_fixture_hello();
        test_fixture_switches();
        test_fixture_combos();
}


//
// freewpc unit test helpers
//

void db_puts_unittest (const char *s) {
	printf(s);
}

//
// run the suite!
//
int main( int argc, char** argv )
{
	db_puts = db_puts_unittest;
	io_init();

        return run_tests(all_tests);
}
