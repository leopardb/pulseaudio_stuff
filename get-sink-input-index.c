#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <pulse/error.h>
#include <pulse/pulseaudio.h>




void pa_state_cb(pa_context *c, void *userdata) {
        pa_context_state_t state;
        int *pa_ready = userdata;

        state = pa_context_get_state(c);
        switch  (state) {
                // There are just here for reference
                case PA_CONTEXT_UNCONNECTED:
                case PA_CONTEXT_CONNECTING:
                case PA_CONTEXT_AUTHORIZING:
                case PA_CONTEXT_SETTING_NAME:
                default:
                        break;
                case PA_CONTEXT_FAILED:
                case PA_CONTEXT_TERMINATED:
                        *pa_ready = 2;
                        break;
                case PA_CONTEXT_READY:
                        *pa_ready = 1;
                        break;
        }
}


void pa_sink_info_cb(pa_context *c, const pa_sink_info *i, int eol, void* userdata) {
}


void pa_sink_input_info_cb(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {

	// If eol is set to a positive number, you're at the end of the list
	if (eol > 0) {
		return;
	}


	const char** argv = userdata;

	const char* user_property = argv[1];
	const char* user_value = argv[2];

	if (strcmp(PA_PROP_APPLICATION_NAME,user_property)==0) {

		const char* value = pa_proplist_gets(i->proplist,PA_PROP_APPLICATION_NAME);

		if ((value!=NULL) && (strcmp(value,user_value)==0))
			printf("%d\n", i->index);
	}
}


int main(int argc, char *argv[]) {

	if (argc != 3)
	{
		printf("usage: %s property value\n  property: pulseaudio sink-input property (ex:application.name)\n  value: the value for the wanted property (ex: \"Quod Libet\")\n", argv[0]);
		return -1;
	}

	pa_mainloop* pa_ml;
	pa_mainloop_api* pa_mlapi;
	pa_context* pa_ctx;
	pa_operation* pa_op;

	int state = 0;
	int pa_ready = 0;

	pa_ml = pa_mainloop_new();
	pa_mlapi = pa_mainloop_get_api(pa_ml);
	pa_ctx = pa_context_new(pa_mlapi, "my_context");

	pa_context_connect(pa_ctx, NULL, PA_CONTEXT_NOFLAGS, NULL);

	pa_context_set_state_callback(pa_ctx, pa_state_cb, &pa_ready);

	for (;;) {
		if (pa_ready == 0) {
			pa_mainloop_iterate(pa_ml, 1, NULL);
			continue;
        	}
		if (pa_ready == 2) {
			pa_context_disconnect(pa_ctx);
			pa_context_unref(pa_ctx);
			pa_mainloop_free(pa_ml);
			return -1;
        	}

		switch (state) {
			case 0:

				pa_op = pa_context_get_sink_input_info_list(pa_ctx, pa_sink_input_info_cb, (void*)argv);

				state++;
				break;
			case 1:
				if (pa_operation_get_state(pa_op) == PA_OPERATION_RUNNING) {
					//printf("PA_OPERATION_RUNNING\n");
				}

				if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE) {

					// clean

					pa_operation_unref(pa_op);

					pa_context_disconnect(pa_ctx);
					pa_context_unref(pa_ctx);

					pa_mainloop_free(pa_ml);
				
					return 0;
				}
				break;
			default:
				// We should never see this state
				fprintf(stderr, "in state %d\n", state);
				return -1;
		}

		pa_mainloop_iterate(pa_ml, 1, NULL);

	}

	printf("we should not be here...\n");

	return -1;
}


