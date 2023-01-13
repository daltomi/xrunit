/*
	Copyright © 2020 Daniel T. Borelli <danieltborelli@gmail.com>

	This file is part of xrunit.

	xrunit is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xrunit is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xrunit.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DEBUG_H_INCLUDE
#define DEBUG_H_INCLUDE

#define ASSERT(Test)                                                         \
do {                                                                         \
        if (! (Test)) {                                                      \
         fprintf (stderr, "Assertion \"%s\" failed in file %s at line %d\n", \
          #Test, __FILE__, __LINE__);                                        \
         _exit(EXIT_FAILURE);                                                \
        }                                                                    \
} while(0)

#define ASSERT_STRING(Test)			\
do {								\
		ASSERT(Test);				\
		ASSERT(strlen((Test)) > 0);	\
} while(0)

#ifdef DEBUG
#define ASSERT_DBG(Test) ASSERT(Test)

#define ASSERT_DBG_STRING(Test) ASSERT_STRING(Test)

#else
#define ASSERT_DBG(Test)
#define ASSERT_DBG_STRING(Test)
#endif

#define SHOW_MESSAGE(Mesg, ...) (fprintf(stderr,"" Mesg "",##__VA_ARGS__))
#define SHOW_MESSAGE_HEADER     (fprintf(stderr,"\n"))

#define WARNING(...)                              \
do {                                              \
  SHOW_MESSAGE_HEADER;                            \
  SHOW_MESSAGE(" Warning:\n" __VA_ARGS__);        \
  SHOW_MESSAGE_HEADER;                            \
} while(0)

#define MESSAGE(...)                              \
do {                                              \
  SHOW_MESSAGE_HEADER;                            \
  SHOW_MESSAGE("Message:\n" __VA_ARGS__);         \
  SHOW_MESSAGE_HEADER;                            \
} while(0)


#define STOP(...)                                 \
do {                                              \
  SHOW_MESSAGE_HEADER;                            \
  SHOW_MESSAGE("Stop:\n" __VA_ARGS__);            \
  SHOW_MESSAGE_HEADER;                            \
  ASSERT(0);                                      \
} while (0)


#ifdef DEBUG
#define WARNING_DBG(...)                          \
do {                                              \
  SHOW_MESSAGE_HEADER;                            \
  SHOW_MESSAGE("Debug - Warning:\n" __VA_ARGS__); \
  SHOW_MESSAGE_HEADER;                            \
} while(0)

#define MESSAGE_DBG(...)                          \
do {                                              \
  SHOW_MESSAGE_HEADER;                            \
  SHOW_MESSAGE("Debug - Message:\n" __VA_ARGS__); \
  SHOW_MESSAGE_HEADER;                            \
} while(0)

#define STOP_DBG(...)                             \
 do {                                             \
  SHOW_MESSAGE_HEADER;                            \
  SHOW_MESSAGE("Debug - Stop:\n" __VA_ARGS__);    \
  SHOW_MESSAGE_HEADER;                            \
  ASSERT_DBG(0);                                  \
} while (0)

#else

#define WARNING_DBG(...)

#define MESSAGE_DBG(...)

#define STOP_DBG(...)

#endif
#endif // DEBUG_H_INCLUDE
