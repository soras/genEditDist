================================================
 Generalized edit distance command line tool
================================================

  GenEditDist tool allows to find approximate matches between a search 
  string and list of strings in a dictionary. In addition to the regular 
  edit distance (the Levenshtein distance), a set of weighted transformations 
  can be used in a search.

  The tool has two working modes:
   *) Find all the matches that are within the maximum edit distance limit;
   *) Find top N closest matches;

  Currently, the tool has been developed and tested only on UNIX platform 
  and it is not expected to work on other platforms.


 1. Environment and input files
-----------------------------------------

  The tool expects that all its inputs are in UTF8 encoding. Before using 
  the tool, the environment should be set to support UTF8 in command line.

  One possible way to do it is with bash command:
  > export LANG=et_EE.utf8

  An input 'dictionary' where search is performed should be a text file, 
  where each entry (a match candidate) is on new line.

  The transformations file should also be a text file, where each 
  transformation is on separate line. Transformation from string A to 
  string B with cost W should be formatted in file as:

         A:B:W

  A or B (but not both together) can be omitted to define an addition or 
  a deletion.
  
  Transformations file should only contain transformations in given format, 
  adding empty lines or comments might result in unexpected search behaviour.


 2. Using the program
-----------------------------------------

 2.1. Full vs partial match
-----------------------------------------

  While calculating the generalized edit distance, one can specify whether 
  a full match or a partial match between the search string and a string in the 
  dictionary is found.

  Four modes of matching exist:
  
    A) Full extent match  (flag -f, default):
    
       Distance between the search string and a string in the dictionary
      is calculated considering both strings at full extent.
        
        Examples (**):
            search string: police
            dictionary string: polis
            full match distance: 0.9
              ( using transformation  ce:s:0.9 )

            search string: computer
            dictionary string: komputa
            full match distance: 2.4
              ( using transformations  
                  c:k:0.5   r::0.9   e:a:1.0 )
            
    B) Distance from some prefix of the dictionary word (flag -p):
    
       Distance between the search string and a string in the dictionary
      is calculated considering only a prefix of the dictionary word.
      
        Examples (**):
            search string: police
            dictionary string: polisman
            prefix match distance: 0.9
              ( using transformation  ce:s:0.9 )

            search string: book
            dictionary string: buksop
            prefix match distance: 0.5
              ( using transformation  oo:u:0.5 )

    C) Distance from some suffix of the dictionary word (flag -s):
    
       Distance between the search string and a string in the dictionary
      is calculated considering only a suffix of the dictionary word.

        Examples (**):
            search string: block
            dictionary string: aisblok
            suffix match distance: 0.5
              ( using transformation  ck:k:0.5 )

            search string: card
            dictionary string: postkat
            suffix match distance: 1.9
              ( using transformations 
                  c:k:0.5  r::0.9  d:t:0.5 )

    D) Distance from some infix of the dictionary word (flag -i):
    
       Distance between the search string and a string in the dictionary
      is calculated considering only an infix of the dictionary word.
      
        Examples (**):
            search string: belong
            dictionary string: glas bilong kapten
            infix match distance: 0.8
              ( using transformation  e:i:0.8 )

            search string: something
            dictionary string: i gat samting gut
            infix match distance: 2.2
              ( using transformations 
                  th:t:0.6  o:a:0.6   e::1.0  )
              
    These matching modes can be combined, e.g flag -fi states that both full 
   match and infix match are calculated.


 2.2. Finding all matches within maximum distance
-----------------------------------------------------

     Parameter -m <max_edit_distance> is used to find all the matches that 
    are within given maximum edit distance limit (inclusively). Usage:

       ./genEditDist -m <max_edit_distance> -[fpsile] <transfFile> <searchString> <dict> <caseFile>

          <max_edit_distance> == maximum edit distance limit;
          <transfFile> == file of user-defined transformations;
          <searchString> == the search string;
          <dict> == the dictionary file where the search is performed;
          <caseFile> == file containing upper-to-lowercase mappings
                        (optional, makes search case-insensitive)
       
       NB! If there are several edit distances computed for a single match 
       candidate (more than one of the flags   -f, -p, -s, -i   is set), then
       the candidate will be output if at least one of the distances is less 
       than or equal to <max_edit_distance>;

       Usage examples (**):

         Searching for string 'book' in the dictionary 'testdata/pidgin_words.txt' 
        using the transformations from file 'testdata/transformations.txt':
       
       ./genEditDist  -m 1.0  -fi  testdata/transformations.txt book testdata/pidgin_words.txt
       ------------------------
       buk
       0.500000 0.500000
       ------------------------
       buksop
       3.500000 0.500000
       ------------------------
       aisblok
       4.000000 1.000000


       ./genEditDist  -m 1.0  -fs  testdata/transformations.txt shop testdata/pidgin_words.txt
       ------------------------
       buksop
       3.600000 0.600000
       ------------------------
       hop
       1.000000 1.000000
       ------------------------
       sop
       0.600000 0.600000
       ------------------------
       woksop
       3.600000 0.600000


 2.3. Finding TOP N of closest matches 
-----------------------------------------------------

     Parameter -b <N> is used to find TOP N matches that are closest
    to the search string. Usage:

       ./genEditDist -b <number_of_best_matches> -[f|p|s|i] <transfFile> <searchString> <dict> <caseFile>

          <number_of_best_matches> == number of closest matches displayed;
          <transfFile> == file of user-defined transformations;
          <searchString> == the search string;
          <dictionary> == the dictionary file where the search is performed;
          <caseFile> == file containing upper-to-lowercase mappings
                        (optional, makes search case-insensitive)
          
       NB! The number of best matches is allowed to be exceeded when there 
       are equal-distance strings for the last place in top.
 
       Usage examples (**):


       ./genEditDist  -b 3 -fi  testdata/transformations.txt book testdata/pidgin_words.txt
       ------------------------
       0.500000
       buk
       buksop
       ------------------------
       1.000000
       aisblok


       ./genEditDist  -b 5  -s  testdata/transformations.txt shop testdata/pidgin_words.txt
       ------------------------
       0.600000
       buksop
       sop
       woksop
       ------------------------
       1.000000
       hop
       ------------------------
       1.200000
       sup
       sausap



 2.4. Using blocked regions in the search string
-----------------------------------------------------

     Blocked regions can be used to constrain the search and allow 
    transformations only in certain parts of the search string;

     When flag -e is set, substrings surrounded with either () and <> in
    the search string will be blocked from changes.

     () will block only regular edit distance changes, but allows changes 
        through generalized edit distance transformations.

     <> will block both regular edit distance and generalized edit distance,
        so the substring region cannot be changed at all.

     (( or << at the beginning of the search string block additions before 
    the first letter of the string (otherwise, such additions are allowed).
    Analogously, additions after the last letter are blocked by )) or >>.

     Currently, additions between two consecutive regions are also blocked,
    so, in case of search string '<a><b>c', no addition is allowed between
    'a' and 'b'.


       Usage examples (**):

       ./genEditDist  -m 3.0  -s -e  testdata/transformations.txt "<m>e<t>re" testdata/pidgin_words.txt
       ------------------------
       sentimita
       2.700000
       ------------------------
       kilomita
       2.700000
       ------------------------
       milimita
       2.700000
       ------------------------
       semitrela
       2.800000
       ------------------------
       tomato
       2.900000

       Without using the blocked regions, the previous query (suffix matches 
      with the word "metre") gives 32 matches;

       NB! If the "TOP N matches" search mode is used (flag "-b"), current 
      implementation does not quarantee, that matches with changes in
      blocked regions will be left out. This is because a change in a 
      blocked region has a concrete cost - 3000.0.
       
       For example, if we search for string '<<h>u<t>' in the dictionary 
      'testdata/pidgin_words.txt' using the transformations from the file
      'testdata/transformations.txt', only 2 matches can be reached without 
      changing blocked regions: "hat" and "het".
      So, if user queries for more than 2 closest matches, he/she will get 
      also matches with changes in the blocked regions:

       
       ./genEditDist  -b 3  -s -e  testdata/transformations.txt "<<h>u<t>" testdata/pidgin_words.txt
       ------------------------
       0.700000
       hat
       ------------------------
       1.000000
       het
       ------------------------
       3001.000000
       i gat samting nogut
       huk


 2.5. Showing transformations / alignments
-----------------------------------------------------

     If the plain full extent match mode is used for finding all  matches  within 
    the given maximum distance, i.e. the combination of flags -m and -f  is  used,
    and none of the flags -p, -s, -i, -e  is  set,  then  the  flag  -a  can  be
    used to switch on the mode of showing the transformations / alignments between 
    the search string and each found match.
    
       Usage examples (**):
       
       ./genEditDist -m 1.5 -f -a -y  testdata/transformations.txt  shop  testdata/pidgin_words.txt
       ------------------------
       hop
       1.000000
       s:h:o:p:
        :h:o:p:
       ;
       ------------------------
       sop
       0.600000
       sh:o:p:
        s:o:p:
       ;
       ------------------------
       sup
       1.200000
       sh:o:p:
        s:u:p:
       ;

     Note  that  the  alignments  are  displayed  not   character-wise,  but 
    transformation-wise,   considering   both   the   regular   edit  distance 
    transformations (single character transformations)  and  generalized  edit
    distance transformations (which can also be string to string transformations).
     The character ':' separates different positions in the alignment.
    
     If the optional flag  -y  is used in the mode of showing the transformations
    (like in the previous example), then alignments are output in a pretty-printing 
    mode: if two aligned positions contain different length strings, the shorter
    string is padded with spaces from the left side.
    
     There can be more than one best alignment between the two strings, in that 
    case,  a semicolon in a new line separates different alignments:
    
       ./genEditDist -m 3.5 -f -ay  testdata/transformations.txt shopper testdata/pidgin_words.txt
       ------------------------
       sops
       3.500000
       sh:o:p:p:e:r:
        s:o:p:s: : :
       ;
       sh:o:p:p:e:r:
        s:o:p: :s: :
       ;
       sh:o:p:p:e:r:
        s:o: :p:s: :
       ;
       ------------------------
       sop
       3.500000
       sh:o:p:p:e:r:
        s:o:p: : : :
       ;
       sh:o:p:p:e:r:
        s:o: :p: : :
       ;
    
    
     The optional flag -w can be used to show the weights / costs of the transformations. 
    The weights are displayed in-between the two aligned strings,  and  colons  are  used 
    as separators of different weights:
    
       ./genEditDist -m 1.5 -f -ay -w testdata/transformations.txt shop testdata/pidgin_words.txt
       ------------------------
       hop
       1.000000
       s:h:o:p:
       1.000000:0.000000:0.000000:0.000000:
        :h:o:p:
       ;
       ------------------------
       sop
       0.600000
       sh:o:p:
       0.600000:0.000000:0.000000:
        s:o:p:
       ;
       ------------------------
       sup
       1.200000
       sh:o:p:
       0.600000:0.600000:0.000000:
        s:u:p:
       ;


     NB! The current implementation of showing transformations does not support
    partial matches (flags -p, -s, -i), finding Top N matches (flag -b) and 
    using the blocked regions within the search string (flag -e).



 3. Compiling the program
---------------------------

  In order to compile the tool, GNU C Compiler (gcc) is needed. If the GNU 
  'make' utility is available, automatic compiling can be done with command:
  > make all

  NB! When compiling on a Solaris machine, one should make sure that GNU make
  is used instead of Sun's make. Usually, this can be done by calling GNU make
  with full path, for example /usr/sfw/bin/gmake .
  


 4.  (**) About the examples
-------------------------------

   The dictionary "testdata/pidgin_words.txt" contains words and phrases from 
  a Pidgin English and the dictionary "testdata/english_words.txt" contains
  corresponding English matches. The data is based on an online Pidgin/English 
  Dictionary (Pidgin as spoken in Port Moresby, Papua New Guinea) which can be 
  accessed on:
      http://www.june29.com/hlp/lang/pidgin.html



 5.  Reference
-------------------------------

   Orasmaa, S., Käärik, R., Vilo, J., & Hennoste, T. (2010). Information Retrieval of 
   Word Form Variants in Spoken Language Corpora Using Generalized Edit Distance. 
   In LREC.
