struct FILE_HIST
{
	B8 name[100];
	struct FILE_HIST *next;
};

B8   _load_file(B8 *path,B8 limited_run);
#define load_file(_Mpath) _load_file(_Mpath,FALSE)
B8   write_file(B8 *path);
void print_tl(void);
void read_filehist(int fhandle);
void clean_file(void);
