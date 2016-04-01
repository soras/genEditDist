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

#include "Trie.h"

// create new TrieNode
TrieNode *newTrieNode(wchar_t a){
	TrieNode *newNode;
	newNode = (TrieNode*)malloc(sizeof(TrieNode));
	if(newNode == NULL)
		abort();
	newNode->label = a;
	newNode->replacement = NULL;
	newNode->nextNode  = NULL;
	newNode->prevNode  = NULL;
	newNode->rightNode = NULL;
	return newNode;
}

// create new Trie
Trie *createTrie(){
    Trie *trieRoot;
    trieRoot = (Trie*)malloc(sizeof(Trie));
    if(trieRoot == NULL)
        abort();
    trieRoot->firstNode = NULL;
    return trieRoot;
}

// Create new EndNode
EndNode *newEndNode(wchar_t *repl, double weight){
    EndNode *endNode;
    endNode = (EndNode*)malloc(sizeof(EndNode));
    if(endNode == NULL)
        abort();
    endNode->edit = (wchar_t*)copy_wchar_t(repl, wchar_len(repl));
    endNode->value = weight;
    endNode->nextEN = NULL;
    return endNode;
}

// Add new replacement to the TrieNode
int addEndNode(TrieNode *node, wchar_t *string, double value){
    EndNode *endN;
    int i;
    if(node->replacement == NULL){
        node->replacement = newEndNode(string, value);
        return 0;
    }

    endN = node->replacement;
    while(1){
        i = 0;
        while(1){
            if(string[i] == endN->edit[i]){
                 /*  If replacement strings are matching, we will only
                 *  preserve the lowest weight, as preserving the higher
                 *  weight is unnecessary: while calculating edit distance,
                 *  higher weight will be left out anyway.
                 */ 
                 if(string[i] == L'\0'){
                     if(endN->value > value)
                         endN->value = value;
                     return 0;
                 }
                 /* not in end yet, compare the next letter */
                 i++;
            }
            // letters did not match
            else break;
        }
        if(endN->nextEN == NULL) break;
        else endN = endN->nextEN;
    }
    endN->nextEN = newEndNode(string, value);
    return 0;
}

// Adds string1 to trie, while expanding only downwards (in depth) via nextNode links
int addToTrieDepth(TrieNode *node, wchar_t *string1, int strLen1, wchar_t *string2, double value){
    TrieNode *tmp;
    /* more than 1 letters to add */
    if(strLen1 > 1){
        node->nextNode = newTrieNode(*string1);
        node->nextNode->prevNode = node;
        return addToTrieDepth(node->nextNode, string1+1, strLen1-1, string2, value);
    }
    /* only 1 letter to add */
    else{
        tmp = newTrieNode(*string1);
        tmp->prevNode  = node;
        node->nextNode = tmp;
        tmp->replacement = newEndNode(string2, value);
    }
    return 0;
}

// Adds transformation (string1 => string2 : value) to the trie t
int addToTrie(Trie *t, wchar_t *string1, int strLen1, wchar_t *string2, double value){
    TrieNode *tmp;
    /* if the trie is empty ... */
    if(t->firstNode == NULL){
        t->firstNode = newTrieNode(*string1);
        tmp = t->firstNode;
        if((strLen1-1) == 0)
            tmp->replacement = newEndNode(string2, value);
        else
            addToTrieDepth(tmp, string1+1, strLen1-1, string2, value);
        return 0;
    }

    tmp = t->firstNode;
    TrieNode *prev;
    prev = NULL;
    /* find common prefix from the trie */
    while(strLen1 > 0){
        /* if the left side already exists, add replacement to its ending node */
        if(strLen1 == 1 && tmp->label == *string1){
           addEndNode(tmp,string2, value);
           return 0;
        }
        /* if the label matches, proceed to the next level in depth */
        else if(tmp->label == *string1 && tmp->nextNode != NULL){
           prev = tmp;
           tmp = tmp->nextNode;
           string1 = string1 + 1;
           strLen1--;
        }
        /* if we are at the end of known prefix, expand */
        else if(tmp->label == *string1 && tmp->nextNode == NULL){
           addToTrieDepth(tmp, string1+1,strLen1-1, string2, value);
           return 0;
        }
        /* if labels did not match, check the next node in the same depth level ...*/
        else if(tmp->rightNode != NULL)
            tmp = tmp->rightNode;
        else break;
    }
    /* if common prefix was not found or was incomplete, a new branch must be started */
    tmp->rightNode = newTrieNode(*string1);
    tmp->rightNode->prevNode = prev;
    tmp = tmp->rightNode;
    if(strLen1 == 1)
       addEndNode(tmp,string2, value);
    else
       addToTrieDepth(tmp, (string1+1),strLen1-1, string2, value);
    return 0;
}

// Releases memory under endnode list
void freeEndNode(EndNode *endNode){
    EndNode *tmp;
    tmp = endNode;
    while (tmp != NULL){
        EndNode *next;
        next = tmp->nextEN;
        if (tmp->edit != NULL){
            free(tmp->edit);
        }
        free(tmp);
        tmp = next;
    }
}

// Releases memory under trie node and all of its children & siblings
void freeTrieNode(TrieNode *node){
   TrieNode *tmp;
   tmp = node;
   while (tmp != NULL){
       if (tmp->replacement != NULL){
           freeEndNode(tmp->replacement);
       }
       if (tmp->rightNode != NULL){
           freeTrieNode(tmp->rightNode);
       }
       TrieNode *next;
       next = tmp->nextNode;
       free(tmp);
       tmp = next;
   }
}

// Releases memory under trie and all of its children
void freeTrie(Trie *trie){
   if (trie->firstNode != NULL){
       freeTrieNode(trie->firstNode);
   }
   free(trie);
}
