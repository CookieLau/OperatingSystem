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
void sched_yield(void)
{
	static int count = 0;//using static since we search after last time in circular
	static struct Env *currentEnv = NULL;
	while (count == 0 || currentEnv == NULL || currentEnv->env_status != ENV_RUNNABLE) {
		if (currentEnv != NULL) {
			LIST_REMOVE(currentEnv,env_sched_link);
			LIST_INSERT_HEAD(&env_sched_list[sched_list_id^1],currentEnv,env_sched_link);
		}
		if (LIST_EMPTY(&env_sched_list[sched_list_id])) {
			sched_list_id ^= 1;
		}
		currentEnv = LIST_FIRST(&env_sched_list[sched_list_id]);
		count = currentEnv->env_pri;
	}
	if (currentEnv->env_status == ENV_RUNNABLE) {
		count --;
		env_run(currentEnv);
	} else {
		count = 0;
		currentEnv = NULL;
		sched_yield();
	}
	return;
}
