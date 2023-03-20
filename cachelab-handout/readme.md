# 实验任务

​	通过阅读实验指导，可以知道，本次实验一共有两个任务：实现一个缓存模拟器；编写一个针对缓存性能进行优化的矩阵转置函数。



# Part A

​	part A需要实现一个缓存模拟器，使用lru算法来淘汰缓存行以实现缓存的替换操作。

​	这里重点讲解lru以及缓存结构的实现，lru算法采用最简单的时间戳的方式，维护一个全局的时间戳，缓存命中时更新缓存行的时间戳，每次进行缓存操作都将全局的时间戳进行增加，淘汰缓存行时选择时间戳最小的缓存行淘汰即可，关于lru的实现还有更多不同的方式，以及一些不同的lru算法，比如为了防止出现缓存污染而采用多个队列的lru-k算法，以及mysql数据库innodb存储引擎中采用的特别的防止缓存污染策略的lru算法，感兴趣的话可以去了解一下。

​	每个缓存行的结构：

~~~c
typedef struct Node{
    int timestamp;//时间戳
    unsigned long long address;//用于标识每个缓存行的地址，后面讲解
}Node;
~~~

​	由于缓存采用组相联映射方式，每个组中可以有多个缓存行，通过-E参数控制每个组中的缓存行个数，所以定义单个组的结构如下：

~~~c
typedef struct Set{
    int size;//记录当前组中使用的缓存行个数
    int maxSize;//每个组最大的缓存行个数，其实可以通过全局参数控制
    Node* cacheArray;//缓存行数组
}Set;
~~~

​	而整个缓存的结构就是一个Set数组：

~~~c
Set* cache;
~~~

​	下面解释一下如何通过内存地址定位缓存行，一个内存地址被划分为以下三部分：

![image-20230320213004275](D:\Project\project_revelant\CSAPP\cachelab-handout\readme.assets\image-20230320213004275.png)

​	低b位是块内地址，因为读取缓存时是以块（缓存行）为单位进行读取的，所以如果读取两次同一个块内的数据，只需要从主存中读取一次（前提是没有发生缓存的替换），接着的s位是组地址，通过组地址定位到当前要读取的数据在哪个组中，在我们的代码中就是在Set数组的哪个下标中。

​	更新缓存以及替换操作如下：

~~~c
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
~~~

​	完整代码如下：

~~~c
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
~~~



**结果**

![image-20230320210847405](D:\Project\project_revelant\CSAPP\cachelab-handout\readme.assets\image-20230320210847405.png)
