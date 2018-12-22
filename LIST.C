#include "list.h"
#include <string.h>
#include <allegro.h>


B8 *_liblist_id_={"[ listlib: Version 1.0: by Stephen Kittelson: Built on " __DATE__ " ]"};

void del_sel(struct LIST *list)
{
	void *t;

	if(!list)
		return;
	if(!list->sel)
		return;

	t=list->sel;
	if(list->prev(list->sel))
	{
		*(list->pnext(list->prev(list->sel)))=list->next(list->sel);
	}
	if(list->next(list->sel))
	{
		*(list->pprev(list->next(list->sel)))=list->prev(list->sel);
	}
	if(list->next(list->bot))
	{
		list->bot=list->next(list->bot);
		if(list->sel==list->top)
		{
			if(list->top==list->first)
				list->top=list->first=list->next(list->first);
			else
				list->top=list->next(list->top);
		}
		list->sel=list->next(list->sel);
	}
	// bot==last
	else
	{
		if(list->prev(list->top))
		{
			if(list->last==list->sel)
				list->last=list->bot=list->prev(list->sel);
			list->top=list->prev(list->top);
			list->sel=list->prev(list->sel);
		}
		// top==first ttb_dist<=max_sel_num
		else
		{
			if(list->sel==list->top)
			{
				list->top=list->first=list->next(list->top);
			}
			if(list->sel==list->bot)
			{
				list->sel=list->bot=list->last=list->prev(list->bot);
			}
			else
			{
				list->sel=list->next(list->sel);
			}
			if(list->ttb_dist>0)
				list->ttb_dist--;
		}
	}
	free(t);
	list->refresh();
	return;
}

void scroll_sel_up(struct LIST *list,B8 num,B8 show)
{
	B8 c;

	if(!list->sel)
		return;
	for(c=0;c<num;c++)
	{
		if(!list->prev(list->sel))
			break;
		list->sel=list->prev(list->sel);
	}
	if(c==0)
		return;
	if(show)
		list->unselect();
	list->sel_num-=c;
	if(list->sel_num<0)
	{
		list->sel_num*=-1;
		for(c=0;c<list->sel_num;c++)
		{
			if(!list->prev(list->top))
				break;
			list->top=list->prev(list->top);
			if(list->ttb_dist==list->max_sel_num)
				list->bot=list->prev(list->bot);
			else
				list->ttb_dist++;
		}
		if(show)
			list->refresh();
		list->sel_num=0;
	}
	if(show)
		list->select();
}

void scroll_sel_down(struct LIST *list,B8 num,B8 show)
{
	B8 c;

	if(!list->sel)
		return;
	for(c=0;c<num;c++)
	{
		if(!list->next(list->sel))
			break;
		list->sel=list->next(list->sel);
	}
	if(c==0)
		return;
	if(show)
		list->unselect();
	list->sel_num+=c;
	if(list->sel_num>list->max_sel_num)
	{
		list->sel_num-=list->max_sel_num;
		for(c=0;c<list->sel_num;c++)
		{
			if(!list->next(list->bot))
				break;
			list->bot=list->next(list->bot);
			list->top=list->next(list->top);
		}
		list->sel_num=list->max_sel_num;
		if(show)
			list->refresh();
	}
	if(show)
		list->select();
}

B8 scroll_up(struct LIST *list,B8 num,B8 show)
{
	B8 c;

	if(!list->sel)
		return 0;
	for(c=0;c<num;c++)
	{
		if(!list->prev(list->top))
			break;
		list->top=list->prev(list->top);
		if(list->ttb_dist==list->max_sel_num)
			list->bot=list->prev(list->bot);
		else
			list->ttb_dist++;
		list->sel=list->prev(list->sel);
	}
	if(c==0)
		return 0;
	if(show)
		list->refresh();
	return c;
}

B8 scroll_down(struct LIST *list,B8 num,B8 show)
{
	B8 c;

	if(!list->sel)
		return 0;
	for(c=0;c<num;c++)
	{
		if(!list->next(list->bot))
			break;
		list->bot=list->next(list->bot);
		if(list->ttb_dist==list->max_sel_num)
			list->top=list->next(list->top);
		else
			list->ttb_dist++;
		list->sel=list->next(list->sel);
	}
	if(c==0)
		return 0;
	if(show)
		list->refresh();
	return c;
}

void scroll_to_first(struct LIST *list,B8 show)
{
	B8 c;

	if(list->sel==list->first)
		return;
	list->sel=list->bot=list->top=list->first;
	for(c=1;c<=list->max_sel_num && list->next(list->bot);c++)
		list->bot=list->next(list->bot);
	list->ttb_dist=c-1;
	if(show)
		list->unselect();
	list->sel_num=0;
	if(show)
	{
		list->select();
		list->refresh();
	}
}

void scroll_to_last(struct LIST *list)
{
	B8 c;

	if(list->sel==list->last)
		return;
	if(list->ttb_dist!=list->max_sel_num)
	{
		list->sel=list->last;
		list->unselect();
		list->sel_num=list->ttb_dist;
		list->select();
		return;
	}
	list->sel=list->top=list->bot=list->last;
	for(c=1;c<=list->max_sel_num && list->prev(list->top);c++)
	{
		list->top=list->prev(list->top);
	}
	list->ttb_dist=c-1;
	list->unselect();
	list->sel_num=c-1;
	list->select();
	list->refresh();
}

void insert_tmplt_above_sel(struct LIST *list)
{
	void *temp;

	if(list->nomore)
		return;
	if(!(temp=list->make_template()))
	{
		list->nomore=TRUE;
		return;
	}
	list->unselect();
	//insert into list
	*(list->pnext(temp))=list->sel;
	*(list->pprev(temp))=list->prev(list->sel);
	*(list->pprev(list->sel))=temp;
	if(list->prev(temp))
	{
		*(list->pnext(list->prev(temp)))=temp;
	}
	//move list->top and/or list->bot if needed
	if(list->sel==list->top)
	{
		if(list->top==list->first)
		{
			list->sel_num++;
			list->first=list->top=temp;
			if(list->ttb_dist!=list->max_sel_num)
				list->ttb_dist++;
			else
				list->bot=list->prev(list->bot);
		}
	}
	else
	{
		if(list->ttb_dist==list->max_sel_num)
		{
			if(list->sel==list->bot)
				list->sel=list->prev(list->sel);
			else
				list->sel_num++;
			list->bot=list->prev(list->bot);
		}
		else
		{
			list->ttb_dist++;
			list->sel_num++;
		}
	}
	list->select();
	list->refresh();
}

void add_tmplt_to_list(struct LIST *list)
{
	void *temp;

	if(list->nomore)
		return;
	if(!(temp=list->make_template()))
	{
		list->nomore=TRUE;
		return;
	}
	*(list->pnext(temp))=0;
	*(list->pprev(temp))=list->last;
	if(!list->last)
	{
		#ifdef DEBUG
			if(list->ttb_dist!=0)
				write_logs(0,1,"list.c:add_tmplt_to_list: ttb_dist!=0 && !list->last\n");
		#endif
		list->first=list->top=list->bot=list->sel=temp;
	}
	else
	{
		*(list->pnext(list->last))=temp;
		if(list->ttb_dist<list->max_sel_num)
		{
			list->ttb_dist++;
			list->bot=temp;
		}
	}
	list->last=temp;
}

void handle_geninput(struct LIST *list,B16 keycode)
{
	if(key_shifts&KB_ALT_FLAG)
	{
		switch(keycode>>8)
		{
			case KEY_UP:
			{
				scroll_sel_up(list,10,TRUE);
				return;
			}
			case KEY_DOWN:
			{
				scroll_sel_down(list,10,TRUE);
				return;
			}
			case KEY_HOME:
			{
				scroll_to_first(list,TRUE);
				return;
			}
			case KEY_END:
			{
				scroll_to_last(list);
				return;
			}
			default:
			{
				return;
			}
		}
	}
	switch(keycode>>8)
	{
		case KEY_UP:
		{
			scroll_sel_up(list,1,TRUE);
			return;
		}
		case KEY_DOWN:
		{
			scroll_sel_down(list,1,TRUE);
			return;
		}
		case KEY_HOME:
		{
			if(list->sel==list->top)
				return;
			list->unselect();
			list->sel=list->top;
			list->sel_num=0;
			list->select();
			return;
		}
		case KEY_END:
		{
			if(list->sel==list->bot)
				return;
			list->unselect();
			list->sel=list->bot;
			list->sel_num=list->ttb_dist;
			list->select();
			return;
		}
		case KEY_PGUP:
		{
			scroll_up(list,list->max_sel_num,TRUE);
			return;
		}
		case KEY_PGDN:
		{
			scroll_down(list,list->max_sel_num,TRUE);
			return;
		}
		default:
		{
			return;
		}
	}
}

void search_list_str(struct LIST *list,struct LIST_SEARCH_STR *s_data,B8 ch)
{
	B8 *ch_match;
	B8 *sstr_pos;
	void *x;
	void *(*next)(void *x);

	if(ch>=32 && ch<=126)
	{
		if(s_data->pos >= s_data->boundry)
			return;
		*(s_data->pos)=ch;
		s_data->pos++;
		*(s_data->pos)=0;
	}
	//backspace
	else if(ch==8)
	{
		if(s_data->pos==s_data->base)
			return;
		s_data->pos--;
		*(s_data->pos)=0;
	}
	else
	{
		return;
	}
	next=list->next;
	if(s_data->flags&LIST_SEARCH_FROM_LISTFIRST || !s_data->match)
	{
		x=list->first;
	}
	else if(s_data->flags&(LIST_SEARCH_NEXT|LIST_SEARCH_PREV))
	{
		x=s_data->match;
		if(s_data->flags&LIST_SEARCH_PREV)
		{
			next=list->prev;
		}
	}
	else
	{
		#ifdef DEBUG
			write_logs(0,1,"list.c:search_list_str: not specified whether to begin at list first or continue, assuming list first\n");
		#endif
		x=list->first;
	}
	s_data->match=0;
	if(s_data->flags&LIST_CMP_FROM_STR0)
	{
		while(x)
		{
			if(strncmp(s_data->tocmp(x),s_data->base,s_data->pos-s_data->base)==0)
			{
				if(!s_data->valid_match(x))
				{
					x=next(x);
					continue;
				}
				s_data->match=x;
				s_data->refresh(x);
				return;
			}
			x=next(x);
		}
	}
	else
	{
		while(x)
		{
			ch_match=strchr(s_data->tocmp(x),*(s_data->base));
			if(ch_match)
			{
				sstr_pos=s_data->base+1;
				while(*ch_match && *sstr_pos)
				{
					ch_match++;
					if(*sstr_pos!=*ch_match)
						break;
					sstr_pos++;
				}
				if(*sstr_pos==*ch_match || *sstr_pos==0)
				{
					if(!s_data->valid_match(x))
					{
						x=next(x);
						continue;
					}
					s_data->match=x;
					s_data->refresh(x);
					return;
				}
			}
			x=next(x);
		}
	}
}

void _search_list_prev(struct LIST *list)
{
	void *tobe_sel;

	if(list->sel==list->first)
		return;
	if(list->sel==list->top)
	{
		tobe_sel=list->sel;
		list->sel_num=scroll_up(list,list->max_sel_num,FALSE)-1;
		list->sel=list->prev(tobe_sel);
	}
	else 
	{
		list->sel_num--;
		list->sel=list->prev(list->sel);
	}
}

void _search_list_next(struct LIST *list)
{
	void *tobe_sel;

	if(list->sel==list->last)
		return;
	if(list->sel==list->bot)
	{
		tobe_sel=list->sel;
		list->sel_num-=scroll_down(list,list->max_sel_num,FALSE)-1;
		list->sel=list->next(tobe_sel);
	}
	else 
	{
		list->sel_num++;
		list->sel=list->next(list->sel);
	}
}

void search_list_str_sel(struct LIST *list,struct LIST_SEARCH_STR *s_data,B8 ch)
{
	B8 *ch_match,*sstr_pos;
	void (*next)(struct LIST *list);
	void *boundry;
	void *prev_top=list->top;

	if(ch>=32 && ch<=126)
	{
		if(s_data->pos >= s_data->boundry)
			return;
		*(s_data->pos)=ch;
		s_data->pos++;
		*(s_data->pos)=0;
	}
	//backspace
	else if(ch==8)
	{
		if(s_data->pos==s_data->base)
			return;
		s_data->pos--;
		*(s_data->pos)=0;
	}
	else if(ch!=0)
	{
		return;
	}
	next=_search_list_next;
	boundry=list->last;
	list->unselect();
	if(s_data->flags&LIST_SEARCH_FROM_LISTFIRST)
	{
		scroll_to_first(list,FALSE);
	}
	else if(s_data->flags&(LIST_SEARCH_NEXT|LIST_SEARCH_PREV))
	{
		if(s_data->flags&LIST_SEARCH_PREV)
		{
			next=_search_list_prev;
			boundry=list->first;
		}
		if(ch==0)
			next(list);
	}
	else
	{
		#ifdef DEBUG
			write_logs(0,1,"list.c:search_list_str: not specified whether to begin at list first or continue, assuming list first\n");
		#endif
		scroll_to_first(list,FALSE);
	}
	s_data->match=0;
	if(s_data->flags&LIST_CMP_FROM_STR0)
	{
		while(1)
		{
			if(strncmp(s_data->tocmp(list->sel),s_data->base,s_data->pos-s_data->base)==0)
			{
				if(!s_data->valid_match(list->sel))
				{
					next(list);
					continue;
				}
				(SB32)(s_data->match)=TRUE;
				break;
			}
			if(list->sel==boundry)
				break;
			next(list);
		}
	}
	else
	{
		while(1)
		{
			ch_match=strchr(s_data->tocmp(list->sel),*(s_data->base));
			if(ch_match)
			{
				sstr_pos=s_data->base+1;
				ch_match++;
				while(*ch_match && *sstr_pos)
				{
					if(*sstr_pos!=*ch_match)
						break;
					sstr_pos++;
					ch_match++;
				}
				if(*sstr_pos==0)
				{
					if(!s_data->valid_match(list->sel))
					{
						next(list);
						continue;
					}
					(SB32)s_data->match=TRUE;
					break;
				}
			}
			if(list->sel==boundry)
				break;
			next(list);
		}
	}
	if(prev_top!=list->top)
		list->refresh();
	list->select();
}

void search_list_sel(struct LIST *list,struct LIST_SEARCH *s_data)
{
	void (*next)(struct LIST *list);
	void *boundry;
	void *prev_top=list->top;

	next=_search_list_next;
	boundry=list->last;
	list->unselect();
	s_data->flags&=~LIST_SEARCH_MATCH_FOUND;
	if(s_data->flags&(LIST_SEARCH_NEXT|LIST_SEARCH_PREV))
	{
		if(s_data->flags&LIST_SEARCH_PREV)
		{
			next=_search_list_prev;
			boundry=list->first;
		}
		next(list);
	}
	else //if(s_data->flags&LIST_SEARCH_FROM_LISTFIRST)
	{
		scroll_to_first(list,FALSE);
	}
	while(1)
	{
		if(s_data->cmp())
		{
			s_data->flags|=LIST_SEARCH_MATCH_FOUND;
			break;
		}
		if(list->sel==boundry)
			break;
		next(list);
	}
	if(prev_top!=list->top)
		list->refresh();
	list->select();
}
