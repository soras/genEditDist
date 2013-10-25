/*
*    Copyright (C) 2010 University of Tartu
*    Authors: Reina K‰‰rik, Siim Orasmaa, Kristo Tammeoja, Jaak Vilo
*    Contact:  siim . orasmaa {at} ut . ee
*
*    This file is part of Generalized Edit Distance Tool.
*
*    Generalized Edit Distance Tool is free software: you can redistribute 
*    it and/or modify it under the terms of the GNU General Public License 
*    as published by the Free Software Foundation, either version 3 of the
*    License, or (at your option) any later version.
*
*    Generalized Edit Distance Tool is distributed in the hope that it will 
*    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
*    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with Generalized Edit Distance Tool. 
*    If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef FILETOTRIE_H
#define FILETOTRIE_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include "Trie.h"
#include "List.h"
#include <math.h>
#include "ARTrie.h"
#include <string.h>
#include <locale.h>


extern double add;
extern double rep;
extern double rem;
extern Trie *t;
extern IgnoreCaseListElement *ignoreCase;
extern int debug;
extern int caseInsensitiveMode;
extern ARTrie *addT;
extern ARTrie *remT;

/**
*   Reads file \a *filename into memory, using \c mmap() function. Returns
*   pointer to the memory-mapped file, which is read-only. Any errors on 
*   reading will cause the program to abort.
*/
char *readFile(char *filename);



/**
*   Reads a string snippet ( at position \a i to \a j ) from \a *data and 
*   converts into a double value. Returns found double.
*/
double findValue(char *data, int i, int j);

/**
*   Reads transformations from file content \a *data and builds tries \a *t,
*  \a *addT and \a *remT. If the file also specifies weights for default
*   edit operations \a add, \a rep and \a rem, these weights will also be
*   set.
*   At the end of work, the memory under \a *data is released via method 
*  \c munmap().
*/
int trieFromFile(char *data);

/**
*   Reads 'ignore case' transformations from file content \a *data and 
*  builds tries \a ignoreCase list.
*   At the end of work, the memory under \a *data is released via method 
*  \c munmap().
*/
int ignoreCaseListFromFile(char *data);



/**
*   Normalizes a case of a single character via transforming it using
*   a transformation from \a ignoreCase list. More precisely: if the
*   input char matches one of the left side chars in \a ignoreCase 
*   list, corresponding right side char from the list is returned;
*   if no match is found, returns the input character.
*   Only the first match is returned, as multiple matches are not 
*   expected.
*/
wchar_t makeToIgnoreCase(wchar_t s);

/**
*   Makes \a *string case insensitive via transforming it with
*   \a makeToIgnoreCase() method. Transformation is applied to
*   a substring (0, \a len - 1).
*/
wchar_t *makeStringToIgnoreCase(wchar_t *string, int len);



/**
*   Transforms given wchar string \a *str into multibyte char string.
*/
char *wcharToLocale(wchar_t *str);

/**
*   Transforms given multibyte char string \a *str into wchar string.
*/
wchar_t *localeToWchar(char *str);

#endif
