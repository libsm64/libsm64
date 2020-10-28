#include "global_state.h"

#include <stdlib.h>
#include <string.h>

struct GlobalState *g_state = 0;

struct GlobalState *global_state_create(void)
{
	struct GlobalState *state = malloc( sizeof( struct GlobalState ));
	memset( state, 0, sizeof( struct GlobalState ));
	state->msSwimStrength = MIN_SWIM_STRENGTH;
	return state;
}

void global_state_bind(struct GlobalState *state)
{
	g_state = state;
}

void global_state_delete(struct GlobalState *state)
{
	free( state );
}