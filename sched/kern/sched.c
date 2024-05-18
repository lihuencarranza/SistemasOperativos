#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

void sched_halt(void);

#ifdef SCHED_PRIORITIES
	void reduce_priority(struct Env *env){
		if(env->env_priority>MIN_PRIORITY_LEVEL){
			env->env_priority--;
		}
	}

	void print_first_3(){
		for (int i = 0; i < NENV; i++) {
			if (envs[i].env_status == ENV_RUNNABLE) {
				// cprintf("envs[%d].env_priority = %d\n", envs[i].env_id,envs[i].env_priority);
			}
		}
	}

	void boost_priority(){
		// cprintf("Boosting priority\n");
		for (int i = 0; i < NENV; i++) {
			if (envs[i].env_status == ENV_RUNNABLE) {
				envs[i].env_priority = MAX_PRIORITY_LEVEL;
			}
		}
	}

#endif


unsigned times_scheduler_called = 0;

void
sched_yield(void)
{
	struct Env *idle = NULL;
	times_scheduler_called++;
#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.

	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.
	// cprintf("Running round robin scheduler\n");
	// Your code here - Round robin
	if (!curenv) {
		int i = 0;
		while (i < NENV && envs[i].env_status != ENV_RUNNABLE) {
			i++;
		}
		if (envs[i].env_status == ENV_RUNNABLE)
			idle = &envs[i];
	} else {
		int i = ENVX(curenv->env_id) + 1;
		while (curenv != &envs[i]) {
			if (i >= NENV) {
				i = 0;
			} else {
				if (envs[i].env_status == ENV_RUNNABLE) {
					idle = &envs[i];
					break;
				}
				i++;
			}
		}
	}
	if (idle) {
		env_run(idle);
	} else if (curenv && curenv->env_status == ENV_RUNNING &&
	           curenv->env_cpunum == cpunum()) {
		env_run(curenv);
	}
#endif


#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	// cprintf("Times scheduler called: %d\n",times_scheduler_called);
	if (times_scheduler_called%30==0)
		boost_priority();
	//  print_first_3();
	struct Env *highest_priority = NULL;
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			if (highest_priority == NULL || envs[i].env_priority > highest_priority->env_priority) 
				highest_priority = &envs[i];
		}
	}

	if(highest_priority && highest_priority->env_runs!=0){
		reduce_priority(highest_priority);
	}

	// Run the highest priority environment
	if (highest_priority) {
		env_run(highest_priority);
	}
	// If there is no runnable environment, run the current environment
	if (curenv && curenv->env_status == ENV_RUNNING && curenv->env_cpunum == cpunum()) {
		env_run(curenv);
	}

	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities
#endif

	// Without scheduler, keep runing the last environment while it exists
	// if (curenv) {
	// 	env_run(curenv);
	// }

	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
