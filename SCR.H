#include LIBDIR"std.h"

#define CALC_SCR_OFFSET(_Mx,_My) (2*((80*_My)+_Mx))
#define HIGHLIGHT(_Mx,_My) set_text_color(_Mx,_My,1,user_colors.highlight);

//functions
void write_scr(B8 x,B8 y,B8 *str);
void clear_scr(void);
void init_scr(void);
void clean_scr(void);
void print_infoline(B8 *str,...);
void set_text_color(B8 x,B8 y,B16 length,B8 color);
