#ifndef TCP_CBNA_H_INCLUDED
#define TCP_CBNA_H_INCLUDED
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#define mknew(x)	(x *)malloc(sizeof(x))
#define b_comp(x,y)	(!memcmp(&(x), &(y), sizeof(x)))
#define EXP_SEQ (snd->first_data_seq + rcv->count + rcv->urg_count)  //�����õ������к�

//����Libnids״̬
#define NIDS_JUST_EST 1
#define NIDS_DATA 2
#define NIDS_CLOSE 3
#define NIDS_RESET 4
#define NIDS_TIMED_OUT 5
#define NIDS_EXITING   6	//�����ӽ���ʱ���һ�δ�������///* nids is exiting; last chance to get data */
//��ʾ���ӵ��߼�״̬,�ڻص������Ĵ�������зǳ���Ҫ//
//״̬��NIDS_JUST_EST��ʾһ���ոս��������ӣ�NIDS_DATA��ʾ���ӽ��յ��µ����ݣ�
//NIDS_CLOSE��NIDS_RESET ��NIDS_TIMEOUT����ʾ�������Ѿ��ر���

#define FIN_SENT 120
#define FIN_CONFIRMED 121
#define COLLECT_cc 1   //�ͻ���������
#define COLLECT_sc 2   //��������������
#define COLLECT_ccu 4 //�ͻ����н�������
#define COLLECT_scu 8 //���������н�����?

struct tcp_timeout
{
  struct tcp_stream *a_tcp;
  struct timeval timeout;
  struct tcp_timeout *next;
  struct tcp_timeout *prev;
};

//�ص������Ĵ������ڵ�//
struct proc_node {
  void (*item)();
  struct proc_node *next;
};

//���һֱû���������ص�����֮��Ĺ�ϵ���ȷ��Ű�//
struct lurker_node {
  void (*item)();
  void *data;
  char whatto;//����Ȥ��whatto��Ϊ0
  struct lurker_node *next;

};
//���Ĵ�� ��һ��˫������ ������İ����ȷ�������//
struct skbuff {
  struct skbuff *next;
  struct skbuff *prev;

  void *data;
  u_int len;
  u_int truesize;
  u_int urg_ptr;

  char fin;
  char urg;
  u_int seq;
  u_int ack;
};

//��ַ�˿ڽṹ����ȷ��һ��TCP����
struct tuple4
{
  u_short source;
  u_short dest;
  u_int saddr;
  u_int daddr;
};

//TCP������һ�˵�������Ϣ�������Ƿ��������ǿͻ���
struct half_stream
{
  char state;// socket ״̬ (���磺TCP_ESTABLISHED // ���ӵ��߼�״̬

  char collect;//��ʾ�����ݵ�� if >0, ��ô����Ӧ�ñ���ŵ�data�������С�����
// ����������������������

  char collect_urg; // ���Ƶģ��ж��Ƿ�Ϊ��������

  char *data;// �洢�������յ�������
  int offset;// ���浽"data"�������еĵ�һ���ֽڵ�ƫ����
  int count;// �����ӽ��������Ѿ��ж����ֽ��Ѿ����͵�data��������
  int count_new;// �ж����µ����ݽ�Ҫ  �洢��data�У����Ϊ0����ʾû�������ݵ���
  int bufsize;//��������󳤶�
  int rmem_alloc;//��TCP�����������ݵĳ��Ⱥͣ�����list�������

  int urg_count;// if != 0,�µĽ������ݵ���
  u_int acked;
  u_int seq;
  u_int ack_seq;
  u_int first_data_seq;
  u_char urgdata; //�洢��������
  u_char count_new_urg; //�ж��Ƿ��н������ݵ��� �����0����ʾ���µĽ������ݵ���
  u_char urg_seen; //���µ�URG���ݣ�������ǰ���ظ�����
  u_int urg_ptr;//ָ��URG���������е�λ��
  u_short window;//���ڴ�С
  u_char ts_on;
  u_char wscale_on;
  u_int curr_ts;
  u_int wscale;

  int count_gd;

  struct skbuff *list;
  //���յ���TCP����SEQ��������SEQ��ʱ���������ȱ��浽��˫�������У�
  //Ϊ���ʷ��㣬���������е�SEQ��С����������βΪ������SEQ��TCP��
  struct skbuff *listtail;//ptr to tail of list
};

//��ʾһ��TCP���ӵ�������Ϣ
struct tcp_stream
{
  struct tuple4 addr;//tcp���ӵ��ĸ���Ҫ��Ϣ
  char nids_state; //��ʾ���ӵ��߼�״̬,�������Ҫ���ڻص������зǳ���Ҫ//
  //״̬��NIDS_JUST_EST��ʾһ���ոս��������ӣ�NIDS_DATA��ʾ���ӽ��յ��µ����ݣ�
  //NIDS_CLOSE��NIDS_RESET ��NIDS_TIMEOUT����ʾ�������Ѿ��ر���
  struct lurker_node *listeners;  //��TCP����Ļص�����ʹ�ã�������һ��Աָ��ص�����
  struct half_stream client;
  struct half_stream server;
  struct tcp_stream *next_node;
  struct tcp_stream *prev_node;
  int hash_index;
  struct tcp_stream *next_time;
  struct tcp_stream *prev_time;
  int read; //���ζ������ݻ����������ݵĳ���
  struct tcp_stream *next_free;//ָ����һ������tcp�ṹ

  int filelen;
  int filetype;
  int sm_state;
  int tcp_num;
  char *filename;
  int filesize;
  int filestart;
};

int tcp_cbna_init(int size);
void nids_register_tcp(void (*x));
void process_tcp_cbna(const u_char * data, int skblen);
void http_callback (struct tcp_stream *a_tcp, void ** this_time_not_needed);
void nids_discard(struct tcp_stream * a_tcp, int num);
void tcp_check_timeouts(struct timeval *now);
void tcp_exit(void);
void ftp_callback (struct tcp_stream *a_tcp, void ** this_time_not_needed);
void ftp_data_callback (struct tcp_stream *a_tcp, void ** this_time_not_needed);
//
//void http_search_callback(struct tcp_stream *a_tcp, void ** this_time_not_needed);
void http_test_callback(struct tcp_stream *a_tcp, void ** this_time_not_needed);


//void http_test2_callback(struct tcp_stream *a_tcp, void ** this_time_not_needed);

//void scan_init(void);
//void scan_exit(void);
//void detect_scan(struct ip *);
//int audit_init();
//void  audit_desy();
//�������Ļ�û�ӽ���//
#endif // TCP_CBNA_H_INCLUDED
