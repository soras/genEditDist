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

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "List.h"

/**
*   A single transformation from \c Transformations. 
*   Corresponds to the transformation made when moving in the edit distance
*  table, starting from position \a startCellRow and \a startCellRow, and
*  ending at the position \a endCellRow and \a endCellCol.
*   
*   The link \a *nextTransformation points to the transformation involving 
*  a next position in strings (a position with different \a startCellRow or 
*  \a startCellRow ), moving towards beginnings of strings; 
*  and \a *prevTransformation points to the transformation involving a previous
*  position in strings, moving towards ends of strings.
*
*   The link \a *rightTransformation points to the right next transformation
*  that has the same starting position as this transformation ( \a startCellRow 
*  and \a startCellRow ); and the link \a *leftTransformation points to the left 
*  next transformation that has the same starting position as this transformation;
*
*   The \a weight is the cost/weight of the transformation, and \a isAdditional
*  indicates the type of transformation ( \a isAdditional==0 indicates a regular
*  edit distance operation, and \a isAdditional==1 indicates a generalized edit
*  distance operation );
*  
*   The actual change of strings (or a lack of it) is conveyed in variables 
*  \a *trLeft and \a *trRight, which correspond to the left and right side of
*  the transformation; one of these can be a string with length 0 (in case of
*  an addition or a deletion operation);
*
*   \a inRemovalList indicates whether the item is already inside the removal list
*  (a single linked list containing all transformations from \c Transformations);
*  if the transformation is inside the removal list ( \a inRemovalList==1 ), 
*  \a *nextRemovalTransformation points to next item in the list, or to NULL, if 
*  this removal list is not initiated or the if the item is the last one in the
*  list;
*/
typedef struct Transformation{
    // the table field corresponding to the beginning of the transformation:
    int startCellRow;
    int startCellCol;
    // the table field corresponding to the ending of the transformation:
    int endCellRow;
    int endCellCol;
    struct Transformation *nextTransformation;  // next transformation (with different beginning point)
    struct Transformation *prevTransformation;  // prev transformation (with different beginning point)
    struct Transformation *rightTransformation; // next transformation that has the same beginning point
    struct Transformation *leftTransformation;  // prev transformation that has the same beginning point
    // cost of transformation
    double weight;
    // type of transformation
    int isAdditional;
    // left and right side strings of this transformation:
    wchar_t *trLeft;
    wchar_t *trRight;
    // the removal list:
    int inRemovalList;  // whether the current item is already in the removal list (non-zero for 'yes')
    struct Transformation *nextRemovalTransformation; // link to the next element in the removal list
} Transformation;


/**
*   Structure for storing tracebacks from the edit distance table as a quadruple linked 
*   list of transformations. 
*
*   Contains pointer to the first transformation (which corresponds to the transformation(s) 
*   leading to the "optimal distance" corner of the edit distance table).
*
*   Essentially, the structure represents a tree where \a *firstTransformation is 
*   a root node, and pointers \a *nextTransformation and \a *prevTransformation allow 
*   depth moving between nodes, and pointers \a *rightTransformation and \a *leftTransformation 
*   allow breadth moving between nodes. Each path from the root to a leaf (a node with 
*   \a *nextTransformation \c == \c NULL ) corresponds to one alignment.
*/
typedef struct Transformations{
    struct Transformation *firstTransformation;
} Transformations;

// -----------------------------------------------------------------------------
//    Creating transformations
// -----------------------------------------------------------------------------

/**
*   Creates and returns a new transformation, which corresponds to moving in the 
*  (generalized) edit distance table from position \a x1, \a y1 to the position 
*  \a x2, \a y2, and changing the string from \a *trL to \a *trR (both may be the 
*  same if no change occurs). \a weight is the cost/weight of the transformation, 
*  and \a isAddsitional indicates its type ( \a isAdditional==0 indicates a regular
*  edit distance operation, and \a isAdditional==1 indicates a generalized edit
*  distance operation );
*   Memory under the transformation must be released after the work is complete.
*/
Transformation *createTransformation(int x1, int y1, int x2, int y2, wchar_t *trL, wchar_t *trR, double weight, int isAddsitional);

/**
*   Creates new Transformations structure and sets the first transformation to NULL. 
*   Memory under the structure must be released after the work is complete.
*/
Transformations *createTransformations();

// -----------------------------------------------------------------------------
//    Adding a new transformation to the list of transformations                
// -----------------------------------------------------------------------------

/**
*   Creates a new \a *firstTransformation to the transformations list \a *transF
*  if \a *transForm \c == \c NULL , or, if the first transformation already exists
*  (and \a *transForm points to it), creates a new rightmost branch to the existing
*  \a *firstTransformation. 
*   Created transformation corresponds to moving in the (generalized) edit distance 
*  table from the position \a i1, \a j1 (the "optimal distance" corner of the edit 
*  distance table) to the position \a i2, \a j2, and changing the string from 
*  \a *left to \a *right (both may be the same if no change occurs). 
*  \a weight is the cost/weight of the transformation, and \a ad indicates its type 
*  ( \a isAdditional==0 indicates a regular edit distance operation, and 
*    \a isAdditional==1 indicates a generalized edit distance operation );
*/
double insertFirstTransformationToList(int i1, int j1, int i2, int j2, wchar_t *left, wchar_t *right, double weight, int ad, Transformation *transForm, Transformations *transF);

/**
*   Inserts a new transformation at the position of \a *nextTransformation of
*  given \a *transForm , or, if \a transForm->nextTransformation already exists,
*  adds it a new rightmost branch.
*   Created transformation corresponds to moving in the (generalized) edit distance 
*  table from the position \a i1, \a j1 to the position \a i2, \a j2, and changing 
*  the string from \a *left to \a *right (both may be the same if no change occurs). 
*  \a weight is the cost/weight of the transformation, and \a ad indicates its type 
*  ( \a ad==0 indicates a regular edit distance operation, and 
*    \a ad==1 indicates a generalized edit distance operation );
*/
double insertTransformationToList(int i1, int j1, int i2, int j2, wchar_t *left, wchar_t *right, double weight, int ad, Transformation *transForm);

// -----------------------------------------------------------------------------
//    Releasing memory under the transformations
// -----------------------------------------------------------------------------

/**
*    Releases memory under \a *transf.
*/
int removeTransformation(Transformation *transf);
/**
*    Releases memory under all transformations from \a *transF ,
*    including the \a *transF object itself.
*/
int removeTransformations(Transformations *transF);

/**
*    Traverses all transformations in the \a *transF and finds a transformation which 
*   has \a inRemovalList==0. If \a *transF has no elements or has no element satisfying 
*   the criterion, returns NULL.
*/
Transformation *findItemNotInRemoval(Transformations *transF);
/**
*    Constructs the removal list, a single linked list containing all transformations 
*   from \c Transformations. This list is used in \c removeTransformations() to 
*   traverse all the items in the structure and to remove them one by one;
*/
int constructRemovalList(Transformations *transF);


int debugRemovalListLen(Transformations *transF);

#endif
