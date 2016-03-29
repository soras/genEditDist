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

#include "Transformation.h"

// Creates and returns a new transformation
Transformation *createTransformation(int x1, int y1, int x2, int y2, wchar_t *trL, wchar_t *trR, double w, int ad){
    Transformation *newTransform;
    newTransform = (Transformation*)malloc(sizeof(Transformation));
    if(newTransform == NULL)
        abort();
    // the table field corresponding to the beginning of the transformation:
    newTransform->startCellRow= x1;
    newTransform->startCellCol= y1;

    // the table field corresponding to the ending of the transformation:
    newTransform->endCellRow= x2;
    newTransform->endCellCol= y2;

    newTransform->nextTransformation  = NULL; // next transformation (with different beginning point)
    newTransform->prevTransformation  = NULL; // prev transformation (with different beginning point)
    newTransform->rightTransformation = NULL; // next transformation that has the same beginning point
    newTransform->leftTransformation  = NULL;  // prev transformation that has the same beginning point

    // weight/cost
    newTransform->weight = w;
    
    // type of transformation
    // 1 = generalized edit distance operation
    // 0 = regular edit distance operation
    newTransform->isAdditional = ad;
    
    if(trL != NULL){
        newTransform->trLeft = copy_wchar_t(trL, wchar_len(trL));
    }
    else newTransform->trLeft = NULL;
    if(trR != NULL){
        newTransform->trRight = copy_wchar_t(trR, wchar_len(trR));
    }
    else newTransform->trRight = NULL;

    // whether the transformation is inside the removal list:
    // 0 = no
    // 1 = yes
    newTransform->inRemovalList = 0;
    newTransform->nextRemovalTransformation = NULL;  // link to the next element in the removal list
    
    return newTransform;
}


// Creates and returns a new Transformations element
Transformations *createTransformations(){
    Transformations *link;
    link = (Transformations*)malloc(sizeof(Transformations));
    if(link == NULL)
        abort();
 	link->firstTransformation = NULL;
	return link;
}


// Releases memory under the current transformation
int removeTransformation(Transformation *transf){
    if (transf->trLeft != NULL){
        free(transf->trLeft);
    }
    if (transf->trRight != NULL){
        free(transf->trRight);
    }
    free(transf);
    return 0;
}

// Finds and returns item that is not yet in the removal list 
Transformation *findItemNotInRemoval(Transformations *transF){
    Transformation *current;
    //Transformation *tmp;
    current = transF->firstTransformation;
    // traverse the tree in a depth first manner
    while(current != NULL){
        if (current->inRemovalList == 0)
            return current;
        if(current->nextTransformation != NULL){
            current = current->nextTransformation;
        }else{
            //tmp = current;
            //while(tmp != NULL){
            //    if (tmp->inRemovalList == 0)
            //        return tmp;
            //    tmp = tmp->prevTransformation;
            //}
            if(current->rightTransformation != NULL){
                current = current->rightTransformation; // get next same level transformation
            } else {
                current = current->prevTransformation;
                while(current != NULL){
                    // if we have next same level transformation, take it
                    if(current->rightTransformation != NULL){
                        current = current->rightTransformation;
                        break;
                    }
                    else // otherwise, move one level deeper
                        current = current->prevTransformation;
                }
                if(current == NULL)
                    return NULL;
            }
        }
    }
    return NULL;
}

// Constructs the removal list, containing all transformations from *transF
int constructRemovalList(Transformations *transF){
    Transformation *unlistedItem;
    Transformation *nextUnlistedItem;
    if (transF == NULL || transF->firstTransformation == NULL)
        return 0;
    else {
        unlistedItem = findItemNotInRemoval(transF);
        while(unlistedItem != NULL){
            unlistedItem->inRemovalList = 1;
            nextUnlistedItem = findItemNotInRemoval(transF);
            unlistedItem->nextRemovalTransformation = nextUnlistedItem;
            unlistedItem = nextUnlistedItem;
        }
    }
    return 0;
}

// Releases memory under all items of the Transformations
int removeTransformations(Transformations *transF){
    Transformation *item;
    Transformation *nextItem;
    if (transF == NULL || transF->firstTransformation == NULL)
        return 0;
    else {
        if (transF->firstTransformation->inRemovalList == 0){
            // Construct removal list (if it is not constructed yet)
            constructRemovalList( transF );
        }
        if (transF->firstTransformation->inRemovalList == 0)
            abort();
        // Move through the removal list and remove the items
        item = transF->firstTransformation;
        while (item != NULL){
            nextItem = item->nextRemovalTransformation;
            removeTransformation(item);
            item = nextItem;
        }
    }
    return 0;
}


// (debug) Returns length of the removal list 
int debugRemovalListLen(Transformations *transF){
    Transformation *item;
    int items = 0;
    if (transF == NULL || transF->firstTransformation == NULL)
        return items;
    else {
        if (transF->firstTransformation->inRemovalList != 1){
            return -1;
        } else {
            item = transF->firstTransformation;
            while (item != NULL){
                //printf(" %ls:%ls %i:%i", item->trLeft,item->trRight,item->startCellRow,item->startCellCol);
                //printf("\n");
                items = items + 1;
                item = item->nextRemovalTransformation;
            }
        }
    }
    return items;
}
