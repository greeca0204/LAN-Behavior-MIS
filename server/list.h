struct ANode
{
       char  data[128];   /*���ݳ�Ա�����Ƕ����ͬ���͵�����*/
       char dip[12];
       struct  ANode  *next;     /*ָ�������Աֻ����-��*/
};
typedef struct ad_list
{
    struct ANode *head;
    int len;
}AList;
void initList();
int isEmptyList(AList *lst);
int Insert_ip_to_List(AList *lst, char * xip);
 int Insert_f_to_list(AList *lst ,char *dip,char *fbuff);
struct ANode *getElemNode(AList *lst, char *xip);
int  DelElemNode(AList *lst, char *xip);
void display(AList *lst);
int desylist();
