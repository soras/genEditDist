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

#ifndef ARTRIE_H
#define ARTRIE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>
#include <locale.h>

/**
*   Node of \c ARTrie. A string can be constructed by following links
*  \a *nextNode and \a *rightNode. The link \a *nextNode points to a next 
*  character in the string (add \a nextNode->label to string), while link
*  \a *rightNode points to alternatives of current string position (\a label
*  at current position can be replaced with \a rightNode->label). The node 
*  with \a value \c != \c DBL_MAX indicates that a transformation with cost
*  \a value ends there.
*/
typedef struct ARTNode{
	wchar_t label;
	double value;
	struct ARTNode *rightNode;
	struct ARTNode *nextNode;
} ARTNode;


/**
*   Trie structure for storing generalized edit distance 'add' or 'remove'
*   operations. Contains pointer to the first node in trie.
*/
typedef struct ARTrie{
	struct ARTNode *firstNode;
} ARTrie;

/**
*   Creates new ARTrie and sets the first node to NULL. Memory under the trie
*   must be released after the work is complete.
*/
ARTrie *createARTrie();

/**
*   Creates new ARTNode, setting links to NULL and \a value to \c DBL_MAX. 
*   Memory under the node must be released after the work is complete.
*/
ARTNode *newARTNode(wchar_t a);

/**
*   Adds an add or remove transformation \a *string (length \a strLen) with given
*  weight \a value into the trie \a *art.
*/
int addToARTrie(ARTrie *art, wchar_t *string, int strLen, double value);

/**
*    A debug method for printing the trie, in breadth-first order. NB! Uses a
*    constant size array to store the nodes while traversing.
*/
int showARTrie(ARTrie *t);

/**
*    Releases memory under \a *node and all of its relatives (siblings and 
*    children).
*/
void freeARTNode(ARTNode *node);

/**
*    Releases memory under \a *art and all of its children.
*/
void freeARTrie(ARTrie *art);

#endif

