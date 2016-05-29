#pragma once
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Windows.h>

typedef struct pcb_struct 
{
	int pid;   /* ���� ID */ 
	char name[20]; /* ������ */
	int state;   /* ����״̬ */ 
	int priority;  /* �������ȼ� */
	int tArriving;  /* ����ʱ�� */ 
	int tOverall;  /* ������ʱ�� */ 
	int tRunnng;  /* ������ʱ�� */
} pcb_t;

typedef struct pcb_queue_node_struct
{
	struct pcb_queue_node_struct* next;
	pcb_t* pcb;
} pcb_queue_node_t;

typedef struct scheduler_rr_struct
{
	pcb_queue_node_t* head;
} scheduler_rr_t;

typedef struct scheduler_spf_struct
{
	pcb_t* ctask;
	pcb_queue_node_t* head;
} scheduler_spf_t;

void pcb_init(pcb_t* ppcb);
void node_init(pcb_queue_node_t* pnode);

void task_run(pcb_t* ppcb, int time);

void rr_init(scheduler_rr_t* pr);
void rr_add_task(scheduler_rr_t* pr, pcb_t* ppcb, int time);
void rr_run(scheduler_rr_t* pr, int time);

void spf_init(scheduler_spf_t* ps);
void spf_add_task(scheduler_spf_t* ps, pcb_t* ppcb, int time);
void spf_run(scheduler_spf_t* ps, int time);

#endif
