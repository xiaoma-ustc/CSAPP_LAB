#include "cachelab.h"
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>


typedef struct Cache_line
{
    int valid;
    int tag;
    int time_stamp;
}Cache_line;

typedef struct My_cache
{
    int S;
    int E;
    int B;
    
    Cache_line** line;
}My_cache;

int miss = 0;
int hit = 0;
int eviction = 0;
int verbose = 0;
char t[1000];
char indentifier; 

My_cache* Init_Cache(int s, int E, int b)
{
    My_cache* cache = (My_cache *)malloc(sizeof(My_cache));
    cache -> S = 1 << s;
    cache -> B = 1 << b;
    cache -> E = E;
    
    cache -> line = (Cache_line **)malloc(sizeof(Cache_line*) * cache -> S);
    
    for(int i = 0; i < cache -> S; ++i)
    {
        cache -> line[i] = (Cache_line*)malloc(sizeof(Cache_line) * E);
        
        for(int j = 0; j < E; ++j)
        {
            cache -> line[i][j].valid = 0;
            cache -> line[i][j].tag = -1;
            cache -> line[i][j].time_stamp = 0;
        }
    }
    return cache;
}

void Insert(My_cache* cache, int op_s, int op_e, int op_tag)
{
    cache -> line[op_s][op_e].valid = 1;
    cache -> line[op_s][op_e].tag = op_tag;
    cache -> line[op_s][op_e].time_stamp = 0;
    for(int i = 0; i < cache -> E; ++i)
    {
        if(cache -> line[op_s][i].valid == 1)
        {
            ++cache -> line[op_s][i].time_stamp;
        }
    }
    
}

int LRU(My_cache* cache, int op_s)
{
    int res_e = 0;
    int res_sta = 0;
    for(int i = 0; i < cache -> E; ++i)
    {
        if(cache -> line[op_s][i].time_stamp > res_sta)
        {
            res_e = i;
            res_sta = cache -> line[op_s][i].time_stamp;
        }
    }
    return res_e;
}

int Get_Index(My_cache* cache, int op_s, int op_tag)
{
    int full_index = -1;
    for(int i = 0; i < cache -> E; ++i)
    {
        if(cache -> line[op_s][i].valid && cache -> line[op_s][i].tag == op_tag)
        {
            return i;
        }
        if(cache -> line[op_s][i].valid == 0 && full_index == -1)
        {
            full_index = i;
        }
    }
    return full_index;
}

void Update(My_cache* cache, int op_s, int op_tag)
{
    int op_e = Get_Index(cache, op_s, op_tag);
    if(cache -> line[op_s][op_e].tag != op_tag)
    {
        ++miss;
        if(verbose)
        {
            printf("%c  %d miss!  ",indentifier, op_s);
        }
        if(op_e == -1)
        {
            ++eviction;
            if(verbose)
            {
                printf("evication!");
            }
            
            op_e = LRU(cache, op_s);
        }
        printf("\n");
        Insert(cache, op_s, op_e, op_tag);
        
    }
    else
    {
        ++hit;
        if(verbose)
        {
            printf("%c  %d  %d hit! \n",indentifier, op_s, op_e);
        }
        Insert(cache, op_s, op_e, op_tag);
    }
    

}

void Get_Trace(My_cache* cache, int s, int E, int b)
{
    FILE* fp;
    fp = fopen(t, "r");
    if(fp == NULL)
    {
        printf("file open error!");
        exit(-1);
    }
    unsigned address;
    int size;
    while(fscanf(fp, " %c %x,%d", &indentifier, &address, &size) > 0)
    {
        
        int op_tag = address >> (s + b);
        int op_s = (address >> b) & (0xFFFFFFFF >> (8 * sizeof(unsigned) - s));
        if(indentifier == 'M')
        {
            Update(cache, op_s, op_tag);
            Update(cache, op_s, op_tag);
        }
        else if(indentifier == 'L')
        {
            Update(cache, op_s, op_tag);
        }
        else if(indentifier == 'S')
        {
            Update(cache, op_s, op_tag);
        }
        else if(indentifier =='I')
        {
            continue;
        }
    }

    fclose(fp);
}

void Help()
{
    printf("** A Cache Simulator by Xiaoma\n");
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("-h         Print this help message.\n");
    printf("-v         Optional verbose flag.\n");
    printf("-s <num>   Number of set index bits.\n");
    printf("-E <num>   Number of lines per set.\n");
    printf("-b <num>   Number of block offset bits.\n");
    printf("-t <file>  Trace file.\n\n\n");
    printf("Examples:\n");
    printf("linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void Free_Cache(My_cache* cache)
{
    for(int i = 0; i < cache -> S; ++i)
    {
        free(cache -> line[i]);
    }
    free(cache -> line);
    free(cache);
}

int main(int argc, char* argv[])
{
    char opt;
    int s;
    int E;
    int b;

    while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            Help();
            exit(0);
        case 'v':
            verbose = 1;
            break;
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            strcpy(t, optarg);
            break;
        default:
            Help();
            exit(-1);
        }
    }
    My_cache* cache = Init_Cache(s, E, b);
    Get_Trace(cache, s, E, b);
    Free_Cache(cache);
    printSummary(hit, miss, eviction);
    return 0;
}





