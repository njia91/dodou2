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

#ifndef __LIST__
#define __LIST__
#include <stdbool.h>
#include <stdlib.h>     

#ifndef __DATA__
#define __DATA__
typedef void *data;
#endif

#ifndef __FREEFUNC__
#define __FREEFUNC__
typedef void memFreeFunc(data);
#endif

//Struct an element in the dll
typedef struct cell {
  struct cell *previous;
  data value;
  struct cell *next;
} cell;

typedef cell *list_position;

//Struct for doublelinked list(dll)
typedef struct{
  cell *header;
  memFreeFunc *freeFunc;
  int length;
} dll;

/*
  Creates a dubblelinked list.
  Return: Pointer to the newly created doublelinked list.
*/
dll *dll_empty(void);

/*
  Sets a memoryhandler for the dll and it will have responsibility
  to deallocate memory for elements that no longer presists in the dll.

  Parameters: l - a directed list
              f - functions pointer a function that deallocates memory.
                  This function does only have one pointer of type (void*).
*/
void dll_setMemoryHandler(dll *l, memFreeFunc *f);

/*
  Adds an element before the given position in the list with the given
  value.
  Parameters: *value - pointer to the value
              p - were to insert new element.
              *l - pointer to dll.
  Returns: The position of the new element as an integer.
*/
list_position dll_insert(data value, list_position p, dll *l);

/*
  Checks if the given list if empty.
  Parameters: l - pointer to a doublelinked list.
  Returns : True if empty, false otherwise.
*/
bool dll_isEmpty(dll *l);

/*
 Returns the length of the list
 Parameters: l - pointer to a doublelinked list.
 Return: An integer of the current list length
*/
int dll_getLength(dll *l);


/*
  Inspects and returns the value on the given position in the list.
  Parameters: *p - pointer to the position
              *l - pointer to the list
  Returns: data - pointer to the curernt value on the given position.
*/
data dll_inspect(list_position p, dll *l);

/*
  Returns the position for the first element for the given list.
  Parameters: l - pointer to a doublelinked list.
  Return: The position of the first element in the list.
  NOTE: undefined if list is empty!
*/
list_position dll_first(dll *l);

/*
  Returns the position for the last element for the given list.
  Parameters: l - pointer to a doublelinked list.
  Return: The position of the last element in the list.
  NOTE: undefined if list is empty!
*/
list_position dll_end(dll *l);


/*
  This function returns a pointer to the next element in 
  the doublelinked list.
  Parameters: p - pointer a position in the list
              l - pointer to the doublelinked list
  Return: Pointer to the next position in the given list 
          but will return NULL if on first element

  NOTE: Undefined if list is empty
*/
list_position dll_next(list_position p, dll *l);

/*
  This function returns a pointer to the previous 
  element in the doublelinked list.
  Parameters: p - pointer a position in the list
              l - pointer to the doublelinked list
  Return: Pointer to the previous position in the given list 
          but will return NULL if on first element

  NOTE: Undefined if list is empty

*/
list_position dll_previous(list_position p, dll *l);

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
list_position dll_remove(list_position p, dll *l);


/*
  Removes the list and deallocates all memory.
  Parameters :  l -  A pointer to the given doublelinked list.
  NOTE: The values in the list is only deallocated if the memoryhandler is set.
*/
void dll_free(dll *l);

#endif
