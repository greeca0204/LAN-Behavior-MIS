#include "http_hash_func.h"
#include "public.h"
#include "ringbuff.h"
#include <pthread.h>
#include <glib.h>
#include <string.h>
#include <glib.h>

extern GHashTable *http_hash_white_list;
extern GHashTable *http_hash_person_list;
extern GHashTable *http_hash_group_list;
extern GHashTable *http_hash_whole_list;
extern GHashTable *user_to_gid_hash_list;

//void free_data(gpointer data)
//{
//    g_free(data);
//    data = NULL;
//}

void http_hase_table_create()
{
//    http_hash_white_list=g_hash_table_new(g_str_hash,g_str_equal);
////////    http_hash_black_list = g_hash_table_new(g_str_hash, g_str_equal);
    http_hash_white_list=g_hash_table_new_full(g_str_hash,g_str_equal,
                                               free_data,free_data);

    http_hash_person_list = g_hash_table_new_full(g_str_hash,g_str_equal,
                            free_data,free_data);
//    http_hash_person_list = g_hash_table_new(g_str_hash,g_str_equal);


    http_hash_group_list = g_hash_table_new_full(g_str_hash,g_str_equal,
                           free_data,free_data);
//    http_hash_group_list = g_hash_table_new(g_str_hash,g_str_equal);


    http_hash_whole_list = g_hash_table_new_full(g_str_hash, g_str_equal,
                           free_data,free_data);
//    http_hash_whole_list = g_hash_table_new(g_str_hash, g_str_equal);
}




//���Ҹ��˰����� ���˺����� ������� ȫ�ֺ�����  ��Ҫ�����򷵻� true
int http_hash_look_up(char *http_get_tmp,Sinfo *packet_tmp)
{
    char str_cmp[100];
    char gid[32];

    memset(str_cmp,0,100);
    strcpy(str_cmp,http_get_tmp);//��ȡURL
    strncat(str_cmp,packet_tmp->src_ip,sizeof(packet_tmp->src_ip));//��ȡIP���Խ��к��������˲���

    //printf("Here gets URL::::%s\n",str_cmp);


    if(g_hash_table_lookup(http_hash_white_list, str_cmp))  return 0;  //������
    else
    {
        if(g_hash_table_lookup(http_hash_person_list, str_cmp))
        {
            //printf("person  list \n\n"); //�����������ˣ�
            return 1;
        }
        else
        {
            memset(str_cmp,0,100);
            strcpy(str_cmp,http_get_tmp);//���½��в����ַ�����ƥ��
            if(g_hash_table_lookup(user_to_gid_hash_list,packet_tmp->src_ip))
            {
                memset(gid,0,32);
                strcpy(gid,g_hash_table_lookup(user_to_gid_hash_list,packet_tmp->src_ip));
            }

            strncat(str_cmp,gid,sizeof(gid));
            //printf("GID %s\n",str_cmp);
            if(g_hash_table_lookup(http_hash_group_list, str_cmp))
            {
                //printf("group list \n\n");   //���������飩
                return 1;
            }
            else
            {
                //printf("Whole::::%s\n",http_get_tmp);
                if(g_hash_table_lookup(http_hash_whole_list, http_get_tmp))
                {
                    //printf("whole list %s\n",(char *)g_hash_table_lookup(http_hash_whole_list, http_get_tmp)); //��������ȫ�֣�
                    return 1;
                }
            }
        }
    }
    return 0;
}
