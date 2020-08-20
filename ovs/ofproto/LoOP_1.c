#include "LoOP.h"
#include <math.h>

float distace(data_t *a,data_t *b)
{
    return (sqrt(pow((a->n_flow_norm - b->n_flow_norm),2)+pow((a->traffic_norm - b->traffic_norm),2)));
}

void swap(nei_bor *a, nei_bor *b)
{
    nei_bor t;
    t = *a;
    *a = *b;
    *b = t;
}

void bubble_sort_k(data_t *p,int n,int k)
{
    for (int i=0;i<k;i++)
    {
        for (int j=i+1;j<n;j++)
        {
            if(p->neighbor[i].distance > p->neighbor[j].distance)
            {
                swap(&p->neighbor[i],&p->neighbor[j]);
            }
        }
    }
}
void make_NN_train(data_t  *Arr[],int n)
{
    /////add neighbor for each of point data
    for (int i=0;i<n;i++)
        Arr[i]->id = i;
    for (int i=0;i<n;i++)
    {
        int k=0;
         for (int j=0;j<n;j++)
            {
                if(i != j)
                {
                    Arr[i]->neighbor[k].ind_list = j;
                    Arr[i]->neighbor[k].distance = distace(Arr[i],Arr[j]);
                    k++;
                }
            }
    }
}
void make_NN_udate(data_t *p,data_t *Arr[],int n)
{
    for (int i=0;i<n;i++)
    {
        Arr[i]->id = i;
    }
        for (int j=0;j<n;j++)
        {
            p->neighbor[j].ind_list = j;
            p->neighbor[j].distance = distace(p,Arr[j]);
        }
}

float stan_dis(data_t *a,data_t *Arr[],int k)
{
    //make_NN_train(Arr,DATANUM);
    bubble_sort_k(a,DATANUM,K);
    float sum=0;
	for (int i = 0; i < K; i++)
	{
		sum += pow(a->neighbor[i].distance, 2);
	}
	float st_dis=0;
	st_dis = sqrt(sum / K);
	return st_dis;
}
float stan_dis_udate(data_t *a,data_t *Arr[],int k)
{
    //make_NN_udate(a,Arr,DATANUM);
    bubble_sort_k(a,DATANUM,K);
    float sum=0;
	for (int i = 0; i < K; i++)
	{
		sum += pow(a->neighbor[i].distance, 2);
	}
	float st_dis=0;
	st_dis = sqrt(sum / K);
	return st_dis;
}
float p_dis(data_t *a,data_t *Arr[])
{
    float pdist;
	pdist = lamda * stan_dis(a,Arr,K);
	return pdist;  
}
float p_dis_udate(data_t *a,data_t *Arr[])
{
    float pdist;
	pdist = lamda * stan_dis_udate(a,Arr,K);
	return pdist;  
}
// everage pro_dis neighbor of o
float ev_pdist(data_t *Arr[], data_t *a)
{
    
    float ev=0;
    float sum = 0;
	for (int i = 0; i < K; i++)
	{
		sum += a->neighbor[i].distance;
	}
	for (int i = 0; i < K; i++)
	{
		ev += (a->neighbor[i].distance/sum) * p_dis(Arr[a->neighbor[i].ind_list], Arr);
	}
	return ev;
}
float ev_pdist_udate(data_t *Arr[], data_t *a)
{
    float ev=0;
    float sum = 0;
	for (int i = 0; i < K; i++)
	{
		sum += a->neighbor[i].distance;
	}
	for (int i = 0; i < K; i++)
	{
		ev += (a->neighbor[i].distance/sum) * p_dis_udate(Arr[a->neighbor[i].ind_list], Arr);
	}
	return ev;
}
//Arr mang tat ca cac diem dua vao
float PLOF(data_t*p,data_t  *Arr[],int n)
{
    ////////////////////////////////////
        float plof=0;
        float p_d = p_dis(p, Arr);
	    float p_d1 = ev_pdist(Arr, p);
	    plof += (p_d / p_d1) - 1;
	    return plof;
}
float PLOF_udate(data_t*p,data_t  *Arr[],int n)
{
    float plof=0;
    float p_d = p_dis_udate(p, Arr);
	float p_d1 = ev_pdist_udate(Arr, p);
	plof += (p_d / p_d1) - 1;
	return plof; 
}
float nPLOF(data_t *Arr[], data_t *p)
{
    //make_NN_train(Arr,DATANUM);
    bubble_sort_k(p,DATANUM,K);
    float nplof = 0;
    for (int i=0;i<K;i++)
    {
        bubble_sort_k(Arr[p->neighbor[i].ind_list],DATANUM,K);
    }
   // for (int i=0;i<DATANUM;i++)
    //{
      //  Arr[i]->PLOF = PLOF(Arr[i],Arr,DATANUM);
    //}
    p->PLOF = PLOF(p,Arr,DATANUM);
    nplof = sqrt(pow(p->PLOF, 2));
	nplof = lamda * sqrt(nplof);
	return nplof;
}
float nPLOF_update(data_t *Arr[], data_t *p)
{
    //make_NN_udate(p,Arr,DATANUM);
    bubble_sort_k(p,DATANUM,K);
    float nplof = 0;
    for (int i=0;i<K;i++)
    {
        bubble_sort_k(Arr[p->neighbor[i].ind_list],DATANUM,K);
    }
    //for (int i=0;i<DATANUM;i++)
    //{
      //  Arr[i]->PLOF = PLOF_udate(Arr[i],Arr,DATANUM);
    //}
    p->PLOF = PLOF(p,Arr,DATANUM);
    nplof = sqrt(pow(p->PLOF, 2));
	nplof = lamda * sqrt(nplof);
	return nplof;
}
float max(float a, float b)
{
    if (a >=b)
        return a;
    return b;
}

float LoOP(data_t *p, data_t *Arr[],int n)
{
    //make_NN_train(Arr,n);
    float loop;
	float ERF;
    //float t2 = PLOF(p,Arr,n) ;
	float t1 = nPLOF(Arr, p)*sqrt(2);
	ERF = erf(p->PLOF/ t1);
	loop = max(0,ERF);
	return loop;
}
float LoOP_udate(data_t *p, data_t *Arr[],int n)
{
    float loop;
	float ERF;
	float t1 = nPLOF_update(Arr, p)*sqrt(2);
	ERF = erf(p->PLOF / t1);
	loop = max(0,ERF);
	return loop;
}

//trung binh
float calc_mu(float *arr,int n)
{
    float sum=0;
    for (int i=0;i<n;i++)
        sum+=arr[i];
    return sum/n;
}
// do lech
float calc_sigma(float *arr,int n)
{
    float s=0;
    float mu=calc_mu(arr,n);
    for (int i=0;i<n;i++)
        s+=pow((arr[i]-mu),2);
    return sqrt(s/n);
}
//tanh-estimators
float normalize(float x, float mu,float sigma)
{
    return (tanh(0.1*(x-mu)/sigma)+1)/2;
}
