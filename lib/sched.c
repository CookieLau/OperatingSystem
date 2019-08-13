#include <env.h>
#include <pmap.h>
#include <printf.h>

/* Overview:
 *  Implement simple round-robin scheduling.
 *  Search through 'envs' for a runnable environment ,
 *  in circular fashion statrting after the previously running env,
 *  and switch to the first such environment found.
 *
 * Hints:
 *  The variable which is for counting should be defined as 'static'.
 */
/*void sched_yield(void)
{
	static int count = 0;//using static since we search after last time in circular
	static struct Env *currentEnv = NULL;
	while (currentEnv == NULL || count == 0 || currentEnv->env_status != ENV_RUNNABLE) {
		if (currentEnv != NULL) {
			if (LIST_FIRST(&env_sched_list[sched_list_id]) == currentEnv) {
				LIST_REMOVE(currentEnv,env_sched_link);
				LIST_INSERT_HEAD(&env_sched_list[sched_list_id^1],currentEnv,env_sched_link);
			}
		}
		if (LIST_EMPTY(&env_sched_list[sched_list_id])) {
			sched_list_id ^= 1;
		}
		currentEnv = LIST_FIRST(&env_sched_list[sched_list_id]);
		if (currentEnv != NULL)
			count = currentEnv->env_pri;
	}
	if (currentEnv != NULL && currentEnv->env_status == ENV_RUNNABLE) {
		count --;
//		printf("now run envid:%d\n",currentEnv->env_id);
		env_run(currentEnv);
	} else {
		count = 0;
		currentEnv = NULL;
		sched_yield();
	}
	return;
}*/

void sched_yield(void) {
	int i;
	static int times = 0;
	static struct Env* e;
	while (!(times > 0 && e != NULL && e->env_status == ENV_RUNNABLE)) {
		do{
			if (LIST_EMPTY(&env_sched_list[sched_list_id])) {
				sched_list_id ^= 1;
			}
			e = LIST_FIRST(&env_sched_list[sched_list_id]);
			if (e) {
				LIST_REMOVE(e,env_sched_link);
				LIST_INSERT_HEAD(&env_sched_list[sched_list_id^1],e,env_sched_link);
				times = e->env_pri;
			}
		} while (e == NULL || e->env_status != ENV_RUNNABLE || times <= 0);
	}
	times--;
	env_run(e);
}
