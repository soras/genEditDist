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

#ifndef LIST_H
#define LIST_H


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

extern double lastBest;
extern int debug;

/**
*  An element of the \a List . Holds \a value (which is usually edit 
*  distance cost) and might hold several strings (given as a list of
*  string positions \a index in a text) having that \a value. \a nextItem
*  points to next element in \a List .
*/
typedef struct ListItem{
      double value;
      struct Index *index;
      struct ListItem *nextItem;
} ListItem;

/**
*   A linked list for storing string positions grouped by values.
*  Each element of the list may hold several strings (string positions) 
*  which all have the same value. The structure is used in TOP N search 
*  mode, for storing candidates for best matches.
*/
typedef struct List{
    struct ListItem *firstItem;
} List;

/**
*   A linked list of string positions. \a i is the beginning index and 
*  \a j is ending index of the string in a text. The structure is used 
*  in \c ListItem for storing strings (string positions) with the same 
*  edit distance (\c value).
*/
typedef struct Index{
    int i;
    int j;
    struct Index *nextIndex;
} Index;


/**
*  A linked list for storing 'ignore case' transformations. The 
*  \a left should be an upper case letter and \a right corresponding
*  lower case letter. \a next points to next element in the list.
*  Transformations in given list are used to make the search case
*  insensitive.
*/
typedef struct IgnoreCaseListElement{
    wchar_t *left;
    wchar_t *right;
    struct IgnoreCaseListElement *next;
} IgnoreCaseListElement;


/**
*   Creates new \a List. Returns pointer to aquired memory, which must
*   freed afterwards. \a firstItem of the list will be set to NULL.
*/
List *createList();


/**
*   Creates new \a Index. Returns pointer to aquired memory, which 
*   must be freed afterwards. \a nextIndex of the list will be set to NULL.
*/
Index *createIndex(int i, int j);

/**
*   Creates new \a ListItem. Returns pointer to aquired memory, 
*   which must be freed afterwards. \a nextItem of the list will be set 
*   to NULL.
*/
ListItem *createListItem(double v, int i, int j);


/**
*  Creates new \a IgnoreCaseListElement. Returns pointer to aquired memory, 
*  which must be freed afterwards. Content of strings \a *l and \a *r is 
*  copied, so they can be freed after calling the method without harming the
*  strings in the new \a IgnoreCaseListElement .
*/
IgnoreCaseListElement *createIgnoreCaseElement(wchar_t *l, wchar_t *r);

/**
*  Removes last item of the list \a *li . If there is only one element in
*  given list, it won't be removed.
*/
int removeLastItem(ListItem *li);

/**
*   Creates new \a Index and adds to given list item \a *li .
*/
int insertValues(ListItem * li, int i, int j);

/**
*   Inserts positions \a i and \a j with given \a value to \a *list in a way
*   that \a *list is sorted in ascending order of \c value . Returns 
*   \a inserted \c - \c numberOfInsertionsMade . If \a isFull is set, removes
*   last element (via \c removeLastItem() ) after insertion.
*/
int insertListItem(List *list, double value, int i, int j, int isFull, int inserted);

/**
*   Inserts new 'ignore case' transformation into the list \c *ignoreCase .
*/
int insertIgnoreCaseElement(wchar_t *l, wchar_t *r);

/**
*   Releases memory under \c *ignoreCase and all of its subelements.
*/
void freeIgnoreCaseList();

/**
*   Releases memory under \c *list and all of its subelements.
*/
void freeList(List *list);

/**
*   A debug method for printing given list.
*/  
int printList(List *list);

/**
*   Measures length of given wide-char string \a str . Expects that the string is 
*   null-terminated.
*/
int wchar_len(wchar_t *str);

/**
*   Allocates a new wide-char string and copies contents of \a str into 
*   the new string. \a strLen is the length of \a str .
*/
wchar_t *copy_wchar_t(wchar_t *str, int strLen);


extern IgnoreCaseListElement *ignoreCase;

#endif

