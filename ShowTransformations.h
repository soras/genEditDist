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

#ifndef SHOWTRANSFORMATIONS_H
#define SHOWTRANSFORMATIONS_H

#include <stdio.h>
#include <string.h>
#include "Trie.h"
#include "ARTrie.h"
#include "FileToTrie.h"
#include "Transformation.h"
#include <float.h>
#include <locale.h>

// -----------------------------------------------------------------------------
//    Util(s)
// -----------------------------------------------------------------------------

/**
*   Compares given floats and determines, whether these are approximately equal
*  (with the difference being smaller than 0.000001); 
*  This method is to be used instead of the regular comparison \a a \c == \a b , 
*  which may not work correctly ...
*   http://www.cygnus-software.com/papers/comparingfloats/Comparing%20floating%20point%20numbers.htm
*/
int equalWeights(double a, double b);

// -----------------------------------------------------------------------------
//    Backtracing transformations over generalised edit distance operations     
// -----------------------------------------------------------------------------

/**
*     Searches for transformations in "remove" trie, which can be applied
*    for reaching the \a table position \c [i][j] . 
*     Found transformations are inserted into the \a *transF : if \a first==0 
*    then transformations are added to the first/root position of \a *transF , or
*    to the positions parallel to the root; otherwise transformatons are added
*    as neighbours of the given transformation \a *transForm .
*/
double searchPathFromRemTrie(int cols, double table[][cols], Transformation *transForm, double value, wchar_t* string, int i, int j, int first, Transformations *transF, ARTrie *remTrie);

/**
*     Searches for transformations in "add" trie, which can be applied
*    for reaching the \a table position \c [i][j] . 
*     Found transformations are inserted into the \a *transF : if \a first==0 
*    then transformations are added to the first/root position of \a *transF , or
*    to the positions parallel to the root; otherwise transformatons are added
*    as neighbours of the given transformation \a *transForm .
*/
double searchPathFromAddTrie(int cols, double table[][cols], Transformation *transForm, double value, wchar_t* string, int i, int j, int first, Transformations *transF, ARTrie *addTrie);

/**
*     Searches for transformations in "replace" trie, which can be applied
*    for reaching the \a table position \c [i][j] . 
*     Found transformations are inserted into the \a *transF : if \a first==0 
*    then transformations are added to the first/root position of \a *transF , or
*    to the positions parallel to the root; otherwise transformatons are added
*    as neighbours of the given transformation \a *transForm .
*/
double searchPathFromRepTrie(int cols, double table[][cols], double value, Transformation *transForm, wchar_t *string1, wchar_t *string2, int i, int j, int first, Transformations *transF, Trie *repTrie);

/**
*     Searches a replacement from list \a *n , which matches a prefix of 
*    \a *string2 , and can be applied for reaching the \a table position 
*    \c [i][j] .
*     For each found suitable replacement, inserts the transformation into
*    the \a *transF : if \a first==0 then transformations are added to the 
*    first/root position of \a *transF , or to the positions parallel to 
*    the root; otherwise transformatons are added as neighbours of the given 
*    transformation \a *transForm .
*/
double findReplacementPath(int cols, double table[][cols], double value, Transformation *transForm, wchar_t *string2, wchar_t *left, int leftLen, EndNode *n, int i_start, int i, int j, int first, Transformations *transF);

/**
*      Given two strings \a a and \a b , and the prefilled edit distance 
*    \a table , backtraces the (generalized) edit distance operations and
*    constructs best paths ( series of transformations from string \a a to 
*    string \a b ). Transformations are inserted into \a *transF ;
*
*    \param table prefilled edit distance table
*    \param cols number of columns in the table
*    \param a search string
*    \param b text
*    \param aLen length of a
*    \param bLen length of b
*    \param transF object for storing series of transformations
*    \param remTrie trie with generalized edit distance remove operations
*    \param addTrie trie with generalized edit distance add operations
*    \param repTrie trie with generalized edit distance replace operations
*/
int findBestPaths(int cols, double table[][cols], wchar_t *a, wchar_t *b, int aLen, int bLen, Transformations *transF, ARTrie *remTrie, ARTrie *addTrie, Trie *repTrie);

#endif