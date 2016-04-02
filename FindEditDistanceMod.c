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

#include "FindEditDistanceMod.h"

// Insert value into table[row][col]
int addValueToTable(int cols, double table[][cols], int row, int col, double value){
  if(value < table[row][col])
     table[row][col] = value;
  return 0;
}

// Search 'remove' operations from trie and apply if possible
int searchFromRemTrie(int cols, double table[][cols], wchar_t *string, int i , int j){
   ARTNode *tmp;
   double value;
   int r = 1;

   tmp = remT->firstNode;
   value = table[i][j] + getPenaltOfChangingPosWithGenEd(i);
   while(tmp != NULL && string != L'\0'){
     if(tmp->label == *string){
        /* If given node in trie is an end node: */
        if(tmp->value != DBL_MAX){
            // Operation: at position i. in the search string, remove following r characters
            addValueToTable(cols, table, (i+r), j, (value + tmp->value));
        }
        string = string + 1;
        tmp = tmp->nextNode;
        value += getPenaltOfChangingPosWithGenEd(i+r);
        r++;
     }
     else tmp = tmp->rightNode;
   }
   return 0;
}

// Search 'add' operations from trie and apply if possible
int searchFromAddTrie(int cols, double table[][cols], wchar_t *string, int i , int j){
  ARTNode *tmp;
  double value;
  int c = 1;

  tmp = addT->firstNode;
  value  = table[i][j] + getPenaltOfChangingPosWithGenEd(i);
  while(tmp != NULL && string != L'\0'){
    if(tmp->label == *string){
       /* If given node in trie is an end node: */
       if(tmp->value != DBL_MAX){
           // Operation: at position i. in the search string, insert c characters
           addValueToTable(cols, table, i, (j+c), (value + tmp->value));
       }
       string = string + 1;
       tmp = tmp->nextNode;
       c++;
    }
    else tmp = tmp->rightNode;
  }
  return 0;
}

// Checks from EndNode list, whether there are replacements corresponding to string, and tries to apply them
int findReplacement(int cols, EndNode *n, double table[][cols], wchar_t *string, int i, int j, double value){
  wchar_t *repl;
  int c;

  while(n != NULL){
     repl = n->edit;
     c = 0;
     // Check, whether given replacement can be made
     while(1){
        if(string[c] == repl[c] || repl[c] == L'\0'){
            if(repl[c] == '\0'){   
                 // End of replacement string
                 addValueToTable(cols, table, i, (j + c), (value + n->value));
                 goto nextRepl;
            }
            c++;
        }
        else break;
     }
     nextRepl: n = n->nextEN;
  }
  return 0;
}

// Search 'replace' operations from trie and apply if possible
int searchFromRepTrie(int cols, double table[][cols], wchar_t *string1, wchar_t *string2, int i, int j){
    TrieNode *tmp;
    EndNode *repl;
    double value;
    int r;

    tmp = t->firstNode;
    value = table[i][j] + getPenaltOfChangingPosWithGenEd(i);
    r = 1;
    while(tmp != NULL){
      if(tmp->label == *string1){
        /* If given node in trie is an end node: */
        if(tmp->replacement != NULL){
            // Operation: from position i. in the search string, replace next r characters (including i.)
            repl = tmp->replacement;
            findReplacement(cols, repl, table, string2, (i+r), j, value);
        }
        string1 = string1 + 1;
        tmp = tmp->nextNode;
        value += getPenaltOfChangingPosWithGenEd(i+r);
        r++;
      }
      else tmp = tmp->rightNode;
   }
   return 0;
}

// Gets a penalty which applies for changing i-th char in the search string via edit distance
double getPenaltOfChangingPos(int i){
  if (changeSearchStringWithEd_pen != NULL){
     return changeSearchStringWithEd_pen[ i + 1 ];
  }
  return 0.0;
}

// Gets a penalty which applies for changing i-th char in the search string via generalized edit distance
double getPenaltOfChangingPosWithGenEd(int i){
  if (changeSearchStringWithGenEd_pen != NULL){
     return changeSearchStringWithGenEd_pen[ i + 1 ];
  }
  return 0.0;
}

// Finds regular edit distance between strings a and b.
int editDistance(wchar_t* a, wchar_t* b, int aLen, int bLen){
  int i, j;
  int rows = aLen + 1;
  int cols = bLen +1;
  int table[rows][cols];

  // fill the first row
  for(i = 0; i < rows; i++){
     table[i][0] = i;
  }
  for(j = 1; j < cols; j++){
     table[0][j] = j;
     for(i = 1; i < rows; i++){
         if(*(a+(i-1)) == *(b+(j-1))){
              table[i][j] = min(table[i-1][j-1],
                   min(table[i][j-1]+1, table[i-1][j]+1));
         } else
              table[i][j] = min(table[i-1][j-1]+1,
                   min(table[i][j-1]+1, table[i-1][j]+1));
     }
  }
  return table[rows-1][cols-1];
}

// Finds generalized edit distance between strings a and b, without applying any penalties
double genEditDistance(wchar_t *a, wchar_t *b, int aLen, int bLen, Transformations *transF){
  int i, j;
  int rows = aLen +1;  // search string
  int cols = bLen +1;  // text
  double table[rows][cols];
  double value;

  /*  Fill table with initial values (so we can check applicability of generalized 
  * edit distance transformations even if we haven't reached to the particular 
  * cell yet )
  */
  for(i = 0; i < rows; i++){
     for(j = 0; j < cols; j++){
        table[i][j] = DBL_MAX;
     }
  }
  table[0][0] = 0;
  // fill the first column
  for(i = 1; i < rows; i++){
     if(remT->firstNode != NULL)
        searchFromRemTrie(cols, table, (a + i-1), i-1 ,0);
     value = table[i-1][0] + rem;
     if(value < table[i][0]) table[i][0] = value;
  }

  for(j = 1; j < cols; j++){
     if(addT->firstNode != NULL)
        searchFromAddTrie(cols, table, (b + j - 1), 0, j-1);
     value = table[0][j-1] + add;
     if(value < table[0][j]) table[0][j] = value;

     for(i = 1; i < rows; i++){
        if(remT->firstNode != NULL)
           searchFromRemTrie(cols, table, (a + i-1), i-1 ,j);
        if(addT->firstNode != NULL)
           searchFromAddTrie(cols, table, (b + j - 1), i, j-1);
        if(t->firstNode != NULL)
           searchFromRepTrie(cols, table, (a + i-1 ), b + j-1 , i-1, j-1);

        if(a[i-1] == b[j-1]){
           value = min(table[i-1][j-1],
                   min(table[i][j-1]+ add,
                       table[i-1][j] + rem ));
           if(value < table[i][j]) table[i][j] = value;
        } else {
           value = min(table[i-1][j-1] + rep,
                   min(table[i][j-1]+ add, 
                       table[i-1][j] + rem ));
           if(value < table[i][j]) table[i][j] = value;
        }
     }
  }

  /* Print table for debug */
  /*
   puts("\n");
   printf("%ls",b);
   puts("\n");
   for(i = 0; i < rows; i++){
       for(j = 0; j < cols; j++){
          printf("%f ",table[i][j]);
       }
       puts("\n");
   }
   puts("\n");
  */
  if (transF != NULL){
      // if required, backtrace the transformations
      findBestPaths(cols, table, a, b, aLen, bLen, transF, traceRemT, traceAddT, traceT);
  }
  return table[rows-1][cols-1];
}

// Finds generalized edit distance between strings a and b, also applies penalties if possible
double genEditDistance_pens(wchar_t *a, wchar_t *b, int aLen, int bLen, double* start_pen, double* end_pen){
  int i, j;
  int rows = aLen +1;  // search string
  int cols = bLen +1;  // text
  double table[rows][cols];
  double value;

  /*  Fill table with initial values (so we can check applicability of generalized 
  * edit distance transformations even if we haven't reached to the particular 
  * cell yet )
  */
  for(i = 0; i < rows; i++){
     for(j = 0; j < cols; j++){
        table[i][j] = DBL_MAX;
     }
  }

  table[0][0] = 0;

  // Fill the first row with start penalties (penalty of starting match at given pos in text)
  if (start_pen != NULL){
     for(j = 0; j < bLen; j++){
        table[0][j] = start_pen[j];
     }
  }

  // fill the first column
  for(i = 1; i < rows; i++){
     if(remT->firstNode != NULL)
         searchFromRemTrie(cols, table, (a + i-1), i-1 ,0);
     value = table[i-1][0] + rem + getPenaltOfChangingPos(i-1);  // regular deletion at the search string pos i.
     if(value < table[i][0]) table[i][0] = value;
  }

  for(j = 1; j < cols; j++){
    if(addT->firstNode != NULL)
      searchFromAddTrie(cols, table, (b + j - 1), 0, j-1);
    value = table[0][j-1] + add + getPenaltOfChangingPos(-1);   // adding at the beginning of the search string
    if(value < table[0][j]) table[0][j] = value;
    for(i = 1; i < rows; i++){
        if(remT->firstNode != NULL)
          searchFromRemTrie(cols, table, (a + i-1), i-1 ,j);
        if(addT->firstNode != NULL)
          searchFromAddTrie(cols, table, (b + j - 1), i, j-1);
        if(t->firstNode != NULL)
          searchFromRepTrie(cols, table, (a + i-1 ), b + j-1 , i-1, j-1);

        if(a[i-1] == b[j-1]){
            value = min(table[i-1][j-1],                                      // identity at search string pos i. 
                    min(table[i][j-1] + add + getPenaltOfChangingPos(i),      // insert after search string pos i. 
                        table[i-1][j] + rem + getPenaltOfChangingPos(i-1) )); // delete from search string pos i. 
            if(value < table[i][j]) table[i][j] = value;
        } else {
            value = min(table[i-1][j-1] + rep + getPenaltOfChangingPos(i-1),  // replace at search string pos i.
                        min(table[i][j-1] + add + getPenaltOfChangingPos(i),  // insert after search string pos i. 
                        table[i-1][j] + rem + getPenaltOfChangingPos(i-1) )); // delete from search string pos i. 
            if(value < table[i][j]) table[i][j] = value;
        }

        //
        // NB! The 'add penalty' above { table[i][j-1] + add + getPenaltOfChangingPos(i) } is useful
        // only when the position i+1 (following position i) is also penalized or if it is the end of word.
        // If the position i+1 is free (unpenalized), the blocked adding can be still done by replacements
        // at position i+1 and additions after i+1.
        //
    }
  }

  double score = DBL_MAX;
  if (end_pen == NULL){
     score = table[rows-1][cols-1];
  } else {
      // Fill the last row with end penalties (penalty of ending match at given pos in text)
      for(j = 0; j < bLen; j++){
          double newScore = table[rows-1][j+1] + end_pen[j];
          if (newScore < score){
            score = newScore;
          }
      }
  }
  return score;
}

// Finds generalized edit distance between strings a and b, allowing only partial matches with b
double genEditDistance_mod(wchar_t *a, wchar_t *b, int aLen, int bLen, short isPrefix, short isSuffix){
    double prefix[bLen];
    double suffix[bLen];

    double* prefix_ptr = NULL;
    double* suffix_ptr = NULL;

    int i;
    if (!isPrefix){
      for (i = 0; i < bLen; i++) prefix[i] = 0.0;
      prefix_ptr = prefix;
    }
    if (!isSuffix){
      for (i = 0; i < bLen; i++) suffix[i] = 0.0;
      suffix_ptr = suffix;
    }

    return genEditDistance_pens(a, b, aLen, bLen, prefix_ptr, suffix_ptr);
}

// Finds generalized edit distance between strings a and prefix of b, allows penalizing changes in search string
double genEditDistance_prefix(wchar_t *a, wchar_t *b, int aLen, int bLen){
  return genEditDistance_mod(a, b, aLen, bLen, 1, 0);
}

// Finds generalized edit distance between strings a and suffix of b, allows penalizing changes in search string
double genEditDistance_suffix(wchar_t *a, wchar_t *b, int aLen, int bLen){
  return genEditDistance_mod(a, b, aLen, bLen, 0, 1);
}

// Finds generalized edit distance between strings a and infix of b, allows penalizing changes in search string
double genEditDistance_middle(wchar_t *a, wchar_t *b, int aLen, int bLen){
  return genEditDistance_mod(a, b, aLen, bLen, 0, 0);
}

// Finds generalized edit distance between full strings a and b, allows penalizing changes in search string
double genEditDistance_full(wchar_t *a, wchar_t *b, int aLen, int bLen){
  return genEditDistance_mod(a, b, aLen, bLen, 1, 1);
}

// Prints a view of debug table
void printTableWithChangingPenalties(
     wchar_t *a, wchar_t *b, int aLen, int bLen, int rows, int cols, double table[rows][cols]){
    int i, j;

    puts("\n");
    printf("%ls",b);
    puts("\n");
    for(i = 0; i < rows; i++){  // in search string
        int m = 0;
        for(m = 0; m < 3; m++){
            for(j = 0; j < cols; j++){  // in text
              if (i > 0 && j == 0) {
                  printf( "%5.1f  ", table[i-1][0] + rem + getPenaltOfChangingPos(i-1) );
              } else if (i > 0 && j > 0){
                  if (m==0){
                      if(a[i-1] == b[j-1]){
                          printf( "Sme: %5.1f  ",table[i-1][j-1] );
                      } else {
                          printf( "Rep: %5.1f  ",table[i-1][j-1] + rep + getPenaltOfChangingPos(i-1) );
                      }
                  } 
                  else if (m==1){
                      printf( "Add: %5.1f  ",table[i][j-1]+ add + getPenaltOfChangingPos(i) );
                  } 
                  else if (m==2){
                      printf( "Rem: %5.1f  ",table[i-1][j] + rem + getPenaltOfChangingPos(i-1) );
                  }
              } else {
                  printf( "%5.1f  ",table[i][j] );
              }
            }
            printf("\n");
        }
        printf("\n");
    }
}
