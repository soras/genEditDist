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

#include "ARTrie.h"

/*  Creates new ARTrie  */
ARTrie *createARTrie(){
	ARTrie *root;
	root = (ARTrie*)malloc(sizeof(ARTrie));
	if(root == NULL)
		abort();
	root->firstNode = NULL;
	return root;
}

/*  Creates new ARTNode  */
ARTNode *newARTNode(wchar_t a){
	ARTNode *artN;
	artN = (ARTNode *)malloc(sizeof(ARTNode));
	if(artN == NULL)
		abort();
	artN->label = a;
	artN->value = DBL_MAX;
	artN->rightNode = NULL;
	artN->nextNode  = NULL;
	artN->prevNode  = NULL;
	return artN;
}

/*  Adds an add/remove transformation with given weight into the trie */
int addToARTrie(ARTrie *art, wchar_t *string, int strLen , double value){
	ARTNode *tmp;

	/* If trie is still empty ... */
	if(art->firstNode == NULL){
		art->firstNode = newARTNode(*string);
		tmp = art->firstNode;
		string = string + 1;
		strLen--;
		if(strLen == 0){
			tmp->value = value;
			return 0;
		}
		while(strLen > 1){
			tmp->nextNode = (ARTNode *)newARTNode(*string);
			tmp->nextNode->prevNode = tmp;
			tmp = tmp->nextNode;
			string = string +1;
			strLen--;
		}
		tmp->nextNode = (ARTNode *)newARTNode(*string);
		tmp->nextNode->value = value;
		tmp->nextNode->prevNode = tmp;
		return 0;
	}

  tmp = art->firstNode;
  ARTNode *prev;
  prev = NULL;
  while(strLen > 0){
       /* We are trying to follow an existing path in the trie ... */
       /* ... reached end of the path and end of our string */
       if(strLen == 1 && tmp->label == *string && value < tmp->value){
           /* We want that value of the node is always as small as it can be, as
           * the generalized edit distance also uses only smallest values. Here
           * we replace an existing value with smaller one.
           */
           tmp->value = value;
           return 0;
       }
       /* ... path continues */
       else if(tmp->label == *string && tmp->nextNode != NULL){
           prev = tmp;
           tmp = tmp->nextNode;
           string = string + 1;
           strLen--;
       }
       /* ... reached end of the path, but our string does not end */
       else if(tmp->label == *string && tmp->nextNode == NULL){
           string = string +1;
           strLen--;
           while(strLen > 1){
              tmp->nextNode = (ARTNode *)newARTNode(*string);
              tmp->nextNode->prevNode = tmp;
              tmp = tmp->nextNode;
              string = string +1;
              strLen--;
           }
           tmp = tmp->nextNode = (ARTNode *)newARTNode(*string);
           tmp->nextNode->prevNode = tmp;
           tmp->value = value;
           return 0;
     }
     /* ... if labels do not match, try to switch the track (look next nodes
            in the same level) ... */
     else if(tmp->rightNode != NULL)
           tmp = tmp->rightNode;
     else break;
  }
  /* ... existing path not found, so it must be created */
  tmp->rightNode = newARTNode(*string);
  tmp = tmp->rightNode;
  tmp->prevNode = prev;
  if(strLen == 1)
      tmp->value = value;
  else{
      string = string +1;
      strLen--;
      while(strLen > 1){
        tmp->nextNode = (ARTNode *)newARTNode(*string);
        tmp->nextNode->prevNode = tmp;
        tmp = tmp->nextNode;
        string = string +1;
        strLen--;
      }
      tmp->nextNode = (ARTNode *)newARTNode(*string);
      tmp->nextNode->value = value;
      tmp->nextNode->prevNode = tmp;
  }
  return 0;
}

/* debug: print the trie */
int showARTrie(ARTrie *t){
    ARTNode *nodes[100]; /* NB! a predefined size, increase if necessary */
    ARTNode *tmp;
    ARTNode *current;
    int j = 0;
    int i = 1;
    int k =0;
    char *preC;
    size_t preCLen;

    for(k = 0; k < 100; k++){
       nodes[k] = NULL;
    }
    tmp = t->firstNode;
    nodes[0] = tmp;
    while(tmp->rightNode != NULL){
       nodes[i] = tmp->rightNode;
       i++;
       tmp = tmp->rightNode;
    }
    while(nodes[j] != NULL){
       current = nodes[j];
       preCLen = wctomb(NULL ,current->label);

       if((preC = (char *)malloc(preCLen + 1)) == NULL){
           puts("Error: Could not allocate memory");
           exit(1);
       }
       wctomb(preC ,current->label);
       preC[preCLen] = '\0';

       printf("\n %s", preC);
       if(current->value != DBL_MAX)
           printf(" , %f", current->value);

       /* could not go further down on the branch */
       if(current->nextNode == NULL){
           j++; continue;
       }
       else { /* can go further down on the branch*/
           nodes[i] = current->nextNode;
           i++;
           tmp = current->nextNode;
           while(tmp->rightNode != NULL){
              nodes[i] = tmp->rightNode;
              i++;
              tmp = tmp->rightNode;
           }
           j++;
       }
    }
    puts("\n");
    return 0;
}

// Releases memory under ARTNode and all of its children
void freeARTNode(ARTNode *node){
   ARTNode *tmp;
   tmp = node;
   while (tmp != NULL){
       if (tmp->rightNode != NULL){
           freeARTNode(tmp->rightNode);
       }
       ARTNode *next;
       next = tmp->nextNode;
       free(tmp);
       tmp = next;
   }
}

// Releases memory under ARTrie and all of its children
void freeARTrie(ARTrie *art){
   ARTNode *tmp;
   tmp = art->firstNode;
   if (tmp != NULL){
      freeARTNode(tmp); 
   }
   free(art);
}

