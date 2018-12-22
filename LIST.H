
#ifndef _LIST_H_
#define _LIST_H_

#include LIBDIR"std.h"
#include LIBDIR"flags.h"

/*
	initialize all lists yourself
	set all the variables in the LIST struct to something, there is no NULL
		handling, should there be?
*/

struct LIST
{
	void *first; //first item in the entire list
	void *top;   //first visible item
	void *sel;   //currently selected item
	void *bot;   //last visible item
	void *last;  //last item in the entire list

	void *(*next)(void *);    //should return 'void *' to next item in list
	void **(*pnext)(void *);  //should return 'void **' to next item in list
	void *(*prev)(void *);    //should return 'void *' to previous item in list
	void **(*pprev)(void *);  //should return 'void **' to previous item in list

	void (*select)(void);     //called when 'sel' is selected
	void (*unselect)(void);   //called when 'sel' is unselected
	void (*refresh)(void);    //called when data on screen has changed
	void *(*make_template)(void); //called when template needed for new item
	void (*activate)(B8 redraw); 
		/* called when list is becoming active, or needs to be reactivated
			 if FALSE is passed, then do not redisplay the whole screen, its just a 
				reactivation (ex: a new file has been loaded while that list is active
			 if TRUE is passed, display the whole screen
		*/

	unsigned char (*handle_input)(struct LIST **current_list,B16 keycode); 
		/* current_list is passed so you may change it if you have more than one
			 list. keycode is the key that was hit. high byte:scancode, low byte:
				ASCII code

			 return TRUE if you changed 'current_list', else FALSE
		*/

	SB8 sel_num;      //offset of 'sel' from 'top' by number of items
	SB8 max_sel_num;  //maximum possible sel_num
	B8 ttb_dist;      //offset of 'bot' from 'top' by number of items
	B8 active;        //set to TRUE when list is active, else FALSE
	B8 nomore;        //set when no more items can be created
};

struct LIST_SEARCH_STR
{
	//only used by functions search_list_str and search_list_str_sel

	//pointer to 8 bit array used to store what to search for
	B8 *base; 
	//pointer to location to add, or remove, 8 bit blocks from 'base'
	B8 *pos; 
	//maximum possible pos, never to be overrun
	B8 *boundry;
	//search flags
	B8 flags; 
		/*
			LIST_CMP_FROM_STR0          bit 01
				compare only from the begining of the target string
			LIST_SEARCH_FROM_LISTFIRST  bit 02
				always begin search from the first item in the list
			LIST_SEARCH_NEXT            bit 03
				search forward through the list
			LIST_SEARCH_PREV            bit 04
				search backward through the list
		*/

	//pointer to item in list that matches crieteria
	void *match; 
	//called to retrieve string to compare from item 'x'
	B8 *(*tocmp)(void *x);
	//called when data on screen has changed (not used in search_list_str_sel)
	void (*refresh)(void *x);
	//called when a match is found
	B8 (*valid_match)(void *x);
		/*
			return FALSE if you do not want that item to be considered a match, else
				TRUE
		*/
};

struct LIST_SEARCH
{
	//only used by function search_list_sel

	//called to compare list->sel and whatever you want
	B8 (*cmp)(void);
		//return TRUE if the two items are a match, else return FALSE
	B8 flags; 
		/*
			LIST_SEARCH_FROM_LISTFIRST  bit 02
				always begin search from the first item in the list
			LIST_SEARCH_NEXT            bit 03
				search forward through the list
			LIST_SEARCH_PREV            bit 04
				search backward through the list
			LIST_SEARCH_MATCH_FOUND     bit 05
				TRUE when a match was found, else FALSE
		*/
};

// flag defs for LIST_SEARCH struct flags
	#define LIST_CMP_FROM_STR0          BIT_0
	#define LIST_SEARCH_FROM_LISTFIRST  BIT_1
	#define LIST_SEARCH_NEXT            BIT_2
	#define LIST_SEARCH_PREV            BIT_3
	#define LIST_SEARCH_MATCH_FOUND     BIT_4

//will delete list->sel and make all the necessary adjustments
void del_sel(struct LIST *list);
//will scroll list->sel up/down 'num' items, scrolling the list as necessary
void scroll_sel_up(struct LIST *list,B8 num,B8 show);
void scroll_sel_down(struct LIST *list,B8 num,B8 show);
//will scroll the list up/down 'num' items
B8   scroll_up(struct LIST *list,B8 num,B8 show);
B8   scroll_down(struct LIST *list,B8 num,B8 show);
//will scroll to list->first/list->last
void scroll_to_first(struct LIST *list,B8 show);
void scroll_to_last(struct LIST *list);
//will insert a new template one item about list->sel and make all the changes
//  needed
void insert_tmplt_above_sel(struct LIST *list);
//will add a new template to the end of the list
void add_tmplt_to_list(struct LIST *list);
//will handle general keys that have to do with list scrolling, etc.
void handle_geninput(struct LIST *list,B16 keycode);
	/*
		handles the following keys:
			ALT:
				UP:   calls scroll_sel_up(list,10)
				DN:   calls scroll_sel_down(list,10)
				HOME: calls scroll_to_top
				END:  calls scroll_to_bot
				R:    retries addition of template to end of list
			UP:   calls scroll_sel_up(list,1)
			DN:   calls scroll_sel_down(list,1)
			HOME: scrolls to list->top (without scrolling list)
			END:  scrolls to list->bot (without scrolling list)
			PGUP: calls scroll_up(list,list->max_sel_num)
			PGDN: calls scroll_down(list,list->max_sel_num)
	*/
//search list strings without changing list->sel
void search_list_str(struct LIST *list,struct LIST_SEARCH_STR *s_data,B8 ch);
	/*
		s_data->match is set to point to the matching item in the list
	*/
//search list strings with list->sel, and scroll when needed
void search_list_str_sel(struct LIST *list,struct LIST_SEARCH_STR *s_data,B8 ch);
	/*
		s_data->match is set to TRUE if a match was found, else FALSE
	*/
//search list with list->sel, and scroll when needed 
void search_list_sel(struct LIST *list,struct LIST_SEARCH *s_data);
	//s_data->flags&LIST_SEARCH_MATCH_FOUND is TRUE if match found, else FALSE

#endif //_LIST_H_
