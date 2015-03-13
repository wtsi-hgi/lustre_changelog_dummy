/*******************************************************************************
 * Copyright (c) 2015 Genome Research Ltd. 
 * 
 * Author: Martin Pollard <mp15@sanger.ac.uk>
 * 
 * This program is free software: you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the Free Software 
 * Foundation; either version 3 of the License, or (at your option) any later 
 * version. 
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
 * details. 
 * 
 * You should have received a copy of the GNU General Public License along with 
 * this program. If not, see <http://www.gnu.org/licenses/>. 
 ******************************************************************************/

#include <lustre/lustreapi.h>
#include <signal.h>
#include <stdio.h>

void process_cl_ent( const struct changelog_ext_rec* cl_entry )
{
	printf("type: %s\n", changelog_type2str(cl_entry->cr_type));
}

volatile bool loop = true;

void termination_handler (int signum)
{
	loop = false;
}

int main(int argc, char** argv) {
	// Set ctrl+c handler
	struct sigaction new_action, old_action;
	new_action.sa_handler = termination_handler;
	sigemptyset (&new_action.sa_mask);
	new_action.sa_flags = 0;

	sigaction(SIGINT, &new_action, &old_action);

	// Start the changelog
	void* cl_state;
	if (llapi_changelog_start(&cl_state, CHANGELOG_FLAG_FOLLOW, "lus13", 0LL) != 0) {
		printf("Unable to initialise changelog\n");
		return 1;
	}
	printf("CONNECTED\n");
	while ( loop ) {
		printf("LOOP\n");
		struct changelog_ext_rec* cl_entry;
		int val;
		if ( (val = llapi_changelog_recv(cl_state, &cl_entry)) == 0) {
			printf("RECEIVED\n");
			process_cl_ent(cl_entry);
			llapi_changelog_free(&cl_entry);
		} if (val == 1) {
			loop = false;
			printf("EOF\n");
		} else {
			loop = false;
			printf("RECV != 0: %d\n", val);
		}
	}
	// Clean up
	if (llapi_changelog_fini(&cl_state) != 0) return 1;
	printf("CLEANED UP\n");
	return 0;
}
