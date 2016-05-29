#include "scheduler.h"

static void test_rr()
{
	scheduler_rr_t rr;
	rr_init(&rr);
	for (int i = 0;i < 17;i++)
	{
		if (i == 0)
		{
			pcb_t* pcb = (pcb_t*)malloc(sizeof(pcb_t));
			pcb_init(pcb);
			pcb->pid = 1;
			pcb->tOverall = 2;
			pcb->tRunnng = 2;
			rr_add_task(&rr, pcb, i);
		}
		else if (i == 1)
		{
			pcb_t* pcb = (pcb_t*)malloc(sizeof(pcb_t));
			pcb_init(pcb);
			pcb->pid = 2;
			pcb->tOverall = 6;
			pcb->tRunnng = 6;
			rr_add_task(&rr, pcb, i);
		}
		else if (i == 2)
		{
			pcb_t* pcb = (pcb_t*)malloc(sizeof(pcb_t));
			pcb_init(pcb);
			pcb->pid = 3;
			pcb->tOverall = 1;
			pcb->tRunnng = 1;
			rr_add_task(&rr, pcb, i);
		}
		else if (i == 3)
		{
			pcb_t* pcb = (pcb_t*)malloc(sizeof(pcb_t));
			pcb_init(pcb);
			pcb->pid = 4;
			pcb->tOverall = 5;
			pcb->tRunnng = 5;
			rr_add_task(&rr, pcb, i);
		}

		rr_run(&rr, i);
	}
}

static void test_spf()
{
	scheduler_spf_t spf;
	spf_init(&spf);
	for (int i = 0;i < 17;i++)
	{
		if (i == 0)
		{
			pcb_t* pcb = (pcb_t*)malloc(sizeof(pcb_t));
			pcb_init(pcb);
			pcb->pid = 1;
			pcb->tOverall = 2;
			pcb->tRunnng = 2;
			spf_add_task(&spf, pcb, i);
		}
		else if (i == 1)
		{
			pcb_t* pcb = (pcb_t*)malloc(sizeof(pcb_t));
			pcb_init(pcb);
			pcb->pid = 2;
			pcb->tOverall = 6;
			pcb->tRunnng = 6;
			spf_add_task(&spf, pcb, i);
		}
		else if (i == 2)
		{
			pcb_t* pcb = (pcb_t*)malloc(sizeof(pcb_t));
			pcb_init(pcb);
			pcb->pid = 3;
			pcb->tOverall = 1;
			pcb->tRunnng = 1;
			spf_add_task(&spf, pcb, i);
		}
		else if (i == 3)
		{
			pcb_t* pcb = (pcb_t*)malloc(sizeof(pcb_t));
			pcb_init(pcb);
			pcb->pid = 4;
			pcb->tOverall = 5;
			pcb->tRunnng = 5;
			spf_add_task(&spf, pcb, i);
		}

		spf_run(&spf, i);
	}
}

int main()
{
	test_rr();
	test_spf();
	return 0;
}