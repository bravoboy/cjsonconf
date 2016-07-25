#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include "cjsonconf.h"

static char * g_value;
static char * g_data;
static char  * g_curr;
static char *skip(char *in) {
    while (in && *in && (unsigned char)*in <= 32) in++;
    return in;
}
static int parse_key(cjsonconf *item);
static int parse_string(cjsonconf *item);
static int parse_number(cjsonconf *item);
static int parse_object(cjsonconf *item);
static int parse_array(cjsonconf *item);
static int parse_key(cjsonconf *item) {
    char *ptr = g_curr;
    
    if (*g_value != '\"') {
        return -1;
    }
    g_value++;
    while (*g_value != '\"' && *g_value) {
        *g_curr = *g_value;
        g_value++;
        g_curr++; 
    }
    if (*g_value == '\0') {
        return -1;
    }
    *g_curr = '\0';
    g_curr++;
    item->key = ptr;
    g_value = skip(g_value+1);
    return 0;
}
static int parse_string(cjsonconf *item) {
    if (*g_value != '\"') {
        return -1;
    }
    char *ptr = g_curr;
    g_value++;
    while (*g_value != '\"' && *g_value) {
        *g_curr = *g_value;
        g_value++;
        g_curr++; 
    }
    if (*g_value == '\0') {
        return -1;
    }
    *g_curr = '\0';
    g_curr++;
    item->value = ptr;
    item->type = cjsonconf_string;
    g_value = skip(g_value+1);
    return 0;
}
static int parse_number(cjsonconf *item) {
    char *ptr = g_curr;
    if (*g_value == '-') {
        *g_curr = *g_value;
        g_curr++;
        g_value++;
    }
    if (!(*g_value >= '0' && *g_value <= '9')) {
        return -1;
    }
    while (*g_value >= '0' && *g_value <= '9' ) {
        *g_curr = *g_value;
        g_curr++;
        g_value++;
    }
    if (*g_value == '.') {
        *g_curr = *g_value;
        g_curr++;
        g_value++;
        item->type = cjsonconf_float;
        while (*g_value >= '0' && *g_value <= '9' ) {
            *g_curr = *g_value;
            g_curr++;
            g_value++;
        }
    } else {
        item->type = cjsonconf_int;
    }
    *g_curr = '\0';
    g_curr++;
    item->value = ptr;
    g_value = skip(g_value);
    return 0;
}
static int parse_array(cjsonconf *item) {
    cjsonconf *child;
    if (*g_value != '[') {
        return -1;
    }    /* not an array! */

    item->type = cjsonconf_array;
    g_value = skip(g_value+1);
    if (*g_value == ']') {
        g_value = skip(g_value+1);
        return 0;        /* empty array. */
    }
    item->child = child = (cjsonconf*)calloc(1,sizeof(*child));
    if (!item->child) {
        return -1;              /* memory fail */
    }
    
    int ret = parse_value(child);     /* skip any spacing, get the value. */
    if (ret != 0) return -1;
    while (*g_value == ',') {
        cjsonconf *new_item = (cjsonconf*)calloc(1,sizeof(*new_item));
        if (!new_item) return -1;     /* memory fail */
        child->next = new_item;
        child = new_item;
        g_value = skip(g_value+1);
        ret = parse_value(child);
        if (ret != 0) return -1;   /* memory fail */
    }   

    if (*g_value == ']') {
        g_value = skip(g_value+1);
        return 0;
    } else {
        return -1;
    }
}

static int parse_object(cjsonconf *item) {
    cjsonconf *child;
    int ret = 0;
    if (*g_value != '{') {
        return -1;
    }  /* not an object! */

    item->type = cjsonconf_object;
    g_value = skip(g_value+1);
    if (*g_value == '}') {
        g_value = skip(g_value+1);
        return 0;        /* empty array. */
    }
    item->child = child = (cjsonconf*)calloc(1,sizeof(*child));
    if(!child) {
        return -1;
    }
    ret = parse_key(child);
    if (ret != 0) {
        return -1;
    }

    if (*g_value != ':') {
        return -1; 
    }   /* fail! */
    
    g_value = skip(g_value+1);

    ret = parse_value(child);   /* skip any spacing, get the value. */
    if (ret != 0) {
        return -1;
    }
    while (*g_value == ',') {
        cjsonconf *new_item = (cjsonconf*)calloc(1,sizeof(*child));
        if (!new_item) {
            return -1;
        }
        child->next = new_item;
        child = new_item;

        g_value = skip(g_value+1);
        ret = parse_key(child);
        if (ret != 0) {
            return ret;
        }

        if (*g_value != ':') {
            return -1;
        }   /* fail! */
        
        g_value = skip(g_value+1);

        ret = parse_value(child);   /* skip any spacing, get the value. */
        if (ret != 0) {
            return -1;
        }
    }
    if (*g_value != '}') {
        return -1;
    }
    g_value = skip(g_value+1);
    item->type = cjsonconf_object;
    return 0;
}
int parse_value(cjsonconf *item) {
    if (!g_value)
        return -1;
    if (*g_value == '\"') { 
        return parse_string(item);
    }
    if (*g_value == '-' || (*g_value >= '0' && *g_value <= '9')) {
        return parse_number(item);
    }
    if (*g_value == '[') {
        return parse_array(item);
    }
    if (*g_value == '{') {
        return parse_object(item);
    }
    return -1;
}


cjsonconf *cjsonconf_parse(char *value) {
    cjsonconf *root = (cjsonconf*)calloc(1,sizeof(*root));
    if(!root) {
        fprintf(stderr,"malloc root error");
        exit(1);
    }
    g_value = skip(value);
    int ret = parse_object(root);
    if (ret != 0) {
        fprintf(stderr,"json style error");
        exit(1);
    }
    return root;
}

cjsonconf* parse_conf_file(char *filename) {
    FILE *f = fopen(filename,"rb");
    fseek(f,0,SEEK_END);
    int len = ftell(f);
    fseek(f,0,SEEK_SET);
    char *data = (char*)malloc(len+1);
    g_data = (char*)malloc(len*2);
    g_curr = g_data;
    fread(data,1,len,f);
    data[len]='\0';
    fclose(f);
    return cjsonconf_parse(data);
}
int explode(char *string,char **s) {
    char *ptr = string;
    char *end = string + strlen(string) - 1;
    int idx = 0;
    while (string <= end) {
        ptr = string;
        while(string <= end && *string != '.') {
            string++;
        }
        *string = '\0';
        s[idx++] = ptr;
        if (idx == 16) {
            return idx;
        }
        string++;
    }
    return idx; 
}
cjsonconf *cjsonconf_getobjectitem(cjsonconf *object,const char *string) {
    if (object == NULL) {
        return NULL;
    }
    char *str[16];
    char *data = (char*)malloc(strlen(string)+1);
    memcpy(data,string,strlen(string));
    data[strlen(string)] = '\0';
    int res = explode(data,str);
    cjsonconf * c = object->child;
    int idx = 0;
    while (idx < res) {
        while (c && strcmp(c->key,str[idx]) != 0) {
            c = c->next;
        }
        if (c == NULL) {
            break;
        }
        idx++;
        if (idx == res)
            break;
        c = c->child; 
    }
    free(data);
    return c;
}

int cjsonconf_getarraysize(cjsonconf *object) {
    int res = 0;
    cjsonconf *c = object->child;
    while(c) {
        res++;
        c = c->next;
    }
    return res; 
}

cjsonconf *cjsonconf_getarrayitem(cjsonconf *object,int item) {
    int res = 0;
    cjsonconf *c = object->child;
    while(res != item && c) {
        res++;
        c = c->next;
    }   
    return c;
}
