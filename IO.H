// flag definitions
	#define IO_SHOW_INIT_VAL  BIT_0
	#define IONEG             BIT_1
	#define IODECHIT          BIT_2
	#define IOMINNEG          BIT_3
	#define IOLOW             BIT_4
	#define IOHIGH            BIT_5
	#define IOINVALID         BIT_6
	#define IOESC             BIT_7
	#define IOINS             BIT_8
	#define IOALT             BIT_9
	#define IOUP              BIT_A
	#define IODN              BIT_B
	#define IOF1              BIT_C

B8 get_ub8(B8 *var,B8 min_value,B8 max_value,B8 max_digits,B8 *end_chars,B16 *flags);
B8 get_sb8(SB8 *var,SB8 min_value,SB8 max_value,B8 max_digits,B8 *end_chars,B16 *flags);

B8 get_ub16(B16 *var,B16 min_value,B16 max_value,B8 max_digits,B8 *end_chars,B16 *flags);
B8 get_sb16(SB16 *var,SB16 min_value,SB16 max_value,B8 max_digits,B8 *end_chars,B16 *flags);

B8 get_float(double *var,double min_value,double max_value,B8 max_bddigits,B8 max_addigits,B8 *end_chars,B16 *flags,const B8 *format_str);

B8 get_string(B8 *str,B16 max_size,B8 *end_chars,B8 *inv_chars,B16 *flags);
void fget_string(B8 *str,B16 max_size,B8 *end_chars,FILE *file);
