#if defined NO_POSIX_1_2001
struct timeval {
	time_t		tv_sec;     /* 秒 */
	suseconds_t	tv_usec;    /* マイクロ秒 */
};
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include <sys/select.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "awk.h"


int plugin_is_GPL_compatible;


enum {
	CAN_READ = 1,
	CAN_WRITE,
	HAS_EXCEPTION,
};


static NODE * do_select(int nags);
static NODE * can_sub(int mode);
static NODE * do_can_read(int nags);
static NODE * do_can_write(int nags);
static NODE * do_has_exception(int nags);
static NODE * do_sleep(int nags);
static NODE * do_usleep(int nags);
NODE * dlload(NODE *tree, void *dl);


static NODE *
do_select(int nags)
{
	NODE *tmp, *array, *elm, *value;
	int nfds;
	fd_set rfds, wfds, efds;
	struct timeval timeout;
	struct timeval *timeout_ptr;
	int retval;
	struct redirect *rp;
	double integer, point;
	int i, j;
	int fp;

	if (do_lint && get_curfunc_arg_count() > 4)
		lintwarn("select: called with too many arguments");


	/*** Analyse File-descriptors ***/

	nfds = -1;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	for (i = 0; i < 3; i++ ) {
		array = (NODE *) get_array_argument(i, FALSE);
		if ( array == NULL ) { continue; }

		for (j = 0; j < array->array_size; j++) {

			for (elm = array->var_array[j]; elm != NULL; elm = elm->ahnext) {
				value = elm->hvalue;
				force_string(value);

				rp = getredirect(value->stptr, value->stlen);
				if (rp == NULL) {
					if (do_lint) {
						lintwarn("select: `%.*s' is not an open file, pipe or co-process",
								(int) value->stlen, value->stptr);
					}
				}

				fp = fileno(rp->fp);

				switch (i) {
				case 0:
					FD_SET(fp, &rfds);
					break;
				case 1:
					FD_SET(fp, &wfds);
					break;
				case 2:
					FD_SET(fp, &efds);
					break;
				}
				if (fp + 1 > nfds ) { nfds = fp + 1; }
			}
		}
	}


	/*** Analyse Timeout ***/
	/* timeout specified as milli-seconds */

	tmp = (NODE *) get_actual_argument(3, FALSE, FALSE);
	point = modf(force_number(tmp), &integer);
	if (integer < 0) {
		timeout_ptr = NULL;
	} else {
		timeout.tv_sec = (time_t) (integer / 1000);
		timeout.tv_usec = (suseconds_t) (point * 1000);
		timeout_ptr = &timeout;
	}


	retval = select(nfds, &rfds, &wfds, &efds, timeout_ptr);

	if (retval == -1) {
		perror("select()");
	} else if (retval != 0) {
		/* TODO */
	}

	return make_number((AWKNUM) retval);

}

static NODE *
can_sub(int mode)
{
	NODE *tmp;
	int nfds;
	fd_set fds;
	struct timeval timeout;
	struct timeval *timeout_ptr;
	int retval;
	struct redirect *rp;
	double integer, point;


	/*** Analyse File-descriptor ***/

	tmp = (NODE *) get_actual_argument(0, FALSE, FALSE);
	force_string(tmp);

	rp = getredirect(tmp->stptr, tmp->stlen);

	if (rp == NULL) {	/* no match, return -1 */
		if (do_lint) {
			const char *fn;

			switch (mode) {
			case CAN_READ:
				fn = "can_read";
				break;
			case CAN_WRITE:
				fn = "can_write";
				break;
			case HAS_EXCEPTION:
				fn = "has_excepstion";
				break;
			}
			lintwarn("%s: `%.*s' is not an open file, pipe or co-process",
				fn, (int) tmp->stlen, tmp->stptr);
		}
		return make_number((AWKNUM) 0);
	}

	nfds = fileno(rp->fp) + 1;

	FD_ZERO(&fds);
	FD_SET(fileno(rp->fp), &fds);


	/*** Analyse Timeout ***/
	/* timeout specified as milli-seconds */

	tmp = (NODE *) get_actual_argument(1, FALSE, FALSE);
	point = modf(force_number(tmp), &integer);
	if (integer < 0) {
		timeout_ptr = NULL;
	} else {
		timeout.tv_sec = (time_t) (integer / 1000);
		timeout.tv_usec = (suseconds_t) (point * 1000);
		timeout_ptr = &timeout;
	}


	switch (mode) {
	case CAN_READ:
		retval = select(nfds, &fds, NULL, NULL, timeout_ptr);
		break;
	case CAN_WRITE:
		retval = select(nfds, NULL, &fds, NULL, timeout_ptr);
		break;
	case HAS_EXCEPTION:
		retval = select(nfds, NULL, NULL, &fds, timeout_ptr);
		break;
	}

	if (retval == -1)
		perror("select()");

	return make_number((AWKNUM) retval);
}

static NODE *
do_can_read(int nags)
{
	if (do_lint && get_curfunc_arg_count() > 2)
		lintwarn("can_write: called with too many arguments");

	return can_sub(CAN_READ);
}

static NODE *
do_can_write(int nags)
{
	if (do_lint && get_curfunc_arg_count() > 2)
		lintwarn("can_write: called with too many arguments");

	return can_sub(CAN_WRITE);
}

static NODE *
do_has_exception(int nags)
{
	if (do_lint && get_curfunc_arg_count() > 2)
		lintwarn("has_exception: called with too many arguments");

	return can_sub(HAS_EXCEPTION);
}

static NODE *
do_sleep(int nags)
{
	NODE *tmp;
	struct timeval timeout;
	double integer,  point;

	if (do_lint && get_curfunc_arg_count() > 1)
		lintwarn("sleep: called with too many arguments");

	tmp = (NODE *) get_actual_argument(0, FALSE, FALSE);

	point = modf(force_number(tmp), &integer);

	timeout.tv_sec = (time_t) integer;
	timeout.tv_usec = (suseconds_t) (point * 1000000);

	select(0, NULL, NULL, NULL, &timeout);
	return make_number((AWKNUM) 0);
}

static NODE *
do_usleep(int nags)
{
	NODE *tmp;
	struct timeval timeout;

	if (do_lint && get_curfunc_arg_count() > 1)
		lintwarn("usleep: called with too many arguments");

	timeout.tv_sec = 0;

	tmp = (NODE *) get_actual_argument(0, FALSE, FALSE);
	timeout.tv_usec = (suseconds_t) force_number(tmp);

	select(0, NULL, NULL, NULL, &timeout);
	return make_number((AWKNUM) 0);
}

NODE *
dlload(NODE *tree, void *dl)
{
	make_builtin("select", do_select, 4);
	make_builtin("can_read", do_can_read, 2);
	make_builtin("can_write", do_can_write, 2);
	make_builtin("has_exception", do_has_exception, 2);
	make_builtin("sleep", do_sleep, 1);
	make_builtin("usleep", do_usleep, 1);

	return make_number((AWKNUM) 0);
}
