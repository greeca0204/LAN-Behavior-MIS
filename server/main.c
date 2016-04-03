#include "public.h"
#include "config.h"
#include "sql_func.h"
#include "http_hash_func.h"
#include "IM.h"
#include "http.h"
#include "usr_func.h"
#include "ringbuff.h"
#include "spk.h"
//#include "app_func.h"
#include "audit.h"
#include <pcre.h>
#include <unistd.h>
#include <stdlib.h>
#include <glib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
//http queue//
Ringbuff *cb_http_a; //a for process & pcap��httpץ���İ�

//normal part//
Ringbuff *cb_pk_a; //��tcpץ���İ�

Ringbuff *cb_tcp_a; //������·��.ip��.tcp������İ�
Ringbuff *cb_tcp_rst;//���������ַ���������ڰ�����׼�������ذ��İ�

Ringbuff *cb_udp_a;
Ringbuff *cb_udp_b;

//��ƻ���
Ringbuff *cb_spk_aduit_a;//��׼����Ƶİ�

//mysql
MYSQL mysql;


//system config
char NET_INTERFACE[5];
char SUB_NET[16];
char GATEWAY[17];
u_int8_t FACK_GATEWAY_MAC[7];


//hashtable
//http
GHashTable *http_hash_white_list;
GHashTable *http_hash_person_list;
GHashTable *http_hash_group_list;
GHashTable *http_hash_whole_list;

//APP
GHashTable *app_hash_white_list;
GHashTable *app_hash_person_list;
GHashTable *app_hash_group_list;
GHashTable *app_hash_whole_list;

GHashTable *app_tcp_hash_IPbuff_list;//ֵ��������ʽ���������
GHashTable *app_udp_hash_IPbuff_list;

//hash user
GHashTable *user_to_gid_hash_list;
GHashTable *user_to_uid_hash_list;
GHashTable *user_to_name_hash_list;

//hash pattern
pcre * tcpPcre[1000]; //�ȶ���1000..�Ժ󲻹�.
GHashTable *app_hash_tcp_pattern_list;
char hash_tcp_pattern[1000][300];
char hash_tcp_pattern_name[1000][20];

pcre * udpPcre[1000]; //�ȶ���1000..�Ժ󲻹�.
GHashTable *app_hash_udp_pattern_list;
char hash_udp_pattern[1000][300];
char hash_udp_pattern_name[1000][20];

//pcap bpf signal//
int pcap =0;
pthread_mutex_t *mutex_for_bpf;
pthread_cond_t *notUse_bpf;// *Use_bpf

//http�ź���//
pthread_mutex_t *mutex_for_cb_http_a;
pthread_cond_t *notFull_http_a, *notEmpty_http_a;


//normal �ź���//

//source//
pthread_mutex_t *mutex_for_cb_pk_a;
pthread_cond_t *notFull_pk_a, *notEmpty_pk_a;

//tcp//
pthread_mutex_t *mutex_for_cb_tcp_a;
pthread_cond_t *notFull_tcp_a, *notEmpty_tcp_a;


//TCP RST �߲�  http �� tcp �߲㹲��//
pthread_mutex_t *mutex_for_cb_rst;
pthread_cond_t *notFull_tcpcb_rst, *notEmpty_tcpcb_rst;

//udp//
pthread_mutex_t *mutex_for_cb_udp_a;
pthread_cond_t *notFull_udp_a, *notEmpty_udp_a;
pthread_mutex_t *mutex_for_cb_udp_b;
pthread_cond_t *notFull_udp_b, *notEmpty_udp_b;

//��ƻ�����ź���������ͬ��ץ��������߳�//
pthread_mutex_t *mutex_for_cb_spk_audit_a; // ����ͬ��  ������a���߳� ������߳�//
pthread_cond_t *notFull_spk_audit_a, *notEmpty_spk_audit_a;// ����ͬ��  ������a���߳� ������߳�//

//�̳߳�//
char pooldata[15]="thread_pool";//pool's new job.no more mean!
//GThreadPool *threadpool_http_pcap;
GThreadPool *threadpool_http_process;
GThreadPool *threadpool_tcp_rst;
//GThreadPool *threadpool_spk_pcap;
GThreadPool *threadpool_spk_process;
GThreadPool *threadpool_tcp_deepprocess;
GThreadPool *threadpool_udp_process;
GThreadPool *threadpool_udp_block;

pthread_t thread_http_pcap;
//    pthread_t thread_http_process,thread_http_process;
pthread_t thread_spk_pcap,thread_udp_pcap;
//    pthread_t thread_tcp_deepprocess,thread_udp_deepprocess,thread_spk_process;
//    pthread_t thread_tcp_rst;
pthread_t thread_audit_pcap,thread_spk_audit;

void sigproc( int sig );


int main(int argc,char **argv)
{
    /* regise signal ctrl+c stop capture */
    signal( SIGINT, sigproc ) ;

    int ret=0;
    int q;
    for(q=0; q<=1000; q++)
    {
        memset(hash_tcp_pattern[q],0,300);
        memset(hash_tcp_pattern_name[q],0,20);

        memset(hash_udp_pattern[q],0,300);
        memset(hash_udp_pattern_name[q],0,20);

    }

    //sleep(2);
//    printf("Hello world!\n");



    //httpѭ������a,b


    //��ʼ��normal part ѭ������
    INIT_RINGBUFF(cb_pk_a,Sinfo,1024);
    INIT_RINGBUFF(cb_http_a,Sinfo,1024);  //ringbuff ��ʼ��
    INIT_RINGBUFF(cb_tcp_a,TCPinfo,1024);

    INIT_RINGBUFF(cb_tcp_rst,TCPinfo,1024);

    INIT_RINGBUFF(cb_udp_a,UDPinfo,1024);
    INIT_RINGBUFF(cb_udp_b,UDPinfo,1024);
    INIT_RINGBUFF(cb_spk_aduit_a,Sinfo,4096); //�����СҪ����һ���Ƕ��٣���������Ʋ���Ҫ��.����Ҫ��©��//

    //char *sql_result;
    //��ʼ��sql
    sql_start(&mysql);
    mysql_query(&mysql,"set character set utf8");

    sql_get_system_config(&mysql);

    //http��ϣ���ʼ
    http_hase_table_create();

    printf("url personal white list \n");
    sql_get_website_to_part_hash(GET_URL_white_list,&mysql,white_list,http_hash_white_list);

    printf("url personal black list \n");
    sql_get_website_to_part_hash(GET_URL_person_list,&mysql,person_list,http_hash_person_list);

    printf("url group black list \n");
    sql_get_website_to_part_hash(GET_URL_group_list,&mysql,group_list,http_hash_group_list);

    printf("url whole black list \n");
    sql_get_website_to_whole_hash(GET_URL_whole_list,&mysql,whole_list,http_hash_whole_list);


    //app��ϣ���ʼ��
    app_hase_table_create();
    printf("app personal white list \n");
    sql_get_app_to_hash(GET_APP_white_list,&mysql,white_list,app_hash_white_list);

    printf("app personal black list \n");
    sql_get_app_to_hash(GET_APP_person_list,&mysql,person_list,app_hash_person_list);

    printf("app group black list \n");
    sql_get_app_to_hash(GET_APP_group_list,&mysql,group_list,app_hash_group_list);

    printf("app whole black list \n");
    sql_get_app_to_whole_hash(GET_APP_whole_list,&mysql,whole_list,app_hash_whole_list);

//
//    void iterator(gpointer key, gpointer value, gpointer user_data)
//                {
//                    g_print(user_data, key, value);
//                }
//
//                g_hash_table_foreach(app_hash_white_list,(GHFunc)iterator,"The hash of %s is %s \n");

    //pattern hash and ����������ʽ
    sql_get_pattern_to_hash(GET_TCP_APP_pattern,&mysql,app_hash_tcp_pattern_list,"TCP");
    sql_get_pattern_to_hash(GET_UDP_APP_pattern,&mysql,app_hash_udp_pattern_list,"UDP");


    //�û���ϣ���ʼ��//
    user_hase_table_create();
    sql_get_user_to_hash(GET_user_gid_list,&mysql,user_to_gid_hash_list);
    sql_get_user_to_hash(GET_user_uid_list,&mysql,user_to_uid_hash_list);
    sql_get_user_to_hash(GET_user_name_list,&mysql,user_to_name_hash_list);


    /*���������ź�����ʼ��*/
    //pcap bpf//
    mutex_for_bpf = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex_for_bpf, NULL);
//    Use_bpf = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
//    pthread_cond_init(Use_bpf, NULL);
    notUse_bpf = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notUse_bpf, NULL);


    //http//  //a//
    mutex_for_cb_http_a = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex_for_cb_http_a, NULL);
    notFull_http_a = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notFull_http_a, NULL);
    notEmpty_http_a = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notEmpty_http_a, NULL);



    //normal part//
    //source part //
    mutex_for_cb_pk_a = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex_for_cb_pk_a, NULL);
    notFull_pk_a = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notFull_pk_a, NULL);
    notEmpty_pk_a = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notEmpty_pk_a, NULL);

    //audit signal
    mutex_for_cb_spk_audit_a = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex_for_cb_spk_audit_a, NULL);
    notFull_spk_audit_a = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notFull_spk_audit_a, NULL);
    notEmpty_spk_audit_a = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notEmpty_spk_audit_a, NULL);

    //udp//
    //a//
    mutex_for_cb_udp_a = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex_for_cb_udp_a, NULL);
    notFull_udp_a = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notFull_udp_a, NULL);
    notEmpty_udp_a = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notEmpty_udp_a, NULL);
//    //b//
    mutex_for_cb_udp_b = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex_for_cb_udp_b, NULL);
    notFull_udp_b = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notFull_udp_b, NULL);
    notEmpty_udp_b = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notEmpty_udp_b, NULL);

    //tcp//
    //a//
    mutex_for_cb_tcp_a = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex_for_cb_tcp_a, NULL);
    notFull_tcp_a = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notFull_tcp_a, NULL);
    notEmpty_tcp_a = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notEmpty_tcp_a, NULL);

    //TCP RST //mutex_for_cb_rst   notFull_tcpcb_rst  notEmpty_tcpcb_rst
    mutex_for_cb_rst = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex_for_cb_rst, NULL);
    notFull_tcpcb_rst = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notFull_tcpcb_rst, NULL);
    notEmpty_tcpcb_rst = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(notEmpty_tcpcb_rst, NULL);

    //��ʼ�����̺߳��̳߳�


    /*
    ����1��ָ���̱߳�ʶ��ָ�롣
    ����2���߳����ԡ�
    ����3���߳����к�����ʼ��ַ��
    ����4�����к����Ĳ�����
    �����̳߳ɹ�ʱ����������0������Ϊ0��˵�������߳�ʧ�ܣ������Ĵ��󷵻ش���ΪEAGAIN��EINVAL��ǰ�߱�ʾϵͳ���ƴ����µ��̣߳������߳���Ŀ�����ˣ����߱�ʾ�ڶ�������������߳�����ֵ�Ƿ���
    */
//	/*����http�������߳�*/
//    threadpool_http_pcap=g_thread_pool_new(http_pcap, NULL, 1, TRUE, NULL);
//    if (threadpool_http_pcap==NULL)
    ret = pthread_create(&thread_http_pcap, NULL, (void*)http_pcap, NULL);
    if(ret)
    {
        printf("Create thread_http_pcap error!\n");
        return -1;
    }
    //else printf("Create thread_http_pcap success\n");


    /*����http�������߳�*/
//    ret = pthread_create(&thread_http_process, NULL, (void*)process_http, NULL);
    g_thread_init(NULL);
    threadpool_http_process=g_thread_pool_new(process_http, NULL, 5, TRUE, NULL);
//    if (ret)
    if (threadpool_http_process==NULL)
    {
        printf("Create thread_http_process error!\n");
        return -1;
    }
    //else printf("Create thread_http_process success\n");

//
    /*����normal�������߳�*/
    ret = pthread_create(&thread_spk_pcap, NULL, (void*)spk_pcap, NULL);
    if (ret)
//    g_thread_init(NULL);
//    threadpool_spk_pcap=g_thread_pool_new(spk_pcap, NULL, 1, TRUE, NULL);
//    if (threadpool_spk_pcap==NULL)
    {
        printf("Create thread_normal_pcap error!\n");
        return -1;
    }
    else printf("Create thread_normal_pcap success\n");
//
//
//    /*����normal�������߳�*/
//    ret = pthread_create(&thread_spk_process, NULL, (void*)spk_process, NULL);
//    if (ret)
    g_thread_init(NULL);
    threadpool_spk_process=g_thread_pool_new(spk_process, NULL, 5, TRUE, NULL);
    if (threadpool_spk_process==NULL)
    {
        printf("Create thread_normal_process error!\n");
        return -1;
    }
    else printf("Create thread_normal_process success\n");
//
//
    /*����tcp����ȴ����߳�*/
//    ret = pthread_create(&thread_tcp_deepprocess, NULL, (void*)tcp_deepprocess, NULL);
//    if (ret)
    g_thread_init(NULL);
    threadpool_tcp_deepprocess=g_thread_pool_new(tcp_deepprocess, NULL, 5, TRUE, NULL);
    if (threadpool_tcp_deepprocess==NULL)
    {
        printf("Create thread_TCP_deepprocess error!\n");
        return -1;
    }
    else printf("Create thread_TCP_deepprocess success\n");
//
//
    //����TCP rst �߳�//
//    ret = pthread_create(&thread_tcp_rst, NULL, (void*)tcp_rst_tcp, NULL);
//    if (ret)
    g_thread_init(NULL);
    threadpool_tcp_rst=g_thread_pool_new(tcp_rst_tcp, NULL, 10, TRUE, NULL);
    if (threadpool_tcp_rst==NULL)
    {
        printf("Create thread_rst_tcp error!\n");
        return -1;
    }
    else printf("Create thread_rst_tcp success\n");
//
//
    //udp pcap//
    ret = pthread_create(&thread_udp_pcap, NULL, (void*)udp_pcap, NULL);
    if (ret)
    {
        printf("Create thread_udp_pcap error!\n");
        return -1;
    }
    else printf("Create thread_udp_pcap success\n");
//
    g_thread_init(NULL);
    //udp process//
    threadpool_udp_process=g_thread_pool_new(udp_process, NULL, 5, TRUE, NULL);
    if (threadpool_udp_process==NULL)
    {
        printf("Create thread_udp_process error!\n");
        return -1;
    }
    else printf("Create thread_udp_process success\n");

    g_thread_init(NULL);
    //udp block ptread pool. by arp spoofind to a fake gateway for nat and ipfilter//
    threadpool_udp_block=g_thread_pool_new(udp_block, NULL, 5, TRUE, NULL);
    if (threadpool_udp_block==NULL)
    {
        printf("Create threadpool_udp_block error!\n");
        return -1;
    }
    else printf("Create threadpool_udp_block success\n");

//
    /*����normal��audit_pcap�߳�*/
    ret = pthread_create(&thread_audit_pcap, NULL, (void*)audit_pcap, NULL);
    if (ret)
    {
        printf("Create thread_audit_pcap error!\n");
        return -1;
    }
    else printf("Create thread_audit_pcap success\n");


    /*����normal��ait�߳�*/
    ret = pthread_create(&thread_spk_audit, NULL, (void*)spk_audit, NULL);
    if (ret)
    {
        printf("Create thread_normal_audit error!\n");
        return -1;
    }
    else printf("Create thread_normal_audit success\n");




    //������ǰ���̣߳�ֱ������һ���߳����н��� ��
    pthread_join(thread_http_pcap,NULL);

    pthread_join(thread_spk_pcap,NULL);

    pthread_join(thread_udp_pcap,NULL);

    pthread_join(thread_audit_pcap,NULL);

    pthread_join(thread_spk_audit,NULL);



    return 1;
}

 /* single processing function */
    void sigproc( int sig )
    {
        g_thread_pool_free(threadpool_http_process, 0, 1);
        g_thread_pool_free(threadpool_tcp_rst, 0, 1);

        g_thread_pool_free(threadpool_spk_process, 0, 1);
        g_thread_pool_free(threadpool_tcp_deepprocess, 0, 1);

        g_thread_pool_free(threadpool_udp_process,0,1);
        g_thread_pool_free(threadpool_udp_block,0,1);

        pthread_cancel( thread_http_pcap );
        pthread_cancel( thread_spk_pcap );
        pthread_cancel( thread_udp_pcap );
        pthread_cancel( thread_audit_pcap );
        pthread_cancel( thread_spk_audit );

        //pthread_mutex_destroy() ɾ��������
        pthread_mutex_destroy(mutex_for_bpf);

        pthread_mutex_destroy(mutex_for_cb_http_a);

        pthread_mutex_destroy(mutex_for_cb_pk_a);

        pthread_mutex_destroy(mutex_for_cb_tcp_a);

        pthread_mutex_destroy(mutex_for_cb_rst);

        pthread_mutex_destroy(mutex_for_cb_udp_a);
        pthread_mutex_destroy(mutex_for_cb_udp_b);

        pthread_mutex_destroy(mutex_for_cb_spk_audit_a); // ����ͬ��  ������a���߳� ������߳�//


        //�ͷ�ѭ������//
        free_ringbuff_queue(cb_http_a);

        free_ringbuff_queue(cb_pk_a);

        free_ringbuff_queue(cb_tcp_a);
        free_ringbuff_queue(cb_tcp_rst);

        free_ringbuff_queue(cb_udp_a);
        free_ringbuff_queue(cb_udp_b);
        free_ringbuff_queue(cb_spk_aduit_a);

        //�ͷ����ݿ�?//
        mysql_close(&mysql);

        //������������
        pthread_cond_destroy(notUse_bpf);

        pthread_cond_destroy(notFull_http_a);
        pthread_cond_destroy(notEmpty_http_a);

        pthread_cond_destroy(notFull_pk_a);
        pthread_cond_destroy(notEmpty_pk_a);

        pthread_cond_destroy(notFull_tcp_a);
        pthread_cond_destroy(notEmpty_tcp_a);

        pthread_cond_destroy(notFull_tcpcb_rst);
        pthread_cond_destroy(notEmpty_tcpcb_rst);

        pthread_cond_destroy(notFull_udp_a);
        pthread_cond_destroy(notEmpty_udp_a);
        pthread_cond_destroy(notFull_udp_b);
        pthread_cond_destroy(notEmpty_udp_b);

        pthread_cond_destroy(notFull_spk_audit_a);
        pthread_cond_destroy(notEmpty_spk_audit_a);

        int k;
        for(k=0; k<=1000; k++)
        {
            pcre_free(tcpPcre[k]);
            pcre_free(udpPcre[k]);

        }

        //�ͷŹ�ϣ��//
        g_hash_table_destroy(http_hash_white_list);
        g_hash_table_destroy(http_hash_person_list);
        g_hash_table_destroy(http_hash_group_list);
        g_hash_table_destroy(http_hash_whole_list);
//
        //user//
        g_hash_table_destroy(user_to_gid_hash_list);
        g_hash_table_destroy(user_to_uid_hash_list);
        g_hash_table_destroy(user_to_name_hash_list);
//
        //pattern//
        g_hash_table_destroy(app_hash_tcp_pattern_list);
        g_hash_table_destroy(app_hash_udp_pattern_list);

        //app//
        g_hash_table_destroy(app_tcp_hash_IPbuff_list);
        g_hash_table_destroy(app_udp_hash_IPbuff_list);




        printf("exit program. \n");

        sleep(3);
        exit(0);
    }


