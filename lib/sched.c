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
	if (count > 0 & currentEnv != NULL) {
		count --;
		env_run(currentEnv);
		return ;
	} else {
		if (LIST_EMPTY(&env_sched_list[sched_list_id])) {//the first time in
			sched_list_id ^= 1;
		} else {//has begun
			struct Env *ee = LIST_FIRST(&env_sched_list[sched_list_id]);
			LIST_REMOVE(ee,env_sched_link);
			LIST_INSERT_HEAD(&env_sched_list[sched_list_id^1],ee,env_sched_link);
			if (LIST_EMPTY(&env_sched_list[sched_list_id])) {//check if it is run out
				sched_list_id ^= 1;
			}
		}
		if (LIST_EMPTY(&env_sched_list[sched_list_id])) {
			printf("fuckoff two env_sched_links are both empty???\n");
		}
		currentEnv = LIST_FIRST(&env_sched_list[sched_list_id]);
		count = LIST_FIRST(&env_sched_list[sched_list_id])->env_pri;
		count --;
		env_run(currentEnv);
	}
	return;
}
