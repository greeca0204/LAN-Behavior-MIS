#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct ad_list  *lshdr;
void display(AList *lst)
{
    struct ANode *phdr=lst->head;
    while(phdr)
    {
        if(phdr->dip)
        printf("FIP %s\n",phdr->dip);
        if(phdr->data)
        printf("F data %s \n",phdr->data);
        phdr=phdr->next;
    }
}
/* 1.��ʼ�����Ա����õ�����ı�ͷָ��Ϊ�� */
void initList()
{
   lshdr =(struct ad_list * ) malloc(sizeof(struct ad_list));
   lshdr->head = NULL;
   lshdr->len=0;
  //  printf("initList����ִ�У���ʼ���ɹ�\n");
}

/* 6.��鵥�����Ƿ�Ϊ�գ���Ϊ���򷵻أ������򷵻أ� */
int isEmptyList(AList *lst)
{
    struct ANode *pHead = lst->head;
    if(pHead == NULL)
    {
       // printf("isEmptyList\n");
        return 1;
    }
   // printf("NOT EmptyList\n");
    return 0;
}
 int Insert_ip_to_List(AList *lst , char * xip)
 {
        //��ֹ�ظ��ڵ�//
        struct ANode *node ;
        node =getElemNode(lst, xip);
        if(node)
        {
            return 1;
            printf("Found in 31 line\n");
        }

        node = (struct ANode *)malloc(sizeof(struct ANode ));
        if(!node)
        return 0;

        memset(node->dip,0,12);
        strcpy(node->dip,xip);
        memset(node->data,0,128);
        node -> next = lst->head ;
        lst->head = node;
//        //display(Head);
//        if(getElemNode(Head,xip))
//        {
//            printf("success insert\n");
//        }
        return 1;
 }
 int Insert_f_to_list(AList *lst,char *dip,char *fbuff)
 {
     struct ANode *pHead=NULL;
     pHead = getElemNode(lst,dip);
     if(pHead)
     {
        memset(pHead->data,0,128);
        strcpy(pHead->data,fbuff);
        printf("Here we insert file name \n");
        return 1;
     }
     return 0;
 }

/* �ӵ������в��Ҿ��и���ֵx�ĵ�һ��Ԫ�أ������ҳɹ��򷵻ظý��Ĵ洢��ַ�����򷵻�NULL */
struct ANode *getElemNode(AList *lst, char *xip)
{
    struct ANode *pHead;
    pHead = lst->head;
    if(NULL == pHead)
    {
       // printf("getElemAddr����ִ�У�����Ϊ��\n");
        return NULL;
    }
    if(xip  == NULL)
    {
       // printf("getElemAddr����ִ�У�����ֵX���Ϸ�\n");
        return NULL;
    }
//(pHead->element != x)//
    while( (strcmp(pHead->dip,xip)!=0) && (NULL != pHead->next)) //�ж��Ƿ�����ĩβ���Լ��Ƿ������Ҫ�ҵ�Ԫ��
    {
        pHead = pHead->next;
    }
    if( (strcmp(pHead->dip,xip)!=0) && (pHead != NULL))
    {
      //  printf("getElemAddr����ִ�У���������δ�ҵ�xֵ\n");
        return NULL;
    }
    return pHead;//����Ԫ�صĵ�ַ
}



/* ɾ��һ���ڵ� */
int  DelElemNode(AList *lst, char *xip)
{
    struct ANode *pHead,*pev;
    pHead = lst->head;
    if(NULL == pHead)
    {

        return 0;
    }
    if(strcmp(pHead->dip,xip)==0)
    {
        lst->head = (lst->head) ->next;
        free(pHead);
        pHead = NULL;
        return 1;
    }
    while(  (strcmp(pHead->dip,xip)!=0)  && (NULL != pHead->next) ) //�ж��Ƿ�����ĩβ���Լ��Ƿ������Ҫ�ҵ�Ԫ��
    {
        pev = pHead;
        pHead = pHead->next;
    }
    if( (strcmp(pHead->dip,xip)!=0)  && (pHead != NULL))
    {
      //  printf("getElemAddr����ִ�У���������δ�ҵ�xֵ\n");
        return 0;
    }
    pev ->next = pHead->next;
    free(pHead);
    pHead=NULL;
    return 1;//����Ԫ�صĵ�ַ
}
int desylist()
{
    struct ANode * ptr,*sss;
    ptr = lshdr->head;
    while(ptr)
    {
        sss=ptr->next;
        free(ptr);
        ptr = sss;
    }
    ptr = NULL;
    sss = NULL;
    free(lshdr);
    lshdr = NULL;
    return 1;
}
