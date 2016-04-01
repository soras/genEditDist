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

#ifndef TRIE_H
#define TRIE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include "List.h"

/**
*   Trie structure for storing generalized edit distance 'replace' operations.
*   'replace' operations are string transformations in form \c a => \c b (\c d), 
*   where the left side \c a is the string which can be transformed with cost
*   \c d and the right side \c b is result of the transformation (replacement).
*   
*   This structure holds pointer to first node in trie.
*/
typedef struct Trie {
    struct TrieNode *firstNode;
} Trie;


/**
*   Node of \c Trie. Basically, left sides of all transformations will be held
*   in the trie with such nodes.
*
*   While traversing the trie, left side of a 'replace' operation can be 
*   constructed by adding \a label-s. The link \a *nextNode points to a next 
*   character in the string (add \a nextNode->label to the left side), while 
*   link \a *rightNode points to alternatives of current string position (\a label
*   at current position in the left side can be replaced with \a rightNode->label).
*
*   The node with \a replacement \c != \c NULL indicates that a left side ends 
*   there and possible right sides can be constructed by following  \c EndNode 
*   links.
*   
*   The link \a *prevNode points to the previous character in the string, and it 
*   is used for backwards tracing: starting from a leaf node or from an internal 
*   node and moving backwards to the root;
*/
typedef struct TrieNode {
    /** Last character of a prefix. */
    wchar_t label; 
    /** Right side(s) for transformation(s) having \a label at the end of the left side. */
    struct EndNode *replacement;
    /** Alternate character(s) for \a label. */
    struct TrieNode *rightNode;
    /** Next character(s) following \a label. */
    struct TrieNode *nextNode; 
    /** Prev character(s) following \a label. */
    struct TrieNode *prevNode;
} TrieNode;

/**
*   A linked list node representing a right side of a transformation in \c Trie. 
*  String \a *edit represents a possible right side with cost \a value and 
*  \a *nextEN points to next possible right side.
*/
typedef struct EndNode{
    wchar_t *edit;
    double value;
    struct EndNode *nextEN;
} EndNode;

/**
*   Creates a new \c TrieNode with label \a a and links set to NULL. Returns
*   a pointer to memory which must be released afterwards.
*/
TrieNode *newTrieNode(wchar_t a);

/**
*   Creates new \c Trie with first node set to NULL. Returns a pointer to 
*   memory which must be released afterwards.
*/
Trie *createTrie();

/**
*   Creates new \c EndNode with replacement string \a *repl and cost \a weight.
*   Contents of \a *repl will be copyed, so the new node does depend upon 
*   \a *repl.
*   Returns a pointer to memory which must be released afterwards.
*/
EndNode *newEndNode(wchar_t *repl, double weight);

/**
*    Adds new replacement \a *string with cost \a value to the \a *node. If such
*    replacement already exists, replacement with lowest \a value is kept; otherwise
*    a new replacement is added to the end of \c EndNode list.
*/
int addEndNode(TrieNode *node, wchar_t *string, double value);

/**
*    Adds \a *string1 to the trie \a *node while expanding only the depth of trie (
*    creating new \c TrieNode-s). At the end, a new \c EndNode with \a *string2 and
*    \a value is also created.
*/
int addToTrieDepth(TrieNode *node, wchar_t *string1, int strLen1, wchar_t *string2, double value);

/**
*    Adds new 'replace' transformation (\a string1 \c => \a string2 with cost \a value)
*    to the trie \a *t. 
*/
int addToTrie(Trie *t, wchar_t *string1, int strLen1, wchar_t *string2, double value);

/**
*     Releases memory under \a *endNode and all following endnodes in list.
*/
void freeEndNode(EndNode *endNode);

/**
*     Releases memory under \a *node and all of its siblings and children.
*/
void freeTrieNode(TrieNode *node);

/**
*     Releases memory under \a *trie and all of its children.
*/
void freeTrie(Trie *trie);

#endif

