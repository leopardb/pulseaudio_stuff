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
	// cool

	//int* pa_ready = userdata;

	printf("Callback, eol = %d\n", eol);

	//*pa_ready = 1;
}


void pa_sink_input_info_cb(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {
	// If eol is set to a positive number, you're at the end of the list
	if (eol > 0) {
		return;
	}

	if (strcmp(i->name,(const char*)userdata)==0) {
		printf("%d\n", i->index);
		//printf("sink-input name\t= %s\n", i->name);
	}

}


int main(int argc, char *argv[]) {
	/*const pa_sample_spec ss = {
	.format = PA_SAMPLE_S16LE,
	.rate = atoi(argv[1]),
	.channels = atoi(argv[2])
 	};

 	const pa_buffer_attr buffer_attr = {
	.maxlength = -1,
	.tlength = pa_usec_to_bytes(atoi(argv[3]) * 1e3, &ss),
	.prebuf = -1,
	.minreq = -1,
	.fragsize = -1
	};

	int error = 0;

	pa_simple* s = pa_simple_new(NULL, "test (app name)", PA_STREAM_PLAYBACK, NULL,"test (stream_name)", &ss, NULL, &buffer_attr, &error);
	printf("error: %s\n", pa_strerror(error));

	
	pa_usec_t latency = pa_simple_get_latency(s, &error);


	printf("latency: %lu\n", latency);	
	printf("error: %s\n", pa_strerror(error));


	sleep(15);

	pa_simple_free(s);*/

	if (argc != 2)
	{
		printf("%s takes one arguments (the name of the stream)\n", argv[0]);
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
	pa_ctx = pa_context_new(pa_mlapi, "test");

	pa_context_connect(pa_ctx, NULL, PA_CONTEXT_NOFLAGS, NULL);

	pa_context_set_state_callback(pa_ctx, pa_state_cb, &pa_ready);

	//pa_context_get_sink_info_list(pa_ctx, pa_sink_info_cb, &pa_ready);

	//pa_operation_state_t ret = pa_operation_get_state(pa_op);

	/*while (pa_ready == 0) {

		int ret = pa_mainloop_iterate(pa_ml, 0, NULL);
		printf("number of sources dispatched = %d\n", ret);
		printf("pa_ready = %d\n", pa_ready);

	}*/

	const char* mystring = argv[1];

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
				//pa_op = pa_context_get_sink_info_list(pa_ctx, pa_sink_info_cb, NULL);

				pa_op = pa_context_get_sink_input_info_list(pa_ctx, pa_sink_input_info_cb, (void*)mystring);

				state++;
				break;
			case 1:
				if (pa_operation_get_state(pa_op) == PA_OPERATION_RUNNING) {
					//printf("PA_OPERATION_RUNNING\n");
				}

				if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE) {

					//do my stuff
					//printf("PA_OPERATION_DONE\n");

					// clean shit

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

	return 0;
}


