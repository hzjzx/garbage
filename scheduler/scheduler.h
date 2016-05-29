#pragma once
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Windows.h>

typedef struct pcb_struct 
{
	int pid;   /* 进程 ID */ 
	char name[20]; /* 进程名 */
	int state;   /* 进程状态 */ 
	int priority;  /* 进程优先级 */
	int tArriving;  /* 到达时间 */ 
	int tOverall;  /* 运行总时间 */ 
	int tRunnng;  /* 已运行时间 */
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
