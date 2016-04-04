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

#include "ShowTransformations.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//    Util(s)
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

double min_weight = 0.000001;

// Compares two doubles and determines, whether these are equal
int equalWeights(double a, double b){
   return (fabs(a - b) < (min_weight/10.0));
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//    Backtracing transformations over generalised edit distance operations     
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// Search for best paths leading to the given table cell (i, j) via (generalized edit distance) remove operations
double searchPathFromRemTrie(int cols, double table[][cols], Transformation *transForm, double value, wchar_t* string, int i, int j, int first, Transformations *transF, ARTrie *remTrie){
    ARTNode *tmp;
    int i1, j1;
    int c = 1;
    int t;
    // Start from the first node of remove operations, seek for remove operations
    // that could have led to the position ( i, j ) in the table;
    tmp = remTrie->firstNode;
    i1 = i; j1 = j;
    while(tmp != NULL && i > 0){
        if(tmp->label == string[i-1]){
            c++;
            //  If the end of trie has been reached ...
            if(tmp->value != DBL_MAX){
                // If we have found a legitimate transformation (tmp->value != DBL_MAX),
                // check whether it could have been used for reaching the given cell:
                if( equalWeights( table[i-1][j] + tmp->value, value) ){
                    // We have found a removing transformation that can be used for reaching the given cell
                    // in the table: 
                    //    now, construct the transformation string and add it to the list of 
                    //    transformations
                    wchar_t *s;
                    s = (wchar_t *)malloc(sizeof(wchar_t)*c);
                    s[c-1] = L'\0';
                    t = 0;
                    ARTNode *prev;
                    prev = tmp;
                    while(t < c-1){
                        s[t] = prev->label;
                        prev = prev->prevNode;
                        t++;
                    }
                    if(first == 0)
                        // add as a first transformation of *transF
                        insertFirstTransformationToList(i1, j1, i-1, j, s, NULL, tmp->value, 1, transForm, transF);
                    else
                        // add as a next transformation of given *transForm
                        insertTransformationToList(i1, j1, i-1, j, s, NULL, tmp->value, 1, transForm);
                    free(s);
                }
            }
            tmp = tmp->nextNode;
            i--;
        }
        else tmp = tmp->rightNode;
    }
    return 0;
}


// Search for best paths leading to the given table cell (i, j) via (generalized edit distance) add operations
double searchPathFromAddTrie(int cols, double table[][cols], Transformation *transForm, double value, wchar_t* string, int i, int j, int first, Transformations *transF, ARTrie *addTrie){
    ARTNode *tmp;
    int c = 1;
    int t;
    int i1, j1;
    // Start from the first node of add operations, seek for add operations
    // that could have led to the position ( i, j ) in the table;
    tmp = addTrie->firstNode;
    i1 = i; j1 = j;
    while(tmp != NULL && j > 0){
        if(tmp->label == string[j-1]){
            c++;
            if(tmp->value != DBL_MAX){
                // If we have found a legitimate transformation (tmp->value != DBL_MAX),
                // check whether it could have been used for reaching the given cell:
                if( equalWeights( table[i][j-1]+ tmp->value, value) ) {
                    // We have found an adding transformation that can be used for reaching the given cell
                    // in the table: 
                    //    now, construct the transformation string and add it to the list of 
                    //    transformations
                    wchar_t *s;
                    s = (wchar_t *)malloc(sizeof(wchar_t)*c);
                    s[c-1] = L'\0';
                    t = 0;
                    ARTNode *prev;
                    prev = tmp;
                    while(t < c-1){
                        s[t] = prev->label;
                        prev = prev->prevNode;
                        t++;
                    }
                    if(first == 0)
                        // add as a first transformation of *transF
                        insertFirstTransformationToList(i1, j1, i, j-1, NULL, s, tmp->value, 1, transForm, transF);
                    else
                        // add as a next transformation of given *transForm
                        insertTransformationToList(i1, j1, i, j-1, NULL, s, tmp->value, 1, transForm);
                    free(s);
                }
            }
            tmp = tmp->nextNode;
            j--;
        }
        else tmp = tmp->rightNode; // labels did not match: check for the next node at the same level
    }
    return 0;
}

// Search for best paths leading to the given table cell (i, j) via (generalized edit distance) replace operations
double searchPathFromRepTrie(int cols, double table[][cols], double value, Transformation *transForm, wchar_t *string1, wchar_t *string2, int i, int j, int first, Transformations *transF, Trie *repTrie){
    TrieNode *tmp;
    int t1;
    int c = 1;
    int i_start = i;
    // Start from the first node of replace operations trie, seek for replace operations
    // that could have led to the position ( i, j ) in the table;
    tmp = repTrie->firstNode;
    while(tmp != NULL){
        if(tmp->label == string1[i-1]){
            c++;
            if(tmp->replacement != NULL){
                // If we have found a legitimate transformation (tmp->replacement != NULL),
                // reconstruct the left side of the transformation and then seek for matching 
                // right sides 
                wchar_t *left;
                left = (wchar_t *)malloc(sizeof(wchar_t)*c);
                left[c-1] = L'\0';
                t1 = 0;
                TrieNode *prev;
                prev = tmp;
                while(t1 < c-1){
                   left[t1] = prev->label;
                   prev = prev->prevNode;
                   t1++;
                }
                EndNode *repl;
                repl = tmp->replacement;
                // seek for matching right sides of the transformation, and insert found
                // right sides into the list of transformations
                findReplacementPath(cols, table, value, transForm, string2, left, c-1, repl, i_start, i-1, j, first, transF);
                free(left);
            }
            tmp = tmp->nextNode;
            i--;
        }
        else tmp = tmp->rightNode;  // labels did not match: check for the next node at the same level
    }
    return 0;
}


// Finds concrete replacements that can lead to the given table cell (i, j) via (generalized edit distance) replace
double findReplacementPath(int cols, double table[][cols], double value, Transformation *transForm, wchar_t *string2, wchar_t *left, int leftLen, EndNode *n, int i_start, int i, int j, int first, Transformations *transF){
    wchar_t *repl;
    int j1;
    int t1;
    while(n != NULL){
        // Take the right side string from the EndNode
        repl = n->edit;
        j1 = j;
        int c = 0;
        // Find length of the right side string
        while(repl[c] != L'\0')
            c++;
        int sLen = c;
        while(1){
            // Attempt to match string2 with the right side string
            if(n != NULL && c > 0 && j1 > 0 && string2[j1 - 1] == repl[c - 1]){
                if(c == 1){ 
                    // If the right side string matches string2, then check whether it could 
                    // have been used for reaching the given cell:
                    if( equalWeights( table[i][j1-1] + n->value, value ) ){
                        // We have found a replace transformation that can be used for reaching the given 
                        // cell in the table: 
                        //    now, re-construct the right side of transformation, and add the transformation
                        //    to the list of transformations
                        wchar_t *s;
                        s = (wchar_t *)malloc(sizeof(wchar_t)*(sLen+1));
                        s[sLen] = L'\0';
                        t1 = sLen-1;
                        while(t1 >= 0){
                            s[t1] = repl[t1];
                            t1--;
                        }
                        if(first == 0)
                            // add as a first transformation of *transF
                            insertFirstTransformationToList(i_start, j, i, j1-1, left, s, n->value, 1, transForm, transF);
                        else
                            // add as a next transformation of given *transForm
                            insertTransformationToList(i_start, j, i, j1-1, left, s, n->value, 1, transForm);
                        free(s);
                    }
                    // goto checking the next EndNode (the next right side string)
                    goto nextRepl;
                }
                c--;
                j1--;
            }
            else break;
        }
        nextRepl: n = n->nextEN;
    }
    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//    Backtracing the (generalized) edit distance table for transformations     
//    made on the best paths                                                    
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// Backtraces (generalized) edit distance operations and finds best edit paths between strings a and b
int findBestPaths(int cols, double table[][cols], 
                  wchar_t *a, wchar_t *b, 
                  int aLen, int bLen, 
                  Transformations *transF, 
                  ARTrie *remTrie, ARTrie *addTrie, Trie *repTrie){
    
    if (transF == NULL || transF->firstTransformation != NULL){
        // The given list of transformations must be initialised and empty,
        // if not: fatal error
        abort();
    }
    
    double value;
    int i;
    int j;
    Transformation *currentTransf;
    wchar_t *l;
    wchar_t *r;

    // -----------------------------------------------------------------------
    //  1) Find transformations leading to the corner cell of the table       
    // -----------------------------------------------------------------------
    i = aLen;
    j = bLen;
    value = table[i][j];
    //
    //  A. Find generalized edit distance operations that could have been used 
    //     for reaching to the corner cell (aLen, bLen)
    if(i > 0 && remTrie != NULL)
        searchPathFromRemTrie(cols, table, transF->firstTransformation, value, a, i, j, 0, transF, remTrie);
    if(j > 0 && addTrie != NULL)
        searchPathFromAddTrie(cols, table, transF->firstTransformation, value, b, i, j, 0, transF, addTrie);
    if(i > 0 && j > 0 && repTrie != NULL)
        searchPathFromRepTrie(cols, table, value, transF->firstTransformation, a, b, i, j, 0, transF, repTrie);
    //
    //  B. Find regular edit distance operations that could have been used 
    //     for reaching to the corner cell (aLen, bLen)
    //  B.1. regular deletion
    if(i > 0 && equalWeights (table[i - 1 ][j] + rem, value)){
        l = (wchar_t *)malloc(sizeof(wchar_t)*2);
        l[1] = L'\0';
        l[0] = a[i - 1];
        insertFirstTransformationToList(i, j, i-1, j, l, NULL, rem, 0, transF->firstTransformation, transF);
        free(l);
    }
    //  B.2. regular addition
    if(j > 0 && equalWeights(table[i][j -1] + add, value)){
        r = (wchar_t *)malloc(sizeof(wchar_t)*2);
        r[1] = L'\0';
        r[0] = b[j - 1];
        insertFirstTransformationToList(i, j, i, j-1, NULL, r, add, 0, transF->firstTransformation, transF);
        free(r);
    }
    //  B.3. regular replace
    if(i > 0 && j > 0 && 
        ( (equalWeights(table[i -1][j -1] + rep, value) || 
             (a[i-1] == b[j-1] && equalWeights(table[i-1][j-1], value)) ) )
       )
    {
        l = (wchar_t *)malloc(sizeof(wchar_t)*2);
        l[1] = L'\0';
        l[0] = a[i - 1];
        r = (wchar_t *)malloc(sizeof(wchar_t)*2);
        r[1] = L'\0';
        r[0] = b[j - 1];
        if(a[i-1] == b[j-1] && equalWeights(table[i-1][j-1], value))
            insertFirstTransformationToList(i, j, i-1, j-1, l, r, 0, 0, transF->firstTransformation, transF);
        else
            insertFirstTransformationToList(i, j, i-1, j-1, l, r, rep, 0, transF->firstTransformation, transF);
        free(l);
        free(r);
    }

    // -----------------------------------------------------------------------
    //  2) Find all the following transformations                             
    // -----------------------------------------------------------------------
    currentTransf = transF->firstTransformation; // get the very first transformation
    if(currentTransf == NULL){
        return 0;
    }
    i = currentTransf->endCellRow;
    j = currentTransf->endCellCol;
    while(currentTransf != NULL){
        value = table[i][j];
        // Backtrace: find all transformations that could have been used for reaching the given cell
        //  A. Find generalized edit distance operations that could have been used 
        //     for reaching to the cell (i, j)
        if(i > 0 && remTrie != NULL)
            searchPathFromRemTrie(cols, table, currentTransf, value, a, i, j, 1, transF, remTrie);
        if(j > 0 && addTrie != NULL)
            searchPathFromAddTrie(cols, table, currentTransf, value, b, i, j, 1, transF, addTrie);
        if(i > 0 && j > 0 && repTrie != NULL)
            searchPathFromRepTrie(cols, table, value, currentTransf, a, b, i, j, 1, transF, repTrie);
        //
        //  B. Find regular edit distance operations that could have been used 
        //     for reaching to the cell (i, j)
        //  B.1. regular deletion
        if(i > 0 && equalWeights(table[i - 1 ][j] + rem, value)){
            l = (wchar_t *)malloc(sizeof(wchar_t)*2);
            l[1] = L'\0';
            l[0] = a[i - 1];
            insertTransformationToList(i, j, i-1, j, l, NULL, rem, 0, currentTransf);
            free(l);
        }
        //  B.2. regular addition
        if(j > 0 && equalWeights(table[i][j -1] + add, value)){
            r = (wchar_t *)malloc(sizeof(wchar_t)*2);
            r[1] = L'\0';
            r[0] = b[j - 1];
            insertTransformationToList(i, j, i, j-1, NULL, r, add, 0, currentTransf);
            free(r);
        }
        //  B.3. regular replace
        if(i > 0 && j > 0 && 
            ( (equalWeights(table[i -1][j -1] + rep, value) || 
                (a[i-1] == b[j-1] && equalWeights(table[i-1][j-1], value))) )
           )
        {
            l = (wchar_t *)malloc(sizeof(wchar_t)*2);
            l[1] = L'\0';
            l[0] = a[i - 1];

            r = (wchar_t *)malloc(sizeof(wchar_t)*2);
            r[1] = L'\0';
            r[0] = b[j - 1];
            if(a[i-1] == b[j-1] && equalWeights(table[i-1][j-1], value))
                insertTransformationToList(i, j, i-1, j-1, l, r, 0, 0, currentTransf);
            else
                insertTransformationToList(i, j, i-1, j-1, l, r, rep, 0, currentTransf);
            free(l);
            free(r);
        }

        //  try to move (backwards) to the next position in the strings;
        //     if backwards moving is not possible ( none of the previously checked edit
        //     operations produced nextTransformation for the currentTransf ), we have 
        //     likely reached to the table field (0, 0), and we must start browsing 
        //     alternatives ( rightmost transformations ) of the current transformation,
        //     or previously expanded transformations;
        if(currentTransf->nextTransformation == NULL ){
            // if backwards moving is not possible, try to move to the alternative
            // transformation(s) at the same level
            if(currentTransf->rightTransformation != NULL){
                currentTransf = currentTransf->rightTransformation;
                i = currentTransf->endCellRow;
                j = currentTransf->endCellCol;
            } else {
                // if there is no same level transformations, try to move forward
                // (towards the root), seeking the transformations that have right 
                // side branches available
                currentTransf = currentTransf->prevTransformation;
                while(currentTransf != NULL && currentTransf->rightTransformation == NULL){
                    currentTransf = currentTransf->prevTransformation;
                }
                if(currentTransf == NULL){
                    // if we have reached the root of the tree (without getting any opportunity
                    // for moving to the right branch), end the process
                    return 0; 
                } else {
                    // otherwise, if we have found a right (alternative) branch, move to it
                    currentTransf = currentTransf->rightTransformation;
                    i = currentTransf->endCellRow;
                    j = currentTransf->endCellCol;
                }
            }
        } else { 
            // move (backwards) to the next position in the strings
            currentTransf = currentTransf->nextTransformation;
            i = currentTransf->endCellRow;
            j = currentTransf->endCellCol;
        }
    }
    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//    Printing alignments and transformations                                   
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// Outputs all transformations from string a to string b
int printTransformations(wchar_t *a, wchar_t *b, Transformations *transF, 
                         int caseInsensitiveMode, int printAlignments, int printTransWeights, int printPretty){
    Transformation *current;
    Transformation *tmp;
    int i;
    // Traverse the tree in a depth first manner, starting from the root (first) transformation
    current = transF->firstTransformation;
    while(current != NULL){
        if(current->nextTransformation != NULL)
            // If possible, move towards the beginning of the strings 
            current = current->nextTransformation;
        else {
            // If we could not move further, we have reached to the 
            // beginning of the strings, and thus we can now reconstruct 
            // the serie of transformations
            // ------------
            //  A) While moving back towards the root, print   
            //     left sides of transformations (a.k.a upper 
            //     side of the alignment)
            // ------------
            tmp = current;
            if (printAlignments > 0){
                while(tmp != NULL){
                    if (caseInsensitiveMode){ 
                        //  If we are in caseInsensitiveMode, we have to copy the strings from 
                        // the original input string, because the trie contains case-converted strings
                        if(tmp->trLeft != NULL){
                            wchar_t *tmpStr;
                            int tmpStrLen;
                            tmpStrLen = (tmp->startCellRow - tmp->endCellRow);
                            tmpStr = (wchar_t*)malloc(sizeof(wchar_t)*(tmpStrLen+1));
                            tmpStr[tmpStrLen] = L'\0';
                            for(i = 0; i < tmpStrLen; i++){
                                tmpStr[i] = a[tmp->endCellRow + i];
                            }
                            if (printPretty > 0){
                               prettyPrint(tmpStr, tmp->trRight);
                            } else {
                               printf("%ls:", tmpStr);
                            }
                            free(tmpStr);
                        }else{
                            if (printPretty > 0){
                               prettyPrint(tmp->trLeft, tmp->trRight);
                            } else {
                               printf(":");
                            }
                        }
                    }else{
                        // print left sides of transformations
                        if(tmp->trLeft != NULL){
                            if (printPretty > 0){
                               prettyPrint(tmp->trLeft, tmp->trRight);
                            } else {
                               printf("%ls:", tmp->trLeft);
                            }
                        } else {
                            if (printPretty > 0){
                               prettyPrint(tmp->trLeft, tmp->trRight);
                            } else {
                               printf(":");
                            }
                        }
                    }
                    tmp = tmp->prevTransformation;
                }
                printf("\n");
            }
            
            // ------------
            //  B) While moving back towards the root, print   
            //     weights of transformations 
            // ------------
            tmp = current;
            if (printTransWeights > 0){
                while(tmp != NULL){
                    // print transformation weight
                    printf("%f:", tmp->weight);
                    tmp = tmp->prevTransformation;
                }
                printf("\n");
            }
            
            // ------------
            //  C) While moving back towards the root, print   
            //     right sides of transformations (a.k.a lower 
            //     side of the alignment)
            // ------------
            tmp = current;
            if (printAlignments > 0){
                while(tmp != NULL){
                    if(caseInsensitiveMode){ 
                        //  If we are in caseInsensitiveMode, we have to copy the strings from 
                        // the original input string, because the trie contains case-converted strings
                        if(tmp->trRight != NULL){
                            wchar_t *tmpStr;
                            int tmpStrLen;
                            tmpStrLen = (tmp->startCellCol - tmp->endCellCol);
                            tmpStr = (wchar_t*)malloc(sizeof(wchar_t)*(tmpStrLen+1));
                            tmpStr[tmpStrLen] = L'\0';
                            for(i = 0; i < tmpStrLen; i++){
                                tmpStr[i] = b[tmp->endCellCol + i];
                            }
                            if (printPretty > 0){
                               prettyPrint(tmpStr, tmp->trLeft);
                            } else {
                               printf("%ls:", tmpStr);
                            }
                            free(tmpStr);
                        } else {
                            if (printPretty > 0){
                               prettyPrint(tmp->trRight, tmp->trLeft);
                            } else {
                               printf(":");
                            }
                        }
                   }
                   else{
                        // print right sides of transformations
                        if(tmp->trRight != NULL){
                            if (printPretty > 0){
                               prettyPrint(tmp->trRight, tmp->trLeft);
                            } else {
                               printf("%ls:", tmp->trRight);
                            }
                        }else{
                            if (printPretty > 0){
                               prettyPrint(tmp->trRight, tmp->trLeft);
                            } else {
                               printf(":");
                            }
                        }
                   }
                    tmp = tmp->prevTransformation;
                }
                printf("\n");
            }
            printf(";\n");
            
            //
            // After we have outputted the serie of transformations,
            // we have to check for possible alternative series
            //
            if(current->rightTransformation != NULL){
                // Move to the next alternative at the same level
                current = current->rightTransformation;
            } else {
                // If there are no same level alternatives, decrease the depth and
                // check for alternative right-branching on the road
                current = current->prevTransformation;
                while(current != NULL){
                    // Check for alternative right-branching
                    if(current->rightTransformation != NULL){
                        current = current->rightTransformation;
                        break;
                    }
                    else
                        // Move upwards ( towards the root )
                        current = current->prevTransformation;
                }
                // If we have reached to the root, there is nothing left to print
                if(current == NULL)
                    return 0;
            }
        }
    }
    return 0;
}


// Prints *thisStr in a pretty-print manner, padding it with spaces if it is shorter than *otherStr
int prettyPrint(wchar_t *thisStr, wchar_t *otherStr){
    // Find maximum length of two sides of the transformation
    int thisLen  = 0;
    int otherLen = 0;
    int maxLen   = 0;
    if (thisStr != NULL){
        while (thisStr[thisLen] != L'\0'){
            thisLen = thisLen + 1;
        }
        maxLen = thisLen;
    }
    if (otherStr != NULL){
        while (otherStr[otherLen] != L'\0'){
            otherLen = otherLen + 1;
        }
        if (otherLen > maxLen){
            maxLen = otherLen;
        }
    }
    if ( thisLen == maxLen )
        printf( "%ls:", thisStr );
    else {
        // This side is shorter than the other side: construct this side as a 
        // new string having an equal length, and pad the gap in the left side 
        // with spaces
        wchar_t *s;
        if ((s = (wchar_t *)malloc((maxLen+1) * sizeof(wchar_t))) == NULL){
            puts("Error: Could not allocate memory");
            exit(1);
        }
        s[maxLen] = L'\0';
        int i;
        for(i = 0; i < maxLen; i++){
            if (i < maxLen-thisLen){
                s[i] = L' ';
            } else {
                s[i] = thisStr[i-(maxLen-thisLen)];
            }
        }
        // Print new this side
        printf( "%ls:", s );
        free(s);
    }
    return 0;
}

