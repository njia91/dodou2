/*
  Author:   Michael Andersson
  CS:       mian
  
  Purpose:  A doublelinked list with an memoryhandler.

  Description: This doublelinked list can be used to store data.
               The data that must be in the form of a pointer.
               When the doublelinked list(dll) is created the user
               can choose to set the memoryhandler. The memoryhandler
               will free all the memory related to the list, including
               the data. If memoryhandler is not set the data stored in 
               the list will not be freed.
*/
#include "list.h"  

/*
  Creates a dubblelinked list.
  Return: Pointer to the newly created doublelinked list if not
  able to allocate memory return NULL.
*/

dll *dll_empty(void){
  dll *l = calloc(1, sizeof(dll));
  if (l != NULL){
    cell *header = calloc(1, sizeof(cell));
    l->header = header;
    l->length = 0;
    l->freeFunc = NULL;
    header->next = header;
    header->previous = header;
    header->value = NULL;
  }
  return l;
}
/*
  Sets a memoryhandler for the dll and it will have responsibility
  to deallocate memory for elements that no longer presists in the dll.

  Parameters: l - a directed list
              f - functions pointer a function that deallocates memory.
                  This function does only have one pointer of type (void*).
*/
void dll_setMemoryHandler(dll *l, memFreeFunc *f){
  l->freeFunc = f;
}

/*
  Adds an element before the given position in the list with the given
  value.
  Parameters: *value - pointer to the value
              p - were to insert new element.
              *l - pointer to dll.
  Returns: The position of the new element as an integer.
*/
list_position dll_insert(data value, list_position p, dll *l){
  list_position pos = malloc(sizeof(cell));
  pos->value = value;
  pos->next = p;
  pos->previous = p->previous;
  p->previous->next = pos;
  p->previous = pos; 
  l->length++;
  return pos;
}

/*
  Checks if the given list if empty.
  Parameters: l - pointer to a doublelinked list.
  Returns : True if empty, false otherwise.
*/
bool dll_isEmpty(dll *l){
  return !(l->length);
}

/*
 Returns the length of the list
 Parameters: l - pointer to a doublelinked list.
 Return: An integer of the current list length
*/
int dll_getLength(dll *l){
  return l->length;
}
/*
  Inspects and returns the value on the given position in the list.
  Parameters: *p - pointer to the position
              *l - pointer to the list
  Returns: data - pointer to the curernt value on the given position.
                  Returns null if p == header.
                  Undefined for empty list!
*/
data dll_inspect(list_position p, dll *l){
  if (p != l->header){
   return p->value;
  }
  return NULL;
}
/*
  Returns the position for the first element for the given list.
  Parameters: l - pointer to a doublelinked list.
  Return: The position of the first element in the list.
  NOTE: undefined if list is empty!
*/
list_position dll_first(dll *l){
  return l->header->next;
}
/*
  Returns the position for the last element for the given list.
  Parameters: l - pointer to a doublelinked list.
  Return: The position of the last element in the list.
  NOTE: undefined if list is empty!
*/
list_position dll_end(dll *l){
  return l->header->previous;
}
/*
  This function returns a pointer to the next element in 
  the doublelinked list.
  Parameters: p - pointer a position in the list
              l - pointer to the doublelinked list
  Return: Pointer to the next position in the given list 
          but will return header if on last element

  NOTE: Undefined if list is empty
*/
list_position dll_next(list_position p, dll *l){
    if (p != dll_end(l)){
      return p->next;
    }
    return (list_position)l->header;
}
/*
  This function returns a pointer to the previous 
  element in the doublelinked list.
  Parameters: p - pointer a position in the list
              l - pointer to the doublelinked list
  Return: Pointer to the previous position in the given list 
          but will return header if on first element.
*/
list_position dll_previous(list_position p, dll *l){
  if (p != dll_first(l)){
    return p->previous;
  }
  return (list_position)l->header;
}
/*
  Removes the element on the given position and returns the position for the
  following element.
  Parameters:   *p - pointer to given position in the doublelinked list
                *l - pointer to given list
  Returns: The position to the element following the removed element.
           Retuns NULL if list is empty and if list becomes empty after
           removing an element.
  NOTE: Undefined for an empty list!
*/
list_position dll_remove(list_position p, dll *l){
  list_position returnPos = p->next;
    dll_previous(p,l)->next = p->next;
    dll_next(p,l)->previous = p->previous; 

  
  if (l->freeFunc != NULL){
    l->freeFunc(p->value);
  }
  
  free(p);
  l->length--;
  if(returnPos == l->header){
    return l->header->previous; 
  }
  return returnPos;
}

/*
  Removes the list and deallocates all memory.
  Parameters :  l -  A pointer to the given doublelinked list.
  NOTE: The values in the list is only deallocated if the memoryhandler is set.
*/
void dll_free(dll *l){
  list_position pos;
  while(!dll_isEmpty(l)){
    pos = dll_first(l);
    dll_remove(pos,l);
  }
  free(l->header);
  free(l);
}
