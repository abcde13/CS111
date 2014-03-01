#include "schedos-kern.h"
#include "x86.h"
#include "lib.h"

/*****************************************************************************
 * schedos-kern
 *
 *   This is the schedos's kernel.
 *   It sets up process descriptors for the 4 applications, then runs
 *   them in some schedule.
 *
 *****************************************************************************/

// The program loader loads 4 processes, starting at PROC1_START, allocating
// 1 MB to each process.
// Each process's stack grows down from the top of its memory space.
// (But note that SchedOS processes, like MiniprocOS processes, are not fully
// isolated: any process could modify any part of memory.)

#define NPROCS		5
#define PROC1_START	0x200000
#define PROC_SIZE	0x100000

// +---------+-----------------------+--------+---------------------+---------/
// | Base    | Kernel         Kernel | Shared | App 0         App 0 | App 1
// | Memory  | Code + Data     Stack | Data   | Code + Data   Stack | Code ...
// +---------+-----------------------+--------+---------------------+---------/
// 0x0    0x100000               0x198000 0x200000              0x300000
//
// The program loader puts each application's starting instruction pointer
// at the very top of its stack.
//
// System-wide global variables shared among the kernel and the four
// applications are stored in memory from 0x198000 to 0x200000.  Currently
// there is just one variable there, 'cursorpos', which occupies the four
// bytes of memory 0x198000-0x198003.  You can add more variables by defining
// their addresses in schedos-symbols.ld; make sure they do not overlap!


// A process descriptor for each process.
// Note that proc_array[0] is never used.
// The first application process descriptor is proc_array[1].
static process_t proc_array[NPROCS];
static int prop[NPROCS];
int perm[NPROCS];
int lrpid = -1000;
int j =0;
int n = 1;
// A pointer to the currently running process.
// This is kept up to date by the run() function, in mpos-x86.c.
process_t *current;

// The preferred scheduling algorithm.

void createTickets(void);
int scheduling_algorithm;


int ticketsLeft = 100;
//static int ticket_num;
pid_t tickets[NPROCS][2];
int lastTicket = 0;
int winner = -1;


/*****************************************************************************
 * start
 *
 *   Initialize the hardware and process descriptors, then run
 *   the first process.
 *
 *****************************************************************************/

void
start(void)
{
	int i;

	// Set up hardware (schedos-x86.c)
	segments_init();
	interrupt_controller_init(1);
	console_clear();

	// Initialize process descriptors as empty
	memset(proc_array, 0, sizeof(proc_array));
	for (i = 0; i < NPROCS; i++) {
		proc_array[i].p_pid = i;
		proc_array[i].p_state = P_EMPTY;
		//Priorities from  -7 -> 7
		proc_array[i].p_priority = i-7;
		prop[i] = i;
		perm[i] = i;
		if(i == 0){
			tickets[i][0] = -2;
			tickets[i][1] = -2;
		} else {
			tickets[i][0] = -1;
			tickets[i][1] = -1;
		}
	}
	proc_array[1].p_priority = 4;
	proc_array[2].p_priority = 4;
	proc_array[3].p_priority = 1;
	proc_array[4].p_priority = 3;

	// Set up process descriptors (the proc_array[])
	for (i = 1; i < NPROCS; i++) {
		process_t *proc = &proc_array[i];
		uint32_t stack_ptr = PROC1_START + i * PROC_SIZE;

		// Initialize the process descriptor
		special_registers_init(proc);

		// Set ESP
		proc->p_registers.reg_esp = stack_ptr;

		// Load process and set EIP, based on ELF image
		program_loader(i - 1, &proc->p_registers.reg_eip);

		// Mark the process as runnable!
		proc->p_state = P_RUNNABLE;
	}

	// Initialize the cursor-position shared variable to point to the
	// console's first character (the upper left).
	cursorpos = (uint16_t *) 0xB8000;

	// Initialize the scheduling algorithm.
	scheduling_algorithm = 2;
	createTickets();

	// Switch to the first process.
	for(i = 0; i < NPROCS;i++){
		if(proc_array[i].p_state == P_RUNNABLE && (lrpid == -1000 ||  proc_array[i].p_priority < proc_array[lrpid].p_priority)){
			lrpid = i;
		}
	}
	run(&proc_array[1]);

	// Should never get here!
	while (1)
		/* do nothing */;
}

int pickWinner(void);

void
createTickets(void){
	int i = 0;
	int give = 0;
	while(i!=NPROCS-2){
		give = (int)(Random() * 10000) % (NPROCS-1);
		give++;
//		cursorpos = console_printf(cursorpos, 0x100, "give: %d \n", give);
		if(tickets[give][0] == -1){
			int low = (int)(Random() * 10000) % ticketsLeft;
			int high = (int)(Random() * 100000) % ticketsLeft;
//			cursorpos = console_printf(cursorpos, 0x100, "1st: %d 2nd: %d \n",low,high);
			if(low < high){
				tickets[give][0] = lastTicket;
				tickets[give][1] = lastTicket + (high-low);
				lastTicket += high-low+1;
				ticketsLeft -=  (high-low)+1;
			} else {
				tickets[give][0] = lastTicket;
				tickets[give][1] = lastTicket + (low-high);
				lastTicket += low-high+1;
				ticketsLeft -= (low-high)+1;
			}
//			cursorpos = console_printf(cursorpos, 0x100, "assigned %d with tickets %d -> %d",give,tickets[give][0],tickets[give][1]);
//			cursorpos = console_printf(cursorpos, 0x100, "ticketsLeft: %d \n",ticketsLeft);
			i++;
		}
	}
	for(i = 0; i < NPROCS; i++){
		if(tickets[i][0] == -1){
//			cursorpos = console_printf(cursorpos, 0x100, "filling ticket %d with tickets %d -> %d \n",i,lastTicket, lastTicket+ticketsLeft-1);
			tickets[i][0] = lastTicket;
			tickets[i][1] = lastTicket + ticketsLeft-1;
			break;
		}
	}
	for(i = 0; i < NPROCS; i++){
		//cursorpos = console_printf(cursorpos, 0x100, "bucket %d: %d %d ", i, tickets[i][0],tickets[i][1]);
	}
	ticketsLeft = 100;
	//cursorpos = console_printf(cursorpos, 0x100, "ticketsleft: %d \n",ticketsLeft);
	lastTicket = 0;
//	cursorpos = console_printf(cursorpos, 0x100, "\nwinner: %d", win);
	
}


/*****************************************************************************
 * interrupt
 *
 *   This is the weensy interrupt and system call handler.
 *   The current handler handles 4 different system calls (two of which
 *   do nothing), plus the clock interrupt.
 *
 *   Note that we will never receive clock interrupts while in the kernel.
 *
 *****************************************************************************/

void
interrupt(registers_t *reg)
{
	// Save the current process's register state
	// into its process descriptor
	current->p_registers = *reg;

	switch (reg->reg_intno) {

	case INT_SYS_YIELD:
		// The 'sys_yield' system call asks the kernel to schedule
		// the next process.
		schedule();

	case INT_SYS_EXIT:
		// 'sys_exit' exits the current process: it is marked as
		// non-runnable.
		// The application stored its exit status in the %eax register
		// before calling the system call.  The %eax register has
		// changed by now, but we can read the application's value
		// out of the 'reg' argument.
		// (This shows you how to transfer arguments to system calls!)
		current->p_state = P_ZOMBIE;
		current->p_exit_status = reg->reg_eax;
		schedule();

	case INT_SYS_PRIORITY:
	
		proc_array[current->p_pid].p_priority = reg->reg_eax;
		run(current);	
		break;
	case INT_SYS_USER1:
		// 'sys_user*' are provided for your convenience, in case you
		// want to add a system call.
		/* Your code here (if you want). */
		prop[current->p_pid] = reg->reg_eax;
		perm[current->p_pid] = reg->reg_eax;
		run(current);
		break;
	/*case INT_SYS_GETTICKET:
		if(ticket_num<MAX_TICKET_NUM){
			tickets[ticket_num] = current->p_pid;
			ticket_num++;
		}
		run(current);*/
	case INT_SYS_USER2:
		/* Your code here (if you want). */
		
		*cursorpos = reg->reg_eax;
		*cursorpos++;
		//cursorpos = console_printf(cursorpos, 0x100, "\nUnknown scheduling algorithm %d\n", scheduling_algorithm);
		run(current);

	case INT_CLOCK:
		// A clock interrupt occurred (so an application exhausted its
		// time quantum).
		// Switch to the next runnable process.
		schedule();

	default:
		while (1)
			/* do nothing */;

	}
}



/*****************************************************************************
 * schedule
 *
 *   This is the weensy process scheduler.
 *   It picks a runnable process, then context-switches to that process.
 *   If there are no runnable processes, it spins forever.
 *
 *   This function implements multiple scheduling algorithms, depending on
 *   the value of 'scheduling_algorithm'.  We've provided one; in the problem
 *   set you will provide at least one more.
 *
 *****************************************************************************/
int
pickWinner(void){
	while(1){	
		int r = Random()*10000;
		int winner = r%ticketsLeft;
		int i = 0;
		for(i = 0; i < NPROCS; i++){
			if(tickets[i][0] <= winner && tickets[i][1] >= winner){
					return i;
			} else {
		//		cursorpos = console_printf(cursorpos, 0x100, "\n winner %d",winner);
			}
		}
	}
	return -1;
}
void
schedule(void)
{
	pid_t pid = current->p_pid;
	//uint32_t priority = current->p_priority;

	if (scheduling_algorithm == 0)
		while (1) {
			pid = (pid + 1) % NPROCS;

			// Run the selected process, but skip
			// non-runnable processes.
			// Note that the 'run' function does not return.
			if (proc_array[pid].p_state == P_RUNNABLE)
				run(&proc_array[pid]);
		}
	else if (scheduling_algorithm == 1){
		while(1) {
			for(pid = 0; pid < NPROCS; pid++){
				if(proc_array[pid].p_state == P_RUNNABLE)
					run(&proc_array[pid]);
			}
		}
	}
		
	else if (scheduling_algorithm == 2){
		while (1) {
			pid = (pid + 1) % NPROCS;
			int i = 0;
			int toRun = 100;
			for(i = pid; i < NPROCS;i++){
				if(proc_array[i].p_state == P_RUNNABLE &&  proc_array[i].p_priority <= proc_array[lrpid].p_priority){
					toRun = i;
					break;
				}
			}
			if(toRun != 100){
				lrpid = toRun;
				run(&proc_array[toRun]);
			} else {
				for(i = 0; i < NPROCS;i++){
					if(proc_array[i].p_state == P_RUNNABLE && (toRun == 100 ||  proc_array[i].p_priority < proc_array[toRun].p_priority)){
						toRun = i;
					}
				}
				lrpid = toRun;
				run(&proc_array[toRun]);
			}
		}
	} else if (scheduling_algorithm == 3) {
		while (1) {
			pid = (pid + 1) % NPROCS;

			// Run the selected process, but skip
			// non-runnable processes.
			// Note that the 'run' function does not return.
			if(prop[NPROCS-1] == 0){
				int i = 0;
				for(;i<NPROCS;i++){
					prop[i] = perm[i];
				}
			}
			if (proc_array[pid].p_state == P_RUNNABLE && prop[pid] > 0) {
				prop[pid]--;
				run(&proc_array[pid]);
			} else if (proc_array[pid].p_state != P_RUNNABLE && prop[pid] > 0) {
				prop[pid]--;
			}
		}
	} else if (scheduling_algorithm == 4){
		while(1){
			winner = pickWinner();
			/*for(;x<NPROCS && n!=100;x++){
				if(tickets[n][0] == -1){
					n++;
				}
				if(n == 4 && x == NPROCS-1){
					n = 100;
					break;
				}
			}
			if(n == 100){
				break;
			}*/
//			cursorpos = console_printf(cursorpos, 0x100, " winner %d",winner);
			//if(proc_array[winner].p_state == P_RUNNABLE){
			if(proc_array[winner].p_state == P_RUNNABLE){
				/*ticketsLeft = ticketsLeft- 1;
				int temp = tickets[winner][1];
				if(tickets[winner][1] == tickets[winner][0]){
					tickets[winner][0] = -1;
					tickets[winner][1] = -1;
					cursorpos = console_printf(cursorpos, 0x100, "\n %d decremented to %d ",winner,tickets[winner][1]);
				} else {
					tickets[winner][1]--;
					cursorpos = console_printf(cursorpos, 0x100, "\n %d decremented to %d ",winner,tickets[winner][1]);
					
				}
				int i =0;
				for(i = 1; i< NPROCS; i++){
					if(tickets[i][0]>temp){
						tickets[i][0] = tickets[i][0] - 1;
						tickets[i][1] = tickets[i][1] - 1;
					}
				}
				for(i = 1; i< NPROCS; i++){
					cursorpos = console_printf(cursorpos, 0x100, "\n bucket %d: %d->%d ",i,tickets[i][0],tickets[i][1]);
				}
				cursorpos = console_printf(cursorpos, 0x100, "\n tickets left: %d ",ticketsLeft);
				j++;*/
/*				int x = 1;
				for(;x<NPROCS;x++){
					tickets[x][0] = -1;
					tickets[x][1] = -1;
				}
				createTickets();
				j++;*/
				run(&proc_array[winner]);
			} 
			
				
//				;
			//}
		}
			
		
	/*	pid_t winner;
		if(ticket_num < NPROCS-1){
			while (1) {
				pid = (pid + 1) % NPROCS;
				// Run the selected process, but skip
				// non-runnable processes.
				// Note that the 'run' function does not return.
				if (proc_array[pid].p_state == P_RUNNABLE)
					run(&proc_array[pid]);
			}
		}
		else{
			winner = get_winner();
			while(proc_array[winner].p_state != P_RUNNABLE){
				winner = get_winner();
			}
		}
		run(&proc_array[winner]);*/
	} 
				
			

	// If we get here, we are running an unknown scheduling algorithm.
	//sys_print_char;
	cursorpos = console_printf(cursorpos, 0x100, "\nUnknown scheduling algorithm %d\n", scheduling_algorithm);
	while (1)
		/* do nothing */;
}
