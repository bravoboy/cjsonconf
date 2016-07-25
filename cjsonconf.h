#ifndef cjsonconf_h_
#define cjsonconf_h_

#ifdef __cplusplus
extern "C" 
{
#endif

#define cjsonconf_int   (1 << 0)
#define cjsonconf_float (1 << 1)
#define cjsonconf_string (1 << 2)
#define cjsonconf_array  (1 << 3)
#define cjsonconf_object (1 << 4)


typedef struct cjsonconf {
    struct cjsonconf *next;
    struct cjsonconf *child;

    int type;
    void *value;
    char *key;
} cjsonconf;


extern cjsonconf* parse_conf_file(char *filename);
extern int  cjsonconf_getarraysize(cjsonconf *array);
extern cjsonconf *cjsonconf_getarrayitem(cjsonconf *array,int item);

extern cjsonconf *cjsonconf_getobjectitem(cjsonconf *object,const char *string);

#ifdef __cplusplus
}
#endif
#endif
