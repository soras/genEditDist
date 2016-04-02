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

#include <stdio.h>
#include <string.h>
#include <float.h>
#include "List.h"
#include <locale.h>
#include <wctype.h>
#include <unistd.h>  /* For parsing command line args. */

#include "FindEditDistanceMod.h"  /* Methods for calculating generalized edit distance. */
#include "ShowTransformations.h"  /* Methods for backtracing and printing transformations. */

/**
*  Default cost for the 'replace' operation in regular edit distance.
*/
double rep = 1;
/**
*   Default cost for the 'remove' operation in regular edit distance.
*/
double rem = 1;
/**
*   Default cost for the 'add' operation in regular edit distance.
*/
double add = 1;


/**
*    Trie for 'replace' operations in generalized edit distance; 
*
*    Used for search;
*/
Trie *t;
/**
*    Trie for 'add' operations in generalized edit distance; Separate 
*    tries for 'add' and 'remove' make look-up more efficient: no need
*    to browse through the 'replace' tree in order to find suitable 
*    transformations.
*
*    Used for search;
*/
ARTrie *addT;
/** 
*    Trie for 'remove' operations in generalized edit distance; Separate 
*    tries for 'add' and 'remove' make look-up more efficient: no need 
*    to browse through the 'replace' tree in order to find suitable 
*    transformations.
*
*    Used for search;
*/
ARTrie *remT;


/**
*    Trie for 'replace' operations in generalized edit distance;
*
*    Used for backtracing the transformations, so, compared to \a *t ,
*    'left'-sides of the transformations are reversed in this trie.
*/
Trie *traceT = NULL;
/**
*    Trie for 'add' operations in generalized edit distance;
*
*    Used for backtracing the transformations, so, compared to \a *addT ,
*    strings are reversed in this trie.
*/
ARTrie *traceAddT = NULL;
/**
*    Trie for 'replace' operations in generalized edit distance;
*
*    Used for backtracing the transformations, so, compared to \a *remT ,
*    strings are reversed in this trie.
*/
ARTrie *traceRemT = NULL;


/**
*   First element of the ignore case list. The list contains upper-case
*  to lower-case transformations that are used to make the search case 
*  insensitive. One should note the following: if some element already 
*  exists in the list, but it is reinserted into the list, the reinserted 
*  one is added to the end of list, however, only the first element will 
*  be ever used during the search.
*/
IgnoreCaseListElement *ignoreCase;

/**
*   Indicates, whether case insensitive search mode is used or not. If
*   value is 1, then case insensitive search is used. Transformations 
*   between lower-case and upper-case characters are given in a separate 
*   file.
*/
int caseInsensitiveMode = 0;

/**
*   Generalized edit distance of the last best match. Used only in Top N 
*   search mode (flag "-b") for memorizing the last best result in the list 
*   of best results.
*/
double lastBest;

/**
*   Indicates, whether debug information will be printed in system output.
*   If value is 1, then debug information will be printed.
*/
int debug = 0;

/**
*   Indicates, whether line number of every found match will be printed.
*   It can be used only in the maximum edit distance search mode (flag '-m').
*   The line number will be match's line number in the input dictionary, 
*   starting count from 0.
*/
int printLineNumbers = 0;

/** 
*  Indicates, whether changes in some substrings of the search string should
*  be blocked. If value == 1 (flag '-e' set), then blocking is on and 
*  characters '(', ')', '<'  and '>' have special meaning in the search string:
*  they surround blocked substrings. Blocking is applied via assigning high 
*  edit distance penalties for changes inside given regions 
*  (see \a changeSearchStringWithEd_pen and \a changeSearchStringWithGenEd_pen );
*/
int blockChangesInSearchString = 0;

/** 
*   Array of penalties that will be applied on changing the search string with 
*  regular edit distance operations. The size of array is \c |searchString|+2 
*  and positions of the array indicate following penalties:
*  <br>
*  \c changeSearchStringWithEd_pen[i] - a penalty for changing char at position 
*                                       \c (i-1) in the search string;<br>
*  \c changeSearchStringWithEd_pen[0] - a penalty for adding a character at the 
*                                       beginning of the search string;<br>
*  \c changeSearchStringWithEd_pen last element - a penalty for adding a character 
*                                                 at the end of the search string;<br>
 */
double *changeSearchStringWithEd_pen = NULL;

/** 
*   Array of penalties that will be applied on changing the search string 
*  with generalized edit distance operations. The size of array and penalty
*  positions are as same as described in \a changeSearchStringWithEd_pen .
*/
double *changeSearchStringWithGenEd_pen = NULL;


/** 
 *   A penalty value that is used in arrays \a changeSearchStringWithEd_pen and 
 *  \a changeSearchStringWithGenEd_pen to block changes. 
 */
#define CHANGE_PENALT  3000.0 

/**
*   Indicates, whether alignments with the search string should be printed
*   for each found match ( \a printAlignments=1 for printing the alignments ).
*   With the current implementation, the alignments are printed only under the 
*   following conditions:
*      -- the mode is the maximum edit distance search (flag '-m' set);
*      -- blocked regions are disabled (flad '-e' not set);
*      -- only full-extent match is used (flags '-p', '-i', '-s' not set);
*/
int printAlignments = 0;

/**
*   Indicates, whether transformation weights should be output while printing 
*   the alignments ( \a printAlignments=1 ). If \a printAlignTransfWeights=1 ,
*   then transformation weights are also printed.
*/
int printAlignTransfWeights = 0;

/**
*   Indicates, whether pretty-printing mode should be used while printing 
*   the alignments ( \a printAlignments=1 ). If \a printAlignmentsPretty=1 ,
*   then pretty-printing mode is used ( strings are padded with spaces in 
    order to better visualise the alignment ).
*/
int printAlignmentsPretty = 0;

/** 
*   Extracts blocked regions from given search string, fills arrays 
*  \a changeSearchStringWithEd_pen and \a changeSearchStringWithGenEd_pen 
*  with penalty information and removes blocked region symbols from 
*  the search string. Returns a pointer to the search string, where 
*  blocked region symbols are already removed. NB! After usage, memory
*  under the returned string and also under \a changeSearchStringWithEd_pen 
*  and \a changeSearchStringWithGenEd_pen must be released!
*
*  \param *searchString pointer to the search string. The search string 
*                       can contain blocked regions;
*  \param *searchStringLen length of the \a searchString (including symbols
*                          indicating blocked regions);
*  \return pointer to the modified search string (blocked regions removed);
*/
wchar_t *extractBlockedRegions(wchar_t *searchString, int *searchStringLen){
    int i;
    // =============================================================
    //    Find actual length of the search string
    //    (do not count blocked region symbols)
    // =============================================================
    int wlen = 0;  // real string length
    for (i = 0; i < *searchStringLen; i++){
        if (searchString[i] != '(' && searchString[i] != ')' &&
            searchString[i] != '<' && searchString[i] != '>'){
            wlen++;
        }
    }
    // Proceed only if there were any blocked regions ...
    if (0 < wlen && wlen < *searchStringLen){
        // Create arrays (masks) for penalties
        changeSearchStringWithEd_pen    = malloc((wlen + 2) * sizeof(double));
        changeSearchStringWithGenEd_pen = malloc((wlen + 2) * sizeof(double));
        if (changeSearchStringWithEd_pen != NULL && changeSearchStringWithGenEd_pen != NULL){
            /* Initialize masks with null penalty at each position */
            for (i = 0; i < wlen+2; i++){
                changeSearchStringWithEd_pen[i]    = 0.0;
                changeSearchStringWithGenEd_pen[i] = 0.0;
            }
            // =============================================================
            //  Mark down unchangable/blocked regions of the search string
            // =============================================================
            int no_eDist_allowed     = 0;
            int no_genEdDist_allowed = 0;
            int penVectorPos     = 1;
            for (i = 0; i < *searchStringLen; i++){
                if (searchString[i] == '(' || searchString[i] == '<'){
                    // beginning of a region
                    if (i == 1 && (no_eDist_allowed == 1 || no_genEdDist_allowed == 1)){
                        // double symbols at the beginning of the string: block changes
                        // (addings) to the beginning
                        changeSearchStringWithEd_pen[0] = CHANGE_PENALT;
                        if (searchString[i] == '<'){
                            changeSearchStringWithGenEd_pen[0] = CHANGE_PENALT;
                        }
                    } else {
                        // Start a new region
                        if (searchString[i] == '('){
                           no_eDist_allowed = 1;
                        }
                        if (searchString[i] == '<'){
                           no_genEdDist_allowed = 1;
                        }
                    }
                } else if (searchString[i] == ')' || searchString[i] == '>'){
                    // ending of a region
                    if (i == (*searchStringLen - 1)){
                        if (i - 1 >= 0 && (searchString[i-1] == ')' || searchString[i-1] == '>')){
                            // double symbols at the end of the string: block changes
                            // (addings) to the end
                            changeSearchStringWithEd_pen[wlen + 1] = CHANGE_PENALT;
                            if (searchString[i] == '>'){
                                changeSearchStringWithGenEd_pen[wlen + 1] = CHANGE_PENALT;
                            }
                        }
                    } else {
                        // end a region
                        if (searchString[i] == ')'){
                            no_eDist_allowed     = 0;
                        }
                        if (searchString[i] == '>'){
                            no_genEdDist_allowed = 0;
                        }
                    }
                } else {
                    // in the middle of a region: make "unchangable"
                    if (no_eDist_allowed == 1){
                        changeSearchStringWithEd_pen[penVectorPos] = CHANGE_PENALT;
                    }
                    if (no_genEdDist_allowed == 1){
                        changeSearchStringWithEd_pen[penVectorPos]    = CHANGE_PENALT;
                        changeSearchStringWithGenEd_pen[penVectorPos] = CHANGE_PENALT;
                    }
                    penVectorPos++;
                }
            }
            // =============================================================
            //    Remove region symbols from the search string
            // =============================================================
            wchar_t *newSearchString;
            /* malloc the necessary space */
            if ((newSearchString = (wchar_t *)malloc((wlen + 1) * sizeof(wchar_t))) == NULL){
                puts("Error: Could not allocate memory");
                exit(1);
            }
            /* copy values from the old string */
            int searchStringPos = 0;
            for (i = 0; i < *searchStringLen; i++){
               if (searchString[i] != '(' && searchString[i] != ')' &&
                   searchString[i] != '<' && searchString[i] != '>'){
                   newSearchString[searchStringPos] = searchString[i];
                   searchStringPos++;
               }
            }
            /* ensure NULL-termination */
            newSearchString[wlen] = L'\0';
            /* replace old string, update length variable */
            free(searchString);
            searchString = newSearchString;
            *searchStringLen = wlen;
        } else {
            puts("Error: Could not allocate memory");
            exit(1);
        }
    }
    //for (i = 0; i < *searchStringLen+2; i++){
    //   printf("--- %f %f \n", changeSearchStringWithEd_pen[i], changeSearchStringWithGenEd_pen[i]);
    //}
    return searchString;
}

/**
*  Finds generalized edit distances between \a string and each word in \a file, outputs 
*  matches with distance <i>less than or equal to</i> \c editD . According to contents of
*  \a flagsInPositions , up to four different matches (full, prefix, suffix, infix matches)
*  can be calculated for every word in \a file - if at least one match has a score 
*  <i>less than or equal to</i> \c editD , the word will appear in the output as a match.
*
*  \param *file a dictionary file where the search will be conducted. Words in the file 
*               should be separated with line breaks;
*  \param *string the search string
*  \param stringLen length of the search string
*  \param editD maximum generalized edit distance score. All matches exceeding the score will
*               be discarded
*  \param flagsInPositions indicates, which of the 4 different match types should be calculated
*/
int findDistances(char *file, wchar_t *string, int stringLen, double editD, char flagsInPositions[FP_MAX_POSITIONS]){
    long lineNR = 0;
    int i = 0;
    int j = 0;
    int datalen;
    char* str;
    wchar_t* wstr     = NULL;
    wchar_t* wstr_new = NULL;
    int wLen;

    datalen = strlen(file);
    str = malloc(2);

    if(caseInsensitiveMode)
        string = makeStringToIgnoreCase(string, stringLen);

    while(i < datalen){
        while(j < datalen && file[j] != '\n' && file[j] != '\r')
            j++;

        str = (char *)realloc(str, (j-i+1));
        if(str == NULL){
           perror("Memory");
           exit(1);
        }
        str[j-i] ='\0';
        strncpy(str, (file+i), (j-i));

        wstr = (wchar_t *)localeToWchar(str);
        wLen = mbstowcs(NULL, str, 0);

        double fullED = DBL_MAX;
        double prefED = DBL_MAX;
        double suffED = DBL_MAX;
        double infxED = DBL_MAX;

        // make string to ignore case
        if(caseInsensitiveMode){
            wstr_new = copy_wchar_t(wstr, wLen);
            wstr_new = makeStringToIgnoreCase(wstr_new, wLen);
        }

        // find different types of matches, according to flagsInPositions
        int pos = 0;
        while ((pos < FP_MAX_POSITIONS) && (flagsInPositions[pos] != L_EMPTY)){
            switch (flagsInPositions[pos++]){
                case L_FULL:
                     fullED = genEditDistance_full(string, 
                                                   ((caseInsensitiveMode)?(wstr_new):(wstr)),
                                                   stringLen, 
                                                   wLen);
                     break;
                case L_PREFIX:
                     prefED = genEditDistance_prefix(string, 
                                                     ((caseInsensitiveMode)?(wstr_new):(wstr)),
                                                     stringLen, 
                                                     wLen); 
                     break;
                case L_SUFFIX:
                     suffED = genEditDistance_suffix(string, 
                                                     ((caseInsensitiveMode)?(wstr_new):(wstr)),
                                                     stringLen, 
                                                     wLen); 
                     break;
                case L_INFIX:
                     infxED = genEditDistance_middle(string, 
                                                     ((caseInsensitiveMode)?(wstr_new):(wstr)),
                                                     stringLen, 
                                                     wLen); 
                     break;
            }
        }

        if(fullED <= editD || prefED <= editD || suffED <= editD || infxED <= editD){
            puts("------------------------");
            if (printLineNumbers){
                printf("%ld\n", lineNR);
            }
            puts(str);
            // print different scores, according to flagsInPositions
            pos = 0;
            int flagsUsed = 0;
            while ((pos < FP_MAX_POSITIONS) && (flagsInPositions[pos] != L_EMPTY)){
               switch (flagsInPositions[pos++]){
                 case L_FULL:
                              printf("%f", fullED);
                              flagsUsed++;
                              break;
                 case L_PREFIX:
                              printf("%f", prefED);
                              flagsUsed++;
                              break;
                 case L_SUFFIX:
                              printf("%f", suffED);
                              flagsUsed++;
                              break;
                 case L_INFIX:
                              printf("%f", infxED);
                              flagsUsed++;
                              break;
              }
              printf(" ");
            }
            printf("\n");
            
            // if required, trace and print transformations
            if(printAlignments > 0 && fullED <= editD && 
               blockChangesInSearchString == 0 && flagsUsed == 1){
                Transformations *transF = createTransformations();
                genEditDistance(string, 
                                ((caseInsensitiveMode)?(wstr_new):(wstr)),
                                stringLen,
                                wLen, transF);
                printTransformations(string, ((caseInsensitiveMode)?(wstr_new):(wstr)), 
                                     transF, caseInsensitiveMode, 
                                     printAlignments, 
                                     printAlignTransfWeights, 
                                     printAlignmentsPretty);
                //printf("  Removal list: %i ",debugRemovalListLen(transF));
                removeTransformations(transF);
            }
            
        }

        free(wstr);
        if(caseInsensitiveMode)
            free(wstr_new);

        lineNR++;
        if(file[j] == '\r')
            j +=2;
        else j++;
        i = j;
    }
    free(str);
    return 0;
}

/**
*  Finds generalized edit distances between \a string and each word in \a file, outputs 
*  first \a best matches. \a flag indicates, which of the four different match types
*  (full, prefix, suffix, infix) is calculated. Note that the number \a best is allowed
*  to be exceeded, if there are multiple equal-score matches for the last position;
*
*  \param *file a dictionary file where the search will be conducted. Words in the file 
*               should be separated with line breaks;
*  \param *string the search string
*  \param stringLen length of the search string
*  \param best maximum number of best matches allowed in output. 
*  \param flag indicates, which of the 4 different match types should be calculated
*/
int findBest(char *file, wchar_t *string, int stringLen, int best, char flag){
    /*
     * Maximum number of best matches allowed in output. Note that the number is 
     * allowed to be exceeded, if there are multiple equal-score matches for the 
     * last position in top;
     */
    int nrOfBestStrings = best;

    int i = 0;
    List *l = createList();
    int j = 0;
    ListItem *item;
    Index *index;

    int datalen;
    char* str;
    wchar_t* wstr;
    int wLen;

    datalen = strlen(file);
    str = malloc(2);

    while(i < datalen){
        while(j < datalen && file[j] != '\n' && file[j] != '\r')
            j++;

        str = (char *)realloc(str, (j-i+1));

        if(str == NULL){
          perror("Memory");
          exit(1);
        }

        str[j-i] ='\0';
        strncpy(str, (file+i), (j-i));

        wstr = (wchar_t *)localeToWchar(str);
        wLen = mbstowcs(NULL, str, 0);

        if(caseInsensitiveMode){
            string = makeStringToIgnoreCase(string, stringLen);
            wstr = makeStringToIgnoreCase(wstr, wLen);
        }

        double ed;
        // find match according to type indicated in flag
        switch (flag){
           case L_PREFIX:
                ed = genEditDistance_prefix(string, wstr, stringLen, wLen);
                break;
           case L_INFIX:
                ed = genEditDistance_middle(string, wstr, stringLen, wLen);
                break;
           case L_SUFFIX:
                ed = genEditDistance_suffix(string, wstr, stringLen, wLen);
                break;
           case L_FULL:
           default:
                ed = genEditDistance_full(string, wstr, stringLen, wLen);
                break;
        }
        
        // there's room in the list
        if(best > 0){
           best = insertListItem(l, ed, i, j, 0, best);
           // printList(l);
        }
        else if(best == 0 && ed <= lastBest){
           insertListItem(l, ed, i, j, 1, 0);
        }

        free(wstr);

        if(file[j] == '\r')
            j +=2;
        else j++;
            i = j;
    }

    /* printing the result */
    item = l->firstItem;
    /*
    *  Output best results by counting matches from the beginning. If 
    *  there are multiple equal-score matches for the last position, 
    *  the number of best results can be exceeded.
    */
    long countBest = 0; 
    while(item != NULL){
        puts("------------------------");
        printf("%f \n", item->value);
        index = item->index;
        while(index != NULL){
            str = (char *)realloc(str, (index->j - index->i + 1));
            if(str == NULL){
                perror("Memory");
                exit(1);
            }

            str[index->j - index->i] ='\0';
            strncpy(str, (file + index->i), (index->j - index->i));
            puts(str);

            index = index->nextIndex;
            countBest++;
        }
        // Is it enough already?
        if (countBest >= nrOfBestStrings){
           break;
        }
        item = item->nextItem;
    }
    freeList(l);
    free(str);
    return 0;
}

/**
*  Outputs help information about the program.
*
*  \param *prog name of the executable
*/
int helpInfo(char *prog){
   puts("Usage:");
   printf("1) %s -m maxED [-lepsfi] [-awy] file_A  string  file_B  [file_C]\n", prog);
   puts("   ");
   puts("   Computes generalized edit distances between <string> and strings in");
   puts("   <file_B>. Outputs all strings which have distance <= maxED;");
   puts("   ");
   puts("  -m maxED       - maximum edit distance;");
   puts("  file_A         - file where the additional edit operations could be found;");
   puts("  string         - a string to search for;");
   puts("  file_B         - file from where to search for matches with given string;");
   puts("  file_C         - file containing upper-to-lower case translations, to ignore");
   puts("                   case during match finding (Non-mandatory argument);\n");
   printf("2) %s -b N [-p|s|f|i] [-e] file_A  string  file_B  [file_C]\n", prog);
   puts("   ");
   puts("   Computes generalized edit distances between <string> and strings in");
   puts("   <file_B>. Outputs top N strings closest to the search string. The number");
   puts("   N is allowed to be exceeded when there are equal-distance strings for");
   puts("   the last place in top.");
   puts("   ");
   puts("  -b N           - number of strings with best edit distances to be displayed;");
   puts("  file_A         - file where the additional edit operations could be found;");
   puts("  string         - a string to search for;");
   puts("  file_B         - file from where to search for matches with given string;");
   puts("  file_C         - file containing upper-to-lower case translations, to ignore");
   puts("                   case during match finding (Non-mandatory argument);\n");
   printf("Optional flags:\n");
   puts("  -f  finds edit distance between full extent strings (default);");
   puts("  -s  finds edit distance between search string and some suffix of text;");
   puts("  -p  finds edit distance between search string and some prefix of text;");
   puts("  -i  finds edit distance between search string and some infix of text;");
   puts("  -l  prints line number before found match (can be used only with '-m');");
   puts("  -e  allows to mark unchangable areas in <string>. Example markings:");
   puts("");
   puts("    (ab)cde(f)) = the prefix 'ab' can't be modified by regular edit dist");
   puts("                  the suffix 'f' can't be modified by regular edit dist");
   puts("                  after the suffix 'f', regular edit dist ADD can't be applied");
   puts("");
   puts("    <ab>cdef    = the prefix 'ab' can't be modified either with regular edit");
   puts("                  distance nor with generalized edit distance.");
   puts("");
   puts("  -a  prints alignments between the search string and each full distant match");
   puts("      Can only be used with flag '-m',  when  flags '-e', '-i', '-s', '-p'");
   puts("      are not set. Has the following suboptions: ");
   puts("        -y  Uses pretty-printing;");
   puts("        -w  Prints weights of transformations;");
   puts("");
   exit(0);
}


/**
*  Main method of the Generalized Edit Distance Tool. Parses input arguments 
*  (format described in output of \c helpInfo()) and performs generalized edit
*  distance calculations.
*/
int main(int argc, char* argv[]){

  // file definitions
  char *filename;
  char *searchString;
  char *wordsFile;
  char *ignoreCaseFile;
  char *data;
  char *words;

  /* set locale */
  if (!setlocale(LC_CTYPE, "")) {
    fprintf(stderr, "Can't set the specified locale! "
                    "Check LANG, LC_CTYPE, LC_ALL.\n");
    return 1;
  }

  // Default costs for regular edit distance operations
  add = rep = rem = 1.0;
  char *err;
  wchar_t *wSearch;
  int wlen;

  // Flags from the command line

  // Array, which indicates the presence and order of the 
  // different match types in the output.
  char flagsInPositions[FP_MAX_POSITIONS];   
  int curInFlags;  // position index in flagsInPositions
  // Set default values: make all flags unset
  for ( curInFlags = 0; curInFlags < FP_MAX_POSITIONS; curInFlags++ ) 
    flagsInPositions[curInFlags] = L_EMPTY; 
  curInFlags = 0;  
  // By default, the flag in first position is -f
  // It may be overwritten by the arguments from command line
  flagsInPositions[curInFlags] = L_FULL; 

  // Number of best results (costs)
  // If the value is >= 0, number of best matches will be output
  int best = -1;

  // Maximum edit distance threshold
  // If the value is >= 0, all matches having distance >= max will be output;
  double max = -1.0;

  // Parse flags from the command line
  int c;
  char *argForOpt;
  while ((c = getopt(argc, argv, "b:m:elpisf?awy")) != -1){
    switch (c){
      case 'f':
         if (curInFlags < FP_MAX_POSITIONS) flagsInPositions[curInFlags++] = L_FULL;
         break;
      case 'p':
         if (curInFlags < FP_MAX_POSITIONS) flagsInPositions[curInFlags++] = L_PREFIX;
         break;
      case 'i':
         if (curInFlags < FP_MAX_POSITIONS) flagsInPositions[curInFlags++] = L_INFIX;
         break;
      case 's':
         if (curInFlags < FP_MAX_POSITIONS) flagsInPositions[curInFlags++] = L_SUFFIX;
         break;
      case 'b':
         argForOpt = optarg;
         // Number of best results (costs)
         best = strtoul(argForOpt, &err, 10);
         break;
      case 'l':
         printLineNumbers = 1;
         break;
      case 'e':
         blockChangesInSearchString = 1;
         break;
      case 'm':
         argForOpt = optarg;
         // Maximum edit distance threshold
         max = strtod(argForOpt, &err);
         break;
      case 'a':
         printAlignments = 1;
         break;
      case 'w':
         printAlignTransfWeights = 1;
         break;
      case 'y':
         printAlignmentsPretty = 1;
         break;
      case '?':
         helpInfo(argv[0]);
         return 0;
    }
  }

  // There must be at least 3 arguments left: transformations file, search string and dictionary file
  if (argc - optind < 3){
     printf("Wrong number of arguments: %i \n",argc-1);
     helpInfo(argv[0]);
     return 1;
  }

  // EXACTLY ONE of the flags '-b' and '-m' must be set
  if ((best < 0 && max < 0.0) || (best >= 0 && max >= 0.0)){
     printf("Exactly one of the flags '-b' and '-m' must be set; \n");
     helpInfo(argv[0]);
     return 1;
  }

  // Parse remaining arguments
  int i;
  for (i = 0; optind + i < argc; i++){
      switch (i){
          case 0: filename     = argv[optind + i]; break;
          case 1: searchString = argv[optind + i]; break;
          case 2: wordsFile    = argv[optind + i]; break;
          case 3:{
                    caseInsensitiveMode = 1;
                    // ignore case file
                    ignoreCaseFile = (char *)readFile(argv[optind + i]);
                    ignoreCaseListFromFile(ignoreCaseFile);
                 }
                 break;
      }
  }

  /* creating tries */
  t = createTrie();
  addT = createARTrie();
  remT = createARTrie();
  
  /* if printing alignments is required, create also tries for backtracing */
  if (printAlignments > 0){
      traceT    = createTrie();
      traceAddT = createARTrie();
      traceRemT = createARTrie();
  }

  /* read transformations file and build trie-structures */
  data = (char *)readFile(filename);
  trieFromFile(data);

  /* the search word */
  wSearch = (wchar_t*)localeToWchar(searchString);
  wlen = mbstowcs(NULL, searchString, 0);

  /* extract blocked regions and create mask of penalties */
  if ( blockChangesInSearchString == 1  &&  wlen > 0 ){
      wSearch = extractBlockedRegions(wSearch, &wlen);
      //printf(" (%ls) (%i) \n", wSearch, wlen);
  }

  /* read dictionary file */
  words = (char *)readFile(wordsFile);

  if (best >= 0.0){
     // ***************
     //  Output matches on best distances
     // ***************
     // Find position of the last non-empty flag
     int lastPos = 0; 
     while ((lastPos < FP_MAX_POSITIONS) && (flagsInPositions[lastPos] != L_EMPTY)) lastPos++;
     findBest(words, wSearch, wlen, best, 
              flagsInPositions[lastPos-1] // match type: according to flag in last position
             );
  } else {
     // ***************
     //  Output matches that are inside given maximum edit distance threshold
     // ***************
     findDistances(words, wSearch, wlen, max, 
                   flagsInPositions  // for every match: output all scores of different types
                  );
  }
  
  
  /* release used memory */

  if (changeSearchStringWithEd_pen != NULL){
     free(changeSearchStringWithEd_pen);
  }

  if (changeSearchStringWithGenEd_pen != NULL){
     free(changeSearchStringWithGenEd_pen);
  }
  
  if (wSearch != NULL){
     free(wSearch);
  }
  if (ignoreCase != NULL){
     freeIgnoreCaseList();
  }
  if (words != NULL){
     munmap(words, strlen(words));
  }

  // Searching tries
  if (t != NULL){
     freeTrie(t);
  }
  if (addT != NULL){
     freeARTrie(addT);
  }
  if (addT != NULL){
     freeARTrie(remT);
  }
  // Backtracing tries
  if (traceT != NULL){
     freeTrie(traceT);
  }
  if (traceAddT != NULL){
     freeARTrie(traceAddT);
  }
  if (traceRemT != NULL){
     freeARTrie(traceRemT);
  }
  return 0;

}


