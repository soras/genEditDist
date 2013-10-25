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

#include "List.h"

// makes a copy of given string
wchar_t *copy_wchar_t(wchar_t *str, int strLen){
    wchar_t *res;
    int i;

    // allocate memory and copy
    if((res = (wchar_t *)malloc(sizeof(wchar_t)*(strLen+1))) == NULL){
        puts("Error: Could not allocate memory");
        exit(1);
    }

    res[strLen] = L'\0';
    for(i = 0; i < strLen; i++)
        res[i] = str[i];

    return res;
}

// measures length of given string
int wchar_len(wchar_t *str){
    int len;

    len = 0;
    while(str[len] != L'\0')
        len++;

    return len;
}

// creates new list
List *createList(){
  List *list;
  list = (List*)malloc(sizeof(List));
  if(list == NULL)
    abort();
  list->firstItem = NULL;
  return list;
}

// creates new index
Index *createIndex(int i, int j){
    Index *index;

    index = (Index*)malloc(sizeof(Index));
    if(index == NULL)
        abort();

    index->i = i;
    index->j = j;
    index->nextIndex = NULL;
    return index;
}

// creates new list item
ListItem *createListItem(double v, int i, int j){
	ListItem *item;
	item = (ListItem *)malloc(sizeof(ListItem));
	if(item == NULL)
		abort();

	item->value = v;
  item->index = createIndex(i, j);
	item->nextItem = NULL;
	return item;
}

// creates new upper-case to lower-case transformation
IgnoreCaseListElement *createIgnoreCaseElement(wchar_t *l, wchar_t *r){
    IgnoreCaseListElement *e;

    e = (IgnoreCaseListElement *)malloc(sizeof(IgnoreCaseListElement));
    if(e == NULL)
       abort();
    e->left = (wchar_t *)copy_wchar_t(l, wchar_len(l));
    e->right = (wchar_t *)copy_wchar_t(r, wchar_len(r));
    return e;
}

// removes last element, the first element will never be removed
int removeLastItem(ListItem *li){
    ListItem *nextItem;
    Index *index;
    Index *nextI;
    int find = 1;

    while( find && li->nextItem != NULL ){
        if(li->nextItem->nextItem == NULL){ // this is the item that should be removed
            nextItem = li->nextItem;
            li->nextItem = NULL;
            lastBest = li->value;

            index = nextItem->index;
            nextItem->index = NULL;
            while(index != NULL){
                nextI = index->nextIndex;
                index->nextIndex = NULL;
                free(index);
                index = nextI;
            }
            free(nextItem);
            return 0;
        }
        li = li->nextItem;
    }
    return 0;
}

// Adds position indexes to given list item
int insertValues(ListItem * li, int i, int j){
    Index *index;
    index = li->index;

    while(index->nextIndex != NULL){
        index = index->nextIndex;
    }
    index->nextIndex = createIndex(i, j);
    return 0;
}


// insert new item into the list
int insertListItem(List *list, double value, int i, int j, int isFull, int inserted){
    ListItem *item;
    ListItem *nextItem;
    ListItem *newFirst;
    int searchPlace = 1;
    double v;

    item = list->firstItem;

    // the list is empty
    if(item == NULL){
        list->firstItem = createListItem(value, i, j);
        lastBest = value;
        inserted--;
        return inserted;
    }

    // this listItem should be the first?
    if(value < item->value){
        newFirst = createListItem(value, i, j);
        newFirst->nextItem = item;
        list->firstItem = newFirst;
         if(isFull){
            removeLastItem(list->firstItem);
        }
        inserted--;
        return inserted;
    }

    /* search the right place */
    while(searchPlace){
        // insert in the same list item
        v = item->value;

        if(item->value == value){
             insertValues(item, i, j);
             return inserted;
        }
        nextItem = item->nextItem;

        // there is no next item
        if(nextItem == NULL){
            item->nextItem = createListItem(value, i, j);
            lastBest = value;
            inserted--;
            return inserted;
        }
        // should we add a new list item between two existing items?
        if(nextItem->value > value){
            item->nextItem = createListItem(value, i, j);
            item->nextItem->nextItem = nextItem;
            if(isFull){
                removeLastItem(list->firstItem);
            }
            inserted--;
            return inserted;
        }
        item = item->nextItem;
    }
    return 0;
}

// Releases memory under the list
void freeList(List *list){
    ListItem *item;
    item = list->firstItem;

    // until the list is empty
    while(item != NULL){
        Index *index;
        index = item->index;
        while(index != NULL){
            Index *nextIndex;
            nextIndex = index->nextIndex;
            free(index);
            index = nextIndex;
        }
        ListItem *nextItem;
        nextItem = item->nextItem;
        free(item);
        item = nextItem;
    }
    free(list);
}

// adds new upper-case to lower-case transformation into the ignore case list
int insertIgnoreCaseElement(wchar_t *l, wchar_t *r){
    IgnoreCaseListElement *current;

    // add as first element
    if(ignoreCase == NULL){
        if(debug)
            puts("Inserting first element to ignore case list");
        ignoreCase = createIgnoreCaseElement(l, r);
        return 0;
    }
    // first element already exists, add to the end
    else{
        current = ignoreCase;
        while(current->next != NULL)
            current = current->next;

        // current->next == NULL
        if(debug)
            puts("Inserted element to ignore case list");
        current->next = createIgnoreCaseElement(l, r);
        return 0;
    }
}

// frees memory under the list ignoreCase
void freeIgnoreCaseList(){
   IgnoreCaseListElement *current;
   current = ignoreCase;
   while (current != NULL){
       IgnoreCaseListElement *next;
       next = current -> next;
       if (current->left != NULL){
          free(current->left);
       }
       if (current->right != NULL){
          free(current->right);
       }
       free(current);
       current = next;
   }
   ignoreCase = NULL;
}

// prints given list
int printList(List *list){
    ListItem *item;
    Index *index;

    item = list->firstItem;
    while(item != NULL){
        printf("\n %f \n", item->value);
        index = item->index;
        while(index != NULL){
            printf("     i: %i  j: %i\n", index->i, index->j);
            index = index->nextIndex;
        }
        puts("------------------------\n");
        item = item->nextItem;
    }
    return 0;
}
