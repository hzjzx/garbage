#include "scheduler.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void idle(int time)
{
	printf("nTime = %d,    idle\n", time);
}

static pcb_queue_node_t* get_tail(pcb_queue_node_t* head)
{
	pcb_queue_node_t* pn = head;
	while (pn->next)
	{
		pn = pn->next;
	}
	return pn;
}

void pcb_init(pcb_t* ppcb)
{
	memset(ppcb, 0, sizeof(pcb_t));
}

void node_init(pcb_queue_node_t* pnode)
{
	memset(pnode, 0, sizeof(pcb_queue_node_t));
}

void task_run(pcb_t* ppcb, int time)
{
	ppcb->tRunnng--;
	printf("nTime = %d,    pid = %d\n", time, ppcb->pid);
}

void rr_init(scheduler_rr_t * pr)
{
	pr->head = NULL;
}

void rr_add_task(scheduler_rr_t* pr, pcb_t* ppcb, int time)
{
	pcb_queue_node_t* tail = NULL;

	if (pr->head)
	{
		tail = get_tail(pr->head);
		tail->next = (pcb_queue_node_t*)malloc(sizeof(pcb_queue_node_t));
		tail = tail->next;
	}
	else
	{
		pr->head = (pcb_queue_node_t*)malloc(sizeof(pcb_queue_node_t));
		tail = pr->head;
	}
	tail->next = NULL;
	tail->pcb = ppcb;
	tail->pcb->tArriving = time;
}

void rr_run(scheduler_rr_t* pr, int time)
{
	if (pr->head)
	{
		task_run(pr->head->pcb, time);
		if (pr->head->pcb->tRunnng == 0)
		{
			pcb_queue_node_t* tmp = pr->head;
			pr->head = pr->head->next;
			free(tmp->pcb);
			free(tmp);
		}
		else if (pr->head->next)
		{
			pcb_queue_node_t* tmp = pr->head;
			pcb_queue_node_t* tail = get_tail(pr->head);
			pr->head = pr->head->next;
			tail->next = tmp;
			tmp->next = NULL;
		}
	}
	else
	{
		idle(time);
	}
}

void spf_init(scheduler_spf_t* ps)
{
	ps->ctask = NULL;
	ps->head = NULL;
}

void spf_add_task(scheduler_spf_t* ps, pcb_t* ppcb, int time)
{
	pcb_queue_node_t* pn = ps->head;
	if (ps->head && (ps->head->pcb->tOverall > ppcb->tOverall))
	{
		pcb_queue_node_t* tmp = (pcb_queue_node_t*)malloc(sizeof(pcb_queue_node_t));
		tmp->next = pn;
		tmp->pcb = ppcb;
		ps->head = tmp;
		return;
	}

	while (pn)
	{
		if ((pn->next == NULL) || (pn->next->pcb->tOverall > ppcb->tOverall))
		{
			pcb_queue_node_t* tmp = (pcb_queue_node_t*)malloc(sizeof(pcb_queue_node_t));
			tmp->next = pn->next;
			tmp->pcb = ppcb;
			pn->next = tmp;
			return;
		}
		pn = pn->next;
	}

	ps->head = (pcb_queue_node_t*)malloc(sizeof(pcb_queue_node_t));
	ps->head->next = NULL;
	ps->head->pcb = ppcb;
}

void spf_run(scheduler_spf_t* ps, int time)
{
	if (!ps->ctask)
	{
		if (ps->head)
		{
			pcb_queue_node_t* tmp = ps->head;
			ps->ctask = ps->head->pcb;
			ps->head = ps->head->next;
			free(tmp);
		}
		else
		{
			idle(time);
			return;
		}
	}
	task_run(ps->ctask, time);
	if (ps->ctask->tRunnng == 0)
	{
		free(ps->ctask);
		ps->ctask = NULL;
	}
}
