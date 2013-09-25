#include <iostream>
#include <dirent.h>
#include <stdio.h>
#include <cstring>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <map>
#include <set>
#include <vector>
using namespace std;

typedef struct flist
{
    char *data;
    char *name;
    int fd;
    int size;
} filelist_t;

int count = 0;

filelist_t *makeup(char *dirname)
{
    filelist_t *filelist = NULL;
    struct dirent **namelist;

    int n = scandir(dirname, &namelist, 0, alphasort);

    printf("scandir n = %d\n",n);
    if (n < 0)
    {
        printf("sacn dir failed!\n");
        exit(-1);
    }

    filelist = (filelist_t*)malloc(sizeof(filelist_t)*n);

    int i;

    for (i = 0; i < n; i++)
    {
        if (strcmp(namelist[i]->d_name, ".") != 0 &&
                strcmp(namelist[i]->d_name, "..") != 0)
        {
            filelist[count].name = strdup(dirname);

            strcat(filelist[count].name, namelist[i]->d_name);

            //printf("file name = %s\n ", filelist[count].name);

            struct stat finfo;
            filelist[count].fd = open(filelist[count].name, O_RDONLY);
            fstat(filelist[count].fd, &finfo);
            filelist[count].size = finfo.st_size + 1;
            filelist[count].data = (char*)malloc(filelist[count].size);
            read(filelist[count].fd, filelist[count].data, finfo.st_size);
            filelist[count].data[filelist[count].size - 1] = '\0';
            //filelist[count].data = (char*)mmap(0, finfo.st_size + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, filelist[count].fd, 0);

            //printf("%s\n", filelist[count].data);
            count++;
        }
    }

    return filelist;
}

#define START		0x00
#define IN_TAG		0x01
#define IN_ATAG		0x02
#define FOUND_HREF	0x03
#define START_LINK	0x04
#define LINE_END    0x05

int main()
{
    //read all files
    float start_time = clock();
    filelist_t * files =
        makeup("/home/zhxfl/cuda-workspace/InvertIndex/data/");
    int state = START;
    map<string, set <string> >Map;

    for(int i = 0; i < count; i++)
    {
        char *buf = files[i].data;
        char* link_end;
        for(int j = 0; j < files[i].size; j++)
        {
            switch(state)
            {
            case START:
                if (buf[j] == '<')
                {
                    state = IN_TAG;
                    //	printf("%c - START -> IN_TAG\n", buf[j]);
                }
                break;
            case IN_TAG:
                if (buf[j] == 'a')
                {
                    state = IN_ATAG;
                    //	printf("%c - IN_TAG -> IN_ATAG\n", buf[j]);
                }
                else if (buf[j] == ' ')
                {
                    state = IN_TAG;
                    //	printf("%c - IN_TAG -> IN_TAG\n", buf[j]);
                }
                else state = START;
                break;
            case IN_ATAG:
                if (buf[j] == 'h')
                {
                    char href[5] = {'h','r','e','f','\0'};
                    char* url_start = buf + j;
                    int x;
                    for (x = 0; x < 5; x++)
                        if (href[x] != url_start[x]) break;
                    if (href[x] == '\0')
                    {
                        state = FOUND_HREF;
                        //printf("%c - IN_ATAG -> FOUND_HREF, %c\n", buf[j], buf[j+3]);
                        j+=3;
                    }
                    else state = START;
                }
                else if (buf[j] == ' ') state = IN_ATAG;
                else state = START;
                break;
            case FOUND_HREF:
                if (buf[j] == ' ') state = FOUND_HREF;
                else if (buf[j] == '=') state = FOUND_HREF;
                else if (buf[j] == '\"')
                {
                    state = START_LINK;
                    //	printf("%c - FOUND_HREF -> START_LINK\n", buf[j]);
                }
                else state = START;
                break;
            case START_LINK:
                link_end = NULL;
                link_end = buf + j;
                for (; *link_end != '\"'; link_end++);
                *link_end = '\0';
                //printf("%s\n", buf + j);
                string s(buf + j);
                int static id = 1;
                //printf("%d = %s\n", id++, s.c_str());
                if(s.size() != 0)
                {
                    Map[s].insert(string(files[i].name));
                    //printf("%d = %s\n", id++, s.c_str());
                }
                //emit
                //printf("*link_end:%c\n", *link_end);
                //printf("url:%s\n", buf + j);
                //printf("%c - START_LINK -> START, %c\n", buf[j], buf[(link_end - (buf + j))]);
                //pKey->url_offset = j + block_offset;
                //EMIT_INTER_COUNT_FUNC(sizeof(II_KEY_T), sizeof(int));
                j += (link_end - (buf + j));
                state = START;
                break;
            }
        }
    }
    float end_time = clock();
    printf("time %f\n", (end_time - start_time) / CLOCKS_PER_SEC);
    freopen("a.txt", "w", stdout);
    map<string, set<string> >::iterator it;
    int k = 0;
    for(it = Map.begin(); it != Map.end() && k < 10000; it++, k++)
    {
        printf("=========================================\n");
        printf("URL:%s\n", it->first.c_str());
        printf("HTLM:\n");
        for(set<string>::iterator i = it->second.begin(); i != it->second.end(); i++)
        {
            printf("%s\n",(*i).c_str());
        }
    }
    return 0;
}

