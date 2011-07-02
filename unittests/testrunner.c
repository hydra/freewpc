/*
 * Copyright 2011 by Dominic Clifton <me@dominicclifton.name>
 *
 * This file is part of FreeWPC.
 *
 * FreeWPC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * FreeWPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FreeWPC; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * Run all unit tests with:
 *
 * make UNITTEST=y unittest
 */

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
