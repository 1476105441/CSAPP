#include "cachelab.h"
#include <unistd.h>
#include <stdio.h>

//定义结构体
typedef struct Node{
    int timestamp;
    unsigned long long address;
}Node;
typedef struct Set{
    int size;
    int maxSize;
    Node* cacheArray;
}Set;

//函数声明
void checkArgs();
void handleArgs(int argc,char** args);
void initCache();
void execute();
void updateCache(unsigned long long address);

//全局变量
int s=0,e=0,b=0,flag=0;
int hits=0,misses=0,evictions=0;
int globalTime=0;//时间戳，每次操作缓存时增加
char* filename;
Set* cache;

//主函数
int main(int argc,char** args)
{
    //首先需要解析参数
    handleArgs(argc,args);
    checkArgs();
    initCache();
    execute();
    printSummary(hits, misses, evictions);
    return 0;
}
//解析参数
void handleArgs(int argc,char** args){
    int i = 1;
    for(;i < argc;i++){
        char* tmp = args[i];
        if(!strcmp(tmp,"-s")){
            if(i == argc-1){
                printf("缺少-s的参数!\n");
                exit(-1);
            }
            s = atoi(args[i+1]);
            i++;
        } else if(!strcmp(tmp,"-E")){
            if(i == argc-1){
                printf("缺少-E的参数!\n");
                exit(-1);
            }
            e = atoi(args[i+1]);
            i++;
        } else if(!strcmp(tmp,"-b")){
            if(i == argc-1){
                printf("缺少-b的参数!\n");
                exit(-1);
            }
            b = atoi(args[i+1]);
            i++;
        } else if(!strcmp(tmp,"-v")){
            //开启详细输出
            flag = 1;
        } else if(!strcmp(tmp,"-t")){
            //获取文件名
            if(i == argc-1){
                printf("缺少-t的参数!\n");
                exit(-1);
            }
            filename = args[i+1];
            i++;
        }
    }
}
//检查输出
void checkArgs(){
    if(!s){
        printf("缺少必要的参数-s，请仔细查看实验指导！\n");
    }
    if(!e){
        printf("缺少必要的参数-E，请仔细查看实验指导！\n");
    }
    if(!b){
        printf("缺少必要的参数-b，请仔细查看实验指导！\n");
    }
    if(!filename){
        printf("缺少必要的参数-t，请仔细查看实验指导！\n");
    }
    //printf("s:%d,e:%d,b:%d,filename:%s\n",s,e,b,filename);
}
//初始化缓存
void initCache(){
    int setSize = 1 << s;
    cache = (Set*)malloc(sizeof(Set)*setSize);
    int i,j;
    for(i=0;i < setSize;i++){
        cache[i].size = 0;
        cache[i].maxSize = e;
        cache[i].cacheArray = (Node*)malloc(sizeof(Node) * e);
        for(j=0;j < e;j++){
            cache[i].cacheArray[j].timestamp = -1;
            cache[i].cacheArray[j].address = -1;
        }
    }
}
//执行操作，打开文件读取内容
void execute(){
    FILE *fp = fopen(filename,"r");
    if(fp == NULL){
        printf("文件打开失败，请检查文件名是否正确！\n");
        exit(-1);
    }
    char operate;
    unsigned long long address;
    int size;
    while(fscanf(fp," %c %x,%d\n",&operate,&address,&size) > 0){
        if(flag && operate != 'I'){
            printf("%c %x,%d",operate,address,size);
        }
        switch(operate){
            case 'L':
                updateCache(address);
                break;
            case 'M': updateCache(address);
            case 'S': updateCache(address);
        }
        if(flag && operate != 'I') printf("\n");
    }
    fclose(fp);
    int i,j;
    for(i=0;i < s;i++){
        free(cache[i].cacheArray);
    }
    free(cache);
}
//更新缓存
void updateCache(unsigned long long address){
    globalTime++;
    int mask = (1 << s) - 1;
    unsigned long long ad = address >> b;
    int index = mask & (ad);
    int i,ri,time = globalTime;
    //先查看缓存是否命中
    for(i=0;i < cache[index].size;i++){
        if(cache[index].cacheArray[i].timestamp < time){
            ri = i;
            time = cache[index].cacheArray[i].timestamp;
        }
        if((cache[index].cacheArray[i].address ^ ad) == 0){
            hits++;
            cache[index].cacheArray[i].timestamp = globalTime;
            if(flag) printf(" hit");
            return;
        }
    }
    if(flag) printf(" miss");
    misses++;
    //检查是否需要替换
    if(cache[index].size < cache[index].maxSize){
        cache[index].cacheArray[cache[index].size].timestamp = globalTime;
        cache[index].cacheArray[cache[index].size].address = ad;
        cache[index].size = cache[index].size+1;
    } else {
        cache[index].cacheArray[ri].timestamp = globalTime;
        cache[index].cacheArray[ri].address = ad;
        evictions++;
        printf(" eviction");
    }
}