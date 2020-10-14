#ifndef __LoOP_h__
#define __LoOP_h__

#define DATANUM 300
#define K 6
#define lamda 3

typedef struct neighbor
{
    float distance;
    int ind_list;
}nei_bor;

typedef struct data
{
    int id;
    float traffic_norm;
    float n_flow_norm;
    nei_bor neighbor[DATANUM];
    float PLOF;
    float LoOP;
}data_t;

float distace(data_t *a,data_t *b);
void swap(nei_bor *a, nei_bor *b);
void bubble_sort_k(data_t *p,int n,int k);
float stan_dis(data_t *a, data_t *Arr[],int k);
float stan_dis_udate(data_t *a,data_t *Arr[],int k);
float p_dis(data_t *a,data_t *Arr[]);
float p_dis_udate(data_t *a,data_t *Arr[]);
float ev_pdist(data_t *Arr[], data_t *a);
float ev_pdist_udate(data_t *Arr[], data_t *a);
void make_NN_train(data_t *Arr[],int n);
void make_NN_udate(data_t *p,data_t *Arr[],int n);
float PLOF(data_t*p,data_t  *Arr[],int n);
float PLOF_udate(data_t*p,data_t  *Arr[],int n);
float nPLOF(data_t *Arr[], data_t *p);
float nPLOF_update(data_t *Arr[], data_t *p);
float max(float a, float b);
float LoOP(data_t *p, data_t *Arr[],int n);
float LoOP_udate(data_t *p, data_t *Arr[],int n);
float calc_mu(float *arr,int n);
float calc_sigma(float *arr,int n);
float normalize(float x, float mu,float sigma);

#endif



