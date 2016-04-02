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

#ifndef FINDEDITDISTMOD_H
#define FINDEDITDISTMOD_H

#include "Trie.h"
#include "ARTrie.h"
#include "FileToTrie.h"
#include "Transformation.h"
#include "ShowTransformations.h"

#define min(x,y) (x > y ? y : x)

// Maximum number of possible match types
#define FP_MAX_POSITIONS  4

// Match types and corresponding indexes in the flag array
#define  L_EMPTY    0
#define  L_FULL     1
#define  L_PREFIX   2
#define  L_INFIX    3
#define  L_SUFFIX   4

// Default values for add, replace and remove operations
extern double rep;
extern double rem;
extern double add;
// Tries containing generalized edit distance transformations for search
extern Trie *t;
extern ARTrie *addT;
extern ARTrie *remT;
// Tries containing generalized edit distance transformations for backtracing
extern Trie *traceT;
extern ARTrie *traceAddT;
extern ARTrie *traceRemT;

extern double lastBest;

// Mask of penalties for regular edit distance
extern double *changeSearchStringWithEd_pen;

// Mask of penalties for generalized edit distance
extern double *changeSearchStringWithGenEd_pen;

// if debug == 1, then debug will be printed
extern int debug;

/**
*     Inserts \a value in \a table at position \a [row][col], but only
*    if current value in given position is greater than inserted value.
*/
int addValueToTable(int cols, double table[][cols], int row, int col, double value);

/**
*     Searches for transformations in "remove" trie, which can be applied
*    from \a table position \c [i][j] onwards. Found transformations are 
*    applied, if they improve the result: make existing values in table
*    smaller.
*/
int searchFromRemTrie(int cols, double table[][cols], wchar_t *string, int i, int j);

/**
*     Searches for transformations in "add" trie, which can be applied 
*    from \a table position \c [i][j] onwards. Found transformations are
*    applied, if they improve the result: make existing values in table
*    smaller.
*/
int searchFromAddTrie(int cols, double table[][cols], wchar_t *string, int i , int j);

/**
*     Searches for transformations in "replace" trie, which can be applied
*    from \a table position \c [i][j] onwards. Found transformations are
*    applied, if they improve the result: make existing values in table 
*    smaller.
*/
int searchFromRepTrie(int cols, double table[][cols], wchar_t *string1, wchar_t *string2, int i, int j);

/**
*    Searches a replacement from list \a *n, which matches a prefix of 
*    \a *string. For each found match, the replacement is applied, if 
*    it improves the result: makes existing values in table smaller.
*/
int findReplacement(int cols, EndNode *n, double table[][cols], wchar_t *string, int i, int j, double value);

/**
*   Returns a penalty value, which must be added to the cost of changing 
*   i-th position in the search string with regular edit distance.
*
*  \param i position in search string
*/
double getPenaltOfChangingPos(int i);

/**
*   Returns a penalty value, which must be added to the cost of changing 
*   i-th position in the search string with generalized edit distance.
*
*  \param i position in search string
*/
double getPenaltOfChangingPosWithGenEd(int i);

/**
*   Calculates regular edit distance between strings \a a and \a b.
*   Cost of every operation is 1.0.
*
*  \param a search string
*  \param b text
*  \param aLen length of a
*  \param bLen length of b
*/
int editDistance(wchar_t* a, wchar_t* b,int aLen, int bLen);

/**
*   Calculates generalized edit distance between full strings \a a 
*   and \a b without applying any penalties.
*
*   If \a *transF \c != \c NULL , transformations are backtraced and
*   stored into \a *transF .
*
*  \param a search string
*  \param b text
*  \param aLen length of a
*  \param bLen length of b
*  \param transF object for storing tracebacks of the transformations
*/
double genEditDistance(wchar_t *a, wchar_t *b, int aLen, int bLen, Transformations *transF);

/**
*   (A shortcut method)
*   Calculates generalized edit distance between string \a a and 
*   a prefix of string \a b. The method also considers penalties 
*   of changing the search string.
*
*  \param a search string
*  \param b text
*  \param aLen length of a
*  \param bLen length of b
*/
double genEditDistance_prefix(wchar_t *a, wchar_t *b, int aLen, int bLen);

/**
*   (A shortcut method)
*   Calculates generalized edit distance between string \a a and 
*   a suffix of string \a b. The method also considers penalties 
*   of changing the search string.
*
*  \param a search string
*  \param b text
*  \param aLen length of a
*  \param bLen length of b
*/
double genEditDistance_suffix(wchar_t *a, wchar_t *b, int aLen, int bLen);

/**
*   (A shortcut method)
*   Calculates generalized edit distance between string \a a and 
*   an infix of string \a b. The method also considers penalties 
*   of changing the search string.
*
*  \param a search string
*  \param b text
*  \param aLen length of a
*  \param bLen length of b
*/
double genEditDistance_middle(wchar_t *a, wchar_t *b, int aLen, int bLen);

/**
*   (A shortcut method)
*   Calculates generalized edit distance between full strings \a a and
*   \a b. The method is practically same as \a genEditDistance(), only
*   difference is that penalties of changing the search string are also
*   considered.
*
*  \param a search string
*  \param b text
*  \param aLen length of a
*  \param bLen length of b
*/
double genEditDistance_full(wchar_t *a, wchar_t *b, int aLen, int bLen);

/**
*   Calculates generalized edit distance between strings \a a and \a b, 
*   allowing only a partial match with \a b (match can skip a prefix, 
*   suffix or a circumfix of \a b). For example, if \a isPrefix==1 and 
*   \a isSuffix==0, then match is calculated only with a prefix of
*   \a b, while the suffix can be arbitary and does not effect the 
*   result. 
*   Penalties of changing the search string are also cosidered while 
*   calculation.
*
*   For a shortcut methods, see \a genEditDistance_full(),
*   \a genEditDistance_middle(), \a genEditDistance_suffix(),
*   \a genEditDistance_prefix().
*
*  \param a search string
*  \param b text
*  \param aLen length of a
*  \param bLen length of b
*  \param isPrefix 1 if prefix cannot be skipped, 0 if it can be skipped
*  \param isSuffix 1 if suffix cannot be skipped, 0 if it can be skipped
*/
double genEditDistance_mod(wchar_t *a, wchar_t *b, int aLen, int bLen, short isPrefix, short isSuffix);

/**
*
*   Calculates generalized edit distance between strings \a a and \a b, 
*   cosidering also penalties. Considers two kinds of penalties: penalties
*   of changing the text (\a start_pen and \a end_pen) and penalties of 
*   changing the search string (see \a getPenaltOfChangingPos() and 
*   \a getPenaltOfChangingPosWithGenEd() ).
*
*   Note that the term "penalties" might be misleading for \a start_pen 
*   and \a end_pen, as they are in this context rather "benefits". If they
*   are both NULL, full (restricted) match is calculated, while if one of
*   them is filled with 0.0 values, the restrictions are loosened, allowing
*   to skip a prefix or a suffix of text while matching.
*   
*
*  \param a search string
*  \param b text
*  \param aLen length of a
*  \param bLen length of b
*  \param start_pen array of length bLen, start_pen[i] shows penalty
*                   for starting from text b at position i
*  \param end_pen array of length bLen, end_pen[i] shows penalty for 
*                 ending in text b at position i
*/
double genEditDistance_pens(wchar_t *a, wchar_t *b, int aLen, int bLen, double* start_pen, double* end_pen);


/**
*   A debug method for printing generalized edit distance table with some additional
*   information.
*/
void printTableWithChangingPenalties( wchar_t *a, wchar_t *b, int aLen, int bLen, 
                                      int rows, int cols, double table[rows][cols]);

#endif // FINDEDITDISTMOD_H
