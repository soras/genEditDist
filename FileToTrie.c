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

#include "FileToTrie.h"

// Read file into memory
char *readFile(char *filename){
    int fd;
    char *data;
    struct stat sbuf;

    if (filename == NULL) {
        fprintf(stderr, "Insert filename!\n");
        exit(1);
    }

    if ((fd = open(filename, O_RDONLY)) == -1) {
        perror("Error on opening file");
        exit(1);
    }

    if (stat(filename, &sbuf) == -1) {
        perror("Error on receiving stat");
        exit(1);
    }

    if ((data = mmap((caddr_t)0, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0)) == (caddr_t)(-1)) {
        perror("Could not map file");
        exit(1);
    }

    if (close(fd) == -1){
        perror("Error on closing file");
        exit(1);
    }
    return data;
}

// Read a snippet from data and convert into double
double findValue(char *data, int i, int j){
	char *value;
	char *err;
	double rep;

	value = malloc(j-i+1);
	if(value == NULL){
		perror("Memory");
		exit(1);
	}
	value[j-i] = '\0';
	strncpy(value, (data +i), (j-i));
	rep = strtod(value, &err);
	free(value);
	return rep;
}

// Builds add, rep and rem tries from given content of transformations file
int trieFromFile(char *data){
	char *string1;
	char *string2;
	wchar_t *wstr1;
	wchar_t *wstr2;
	int w1;
	int w2;
	double v;
	int datalen;
	int i;
	int j;

	datalen = strlen(data);
	string1 = malloc(2);
	string2 = malloc(2);

	i = 0;
	j = 0;

	while(i < datalen){
    /*
    *   Setting weights of default edit operations:
    *   the line must begin with character '>', followed
    *   by "add" for addition, "rep" for replacement and
    *   "del" for deletion operation; after the operation
    *    marker, ':' is placed and finally comes the new
    *    weight as double.
    */
		if(data[i] == '>'){
			i = i +1;
      /* Change weight of default add operation */
			if(strncmp((char *)(data+i), "add", 3) == 0){
				i +=4;
				j = i;
				while(data[j] != '\n' && data[j] != '\r' && j < strlen(data))
					j++;
				add = findValue(data, i, j);
				if(data[j] == '\r') /* In case we are under Windows */
					j = j + 2;
				else j = j+1;
				i = j;
			}
			/* Change weight of default replace operation */
			else if(strncmp((char *)(data+i), "rep", 3) == 0){
				i +=4;
				j = i;
				while(j < strlen(data) && data[j] != '\n' && data[j] != '\r') j++;
				rep = findValue(data, i, j);
				if(data[j] == '\r') /* In case we are under Windows */
					j = j + 2;
				else j = j+1;
				i = j;
			}
      /* Change weight of default remove operation */
			else if(strncmp((char *)(data+i), "rem", 3) == 0){
				i +=4;
				j = i;
				while(j < strlen(data) && data[j] != '\n' && data[j] != '\r') j++;
				rem = findValue(data, i, j);
				if(data[j] == '\r') /* In case we are under Windows */
					j = j + 2;
				else j = j+1;
				i = j;
			}
		}
    /*
    *  Setting weights of generalized edit distance transformations; 
    */
		else{
			j = i;
			/* Locate the left side string of transformation */
			while(data[j] != ':') j++;
			string1 = (char *)realloc(string1, (j-i+1));

			if(string1 == NULL){
				perror("Memory");
				exit(1);
			}
			string1[j-i] ='\0';
			strncpy(string1, (data +i), (j-i));


			/* Locate the right side string of transformation */
			j++; i = j;
			while(data[j] != ':') j++;
			string2 = (char *)realloc(NULL, (j-i+1));

			if(string2 == NULL){
				perror("Memory");
				exit(1);
			}
			string2[(j-i)]='\0';
			strncpy(string2, (data +i), (j-i));
			j++; i = j;
			while(data[j] != '\n' && data[j] != '\r' && j < datalen) j++;

      /* Find the weight of transformation  */
			v =  findValue(data, i, j);

       /* According to the type of transformation, add into suitable trie */
			if(strlen(string1) == 0){
          /* Add to add-operations trie */
          w2 = mbstowcs(NULL, string2, 0);
          wstr2 = (wchar_t *)localeToWchar(string2);
          if(caseInsensitiveMode)
              wstr2 = makeStringToIgnoreCase(wstr2, w2);
          addToARTrie(addT, wstr2, w2, v);
          free(wstr2);
          free(string2);
			}else if(strlen(string2) == 0){
          /* Add to remove-operations trie */
          w1 = mbstowcs(NULL, string1, 0);
          wstr1 = (wchar_t *)localeToWchar(string1);
          if(caseInsensitiveMode)
              wstr1 = makeStringToIgnoreCase(wstr1, w1);
          addToARTrie(remT, wstr1, w1, v);
			}else{
          /* Add to replace-operations trie */
          wstr2 = (wchar_t *)localeToWchar(string2);
          wstr1 = (wchar_t *)localeToWchar(string1);
          w1 = mbstowcs(NULL, string1, 0);
          w2 = mbstowcs(NULL, string2, 0);
          if(caseInsensitiveMode){
              wstr1 = makeStringToIgnoreCase(wstr1, w1);
              wstr2 = makeStringToIgnoreCase(wstr2, w2);
          }
          addToTrie(wstr1, w1, wstr2,v);
          free(string2);
          free(wstr1);
			}
      if(data[j] == '\r') /* In case we are under Windows */
          j = j + 2;
      else 
          j = j+1;
      i = j;
		}
	}
	free(string1);
	/* Release the memory under data. */
	munmap(data, strlen(data));
	return 0;
}


// Builds ignore case list according to given file content
int ignoreCaseListFromFile(char *data){
	char *string1;
	char *string2;
	wchar_t *wstr1;
	wchar_t *wstr2;

	int datalen;
	int i;
	int j;

	datalen = strlen(data);
	i = 0;
	j = 0;

	while(i < datalen){
        j = i;

        /* Find left side of the transformation */
        while(data[j] != ':') j++;

        string1 = (char *)malloc(j-i+1);

        if(string1 == NULL){
           perror("Memory");
           exit(1);
        }
        string1[j-i] ='\0';
        strncpy(string1, (data +i), (j-i));

        /* Find right side of the transformation */
        j++; i = j;

        while(data[j] != '\n' && data[j] != '\r' && j < datalen) j++;

        string2 = (char *)malloc(j-i+1);

        if(string2 == NULL){
           perror("Memory");
           exit(1);
        }
        string2[(j-i)]='\0';
        strncpy(string2, (data +i), (j-i));

        /* Transform into wide-char string */
        wstr2 = (wchar_t *)localeToWchar(string2);
        wstr1 = (wchar_t *)localeToWchar(string1);

        insertIgnoreCaseElement(wstr1, wstr2);
        free(string1);
        free(string2);
        free(wstr1);
        free(wstr2);

        if(data[j] == '\r') /* In case we are under Windows */
           j = j + 2;
        else j = j+1;
           i = j;
  }
  /* Release the file content */
  munmap(data, strlen(data));
  return 0;
}

// Normalizes a case of a single character
wchar_t makeToIgnoreCase(wchar_t s){
    IgnoreCaseListElement *caseList;

    caseList = ignoreCase;
    if(caseList == NULL){
        if(debug)
            puts("Ignore Case list was empty!");
        return s;
    }
    while(caseList != NULL){
        if(s == caseList->left[0]){ // selle t‰he case saame ‰ra muuta
            return caseList->right[0];
        }
        caseList = caseList->next;
    }
    return s;
}

// Normalizes case of a string
wchar_t *makeStringToIgnoreCase(wchar_t *string, int len){
    int i;
    // Attempts to transform every letter/character of given string 
    for(i = 0; i < len; i++){
        string[i] = makeToIgnoreCase(string[i]);
    }
    return string;
}

// Transforms wchar into multibyte char
char *wcharToLocale(wchar_t *str){
	char * ptr;
	size_t s;

	/* calculate needed space */
	s = wcstombs(NULL, str, 0);

	/* there are characters that could not
	   be converted to current locale */
	if(s == -1){
		puts("Error: Could not convert some characters to multibyte");
		exit(1);
	}

	/* malloc the necessary space */
	if((ptr = (char *)malloc(s + 1)) == NULL){
        puts("Error: Could not allocate memory");
		exit(1);
	}
	/* really do it */
	wcstombs(ptr, str, s);

	/* ensure NULL-termination */
	ptr[s] = '\0';

	return(ptr);
}

// Transforms multibyte char into wchar string
wchar_t *localeToWchar(char *str){
	  wchar_t * ptr;
	  size_t s;
  
	  /* calculate needed space*/
	  s = mbstowcs(NULL, str, 0);
  
	  /* is there  an error in encoding? */
	  if(s == -1){
		  puts("Error: could not convert to wchar");
		  exit(1);
	  }
	  /* malloc the necessary space */
	  if((ptr = (wchar_t *)malloc((s + 1) * sizeof(wchar_t))) == NULL){
          puts("Error: Could not allocate memory");
		  exit(1);
	  }
	  /* really do it */
	  mbstowcs(ptr, str, s);
  
	  /* ensure NULL-termination */
	  ptr[s] = L'\0';
  
	  return(ptr);
}

