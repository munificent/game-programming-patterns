//
//  expect.h
//  gpp
//
//  Created by Bob Nystrom on 6/25/13.
//  Copyright (c) 2013 Bob Nystrom. All rights reserved.
//

#ifndef gpp_expect_h
#define gpp_expect_h

#define EXPECT(condition) \
expect_(__FILE__, __LINE__, #condition, condition)

static void expect_(const char * file, int line,
                    const char * expression,
                    bool condition)
{
  using std::cout;
  using std::endl;

  if (condition)
  {
    cout << "PASS: " << expression << endl;
  }
  else
  {
    cout << "FAIL: " << expression << endl;
    cout << "      " << file << ":" << line << endl;
  }
}

#endif
