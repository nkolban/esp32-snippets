#include <stdlib.h>

#include "c_list.h"

/**
 * A list is an ordered set of entries.  We have the following primitives:
 * * list_create() - Create an empty list.  The return is the list pointer.
 * * list_delete() - Delete the list and optionally free all its entries.
 * * list_first() - Return the first item in the list.
 * * list_insert() - Add an item to the end of the list.
 * * list_insert_after() - Add an item to the list after a given entry.
 * * list_insert_before() - Add an item to the list before a given entry.
 * * list_next() - Get the next item in the list.
 * * list_remove() - Remove a specific item from the list.
 * * list_removeByValue() - Find the first element in the list with a matching value and remove it.
 */

/**
 * Create a new list.
 */
list_t *list_createList() {
	list_t *pList = malloc(sizeof(list_t));
	pList->next = NULL;
	pList->prev = NULL;
	pList->value = NULL;
	return pList;
} // list_createList

/**
 * Delete a list.
 */
void list_deleteList(list_t *pList, int withFree) {
	list_t *pNext;
	while(pList != NULL) {
		pNext = pList->next;
		if (withFree) {
			free(pList->value);
		}
		free(pList);
		pList = pNext;
	}
} // list_deleteList

/**
 * Insert a new item at the end of the list.
 *[A] -> [endOLD]    ------>   [A] -> [endOLD] -> [X]
 *
 */
void list_insert(list_t *pList, void *value) {
	while(pList->next != NULL) {
		pList = pList->next;
	}
	list_insert_after(pList, value);
} // list_insert

/**
 * [pEntry] -> [B]    ------>   [pEntry] -> [X] -> [B]
 *
 */
void list_insert_after(list_t *pEntry, void *value) {
	list_t *pNew = malloc(sizeof(list_t));
	pNew->next = pEntry->next;
	pNew->prev = pEntry;
	pNew->value = value;

	// Order IS important here.
	if (pEntry->next != NULL) {
		pEntry->next->prev = pNew;
	}
	pEntry->next = pNew;
} // list_insert_after

/**
 * [A] -> [pEntry]   ------>   [A] -> [X] -> [pEntry]
 *
 */
void list_insert_before(list_t *pEntry, void *value) {
	// Can't insert before the list itself.
	if (pEntry->prev == NULL) {
		return;
	}
	list_t *pNew = malloc(sizeof(list_t));
	pNew->next = pEntry;
	pNew->prev = pEntry->prev;
	pNew->value = value;

	// Order IS important here.
	pEntry->prev->next = pNew;
	pEntry->prev = pNew;
} // list_insert_before

/**
 * Remove an item from the list.
 */
void list_remove(list_t *pList, list_t *pEntry, int withFree) {
	while(pList != NULL && pList->next != pEntry) {
		pList = pList->next;
	}
	if (pList == NULL) {
		return;
	}
	pList->next = pEntry->next;
	if (pEntry->next != NULL) {
		pEntry->next->prev = pList;
	}
	if (withFree) {
		free(pEntry->value);
	}
	free(pEntry);
} // list_delete

/**
 * Delete a list entry by value.
 */
void list_removeByValue(list_t *pList, void *value, int withFree) {
	list_t *pNext = pList->next;
	while(pNext != NULL) {
		if (pNext->value == value) {
			list_delete(pList, pNext, withFree);
			return;
		}
	} // End while
} // list_deleteByValue

/**
 * Get the next item in a list.
 */
list_t *list_next(list_t *pList) {
	if (pList == NULL) {
		return NULL;
	}
	return (pList->next);
} // list_next


list_t *list_first(list_t *pList) {
	return pList->next;
} // list_first

void *list_get_value(list_t *pList) {
	return pList->value;
}

