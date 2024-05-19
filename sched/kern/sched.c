#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

void sched_halt(void);

// inicializar typedef struct stats
struct Stats stats = { 0 };

#ifdef SCHED_PRIORITIES
void
reduce_priority(struct Env *env)
{
	if (env->env_priority > MIN_PRIORITY_LEVEL) {
		env->env_priority--;
	}
}


void
boost_priority()
{
	stats.total_boosts++;
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			envs[i].env_priority = MAX_PRIORITY_LEVEL;
			envs[i].env_boosts++;
		}
	}
}

struct Env *
sched_priorities(void)
{
	struct Env *highest_priority = NULL;

	if (stats.sched_calls % 30 == 0)
		boost_priority();

	// Find the highest priority environment
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			if (highest_priority == NULL ||
			    envs[i].env_priority > highest_priority->env_priority) {
				highest_priority = &envs[i];
			}
		}
	}

	if (highest_priority && highest_priority->env_runs != 0) {
		reduce_priority(highest_priority);
	}

	return highest_priority;
}
#endif

#ifdef SCHED_ROUND_ROBIN
struct Env *
sched_round_robin(void)
{
	struct Env *idle = NULL;

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

	return idle;
}
#endif


void
sched_yield(void)
{
	struct Env *idle = NULL;
	stats.sched_calls++;

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
	// Your code here - Round robin

	idle = sched_round_robin();
#endif


#ifdef SCHED_PRIORITIES

	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.

	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities
	idle = sched_priorities();
#endif

	// Without scheduler, keep runing the last environment while it exists
	if (idle) {
		stats.pid_history[stats.history_index % MAX_HISTORY] =
		        idle->env_id;
		stats.history_index++;
		env_run(idle);
	} else if (curenv && curenv->env_status == ENV_RUNNING &&
	           curenv->env_cpunum == cpunum()) {
		env_run(curenv);
	}

	// sched_halt never returns
	sched_halt();
}

void
print_stats(void)
{
	cprintf("\n=== SCHEDULER STATS ===\n");

	cprintf("Times scheduler called: %d\n", stats.sched_calls);
	cprintf("Total of processes: %d\n", NENV);

	cprintf("\n=== PID STATS ===\n");
	int not_run = 0;
	int runned = 0;
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_runs > 0) {
			cprintf("Process %d executed %d times\n",
			        i,
			        envs[i].env_runs);
			runned++;
		} else {
			not_run++;
		}
	}
	cprintf("Processes executed: %d\n", runned);
	cprintf("Processes not executed: %d\n", not_run);

#ifdef SCHED_PRIORITIES
	cprintf("\n=== BOOSTS STATS ===\n");
	cprintf("Total boosts %d\n", stats.total_boosts);
	cprintf("\n");
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_boosts > 0)
			cprintf("Process %d has %d boosts\n",
			        envs[i].env_parent_id,
			        envs[i].env_boosts);
	}
#endif

	cprintf("\n=== ENV STATS ===\n");
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

		print_stats();

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
