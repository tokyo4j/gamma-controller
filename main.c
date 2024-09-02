#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wlr-gamma-control-unstable-v1-protocol.h>
#include <wlr-output-management-unstable-v1-protocol.h>

struct client_state {
    struct wl_display *wl_display;
    struct wl_registry *wl_registry;
    struct zwlr_gamma_control_manager_v1 *gamma_control_manager;
    struct zwlr_output_manager_v1 *output_manager;
    struct wl_list outputs;
    struct wl_list output_heads;
};

struct output_head {
    struct zwlr_output_head_v1 *head;
    char *name;
    struct wl_list link;
};

struct output {
    struct wl_output *wl_output;
    struct zwlr_gamma_control_v1 *gamma_control;
    struct output_head *head;
    struct client_state *client_state;
    char *name;
    bool gamma_failed;
    struct wl_list link;
};

static void nop() {}

static const struct wl_registry_listener wl_registry_listener;
static const struct wl_output_listener output_listener;
static const struct zwlr_output_head_v1_listener output_head_listener;
static const struct zwlr_output_manager_v1_listener output_manager_listener;

static void registry_global(void *data, struct wl_registry *wl_registry,
                            uint32_t name, const char *interface,
                            uint32_t version) {
    struct client_state *state = data;
    if (!strcmp(interface, zwlr_gamma_control_manager_v1_interface.name)) {
        state->gamma_control_manager = wl_registry_bind(
            wl_registry, name, &zwlr_gamma_control_manager_v1_interface, 1);
    } else if (!strcmp(interface, zwlr_output_manager_v1_interface.name)) {
        state->output_manager = wl_registry_bind(
            wl_registry, name, &zwlr_output_manager_v1_interface, 4);
        zwlr_output_manager_v1_add_listener(state->output_manager,
                                            &output_manager_listener, state);
    } else if (!strcmp(interface, wl_output_interface.name)) {
        struct output *output = calloc(1, sizeof(*output));
        output->wl_output =
            wl_registry_bind(wl_registry, name, &wl_output_interface, 4);
        wl_output_add_listener(output->wl_output, &output_listener, output);
        wl_list_insert(&state->outputs, &output->link);
    }
}

static const struct wl_registry_listener wl_registry_listener = {
    .global = registry_global,
    .global_remove = nop,
};

static void handle_output_done(void *data, struct wl_output *wl_output) {}

static void handle_output_name(void *data, struct wl_output *wl_output,
                               const char *name) {
    struct output *output = data;
    output->name = strdup(name);
}

static const struct wl_output_listener output_listener = {
    .geometry = nop,
    .mode = nop,
    .done = handle_output_done,
    .scale = nop,
    .name = handle_output_name,
    .description = nop,
};

static void handle_gamma_control_failed(
    void *data, struct zwlr_gamma_control_v1 *zwlr_gamma_control_v1) {
    struct output *output = data;
    output->gamma_failed = true;
}

static const struct zwlr_gamma_control_v1_listener gamma_control_listener = {
    .gamma_size = nop,
    .failed = handle_gamma_control_failed,
};

static void handle_ouptut_manager_head(
    void *data, struct zwlr_output_manager_v1 *zwlr_output_manager_v1,
    struct zwlr_output_head_v1 *head) {
    struct client_state *state = data;

    struct output_head *output_head = calloc(1, sizeof(*output_head));
    output_head->head = head;
    zwlr_output_head_v1_add_listener(head, &output_head_listener, output_head);
    wl_list_insert(&state->output_heads, &output_head->link);
}

static const struct zwlr_output_manager_v1_listener output_manager_listener = {
    .head = handle_ouptut_manager_head,
    .done = nop,
    .finished = nop,
};

static void
handle_output_head_name(void *data,
                        struct zwlr_output_head_v1 *zwlr_output_head_v1,
                        const char *name) {
    struct output_head *head = data;
    head->name = strdup(name);
}

static const struct zwlr_output_head_v1_listener output_head_listener = {
    .name = handle_output_head_name,
    .description = nop,
    .physical_size = nop,
    .mode = nop,
    .enabled = nop,
    .current_mode = nop,
    .position = nop,
    .transform = nop,
    .scale = nop,
    .finished = nop,
    .make = nop,
    .model = nop,
    .serial_number = nop,
    .adaptive_sync = nop,
};

int main(void) {
    struct output *output;
    struct output_head *head;

    struct client_state state = {0};
    wl_list_init(&state.outputs);
    wl_list_init(&state.output_heads);
    state.wl_display = wl_display_connect(NULL);
    state.wl_registry = wl_display_get_registry(state.wl_display);
    wl_registry_add_listener(state.wl_registry, &wl_registry_listener, &state);
    wl_display_roundtrip(state.wl_display);
    if (!state.gamma_control_manager) {
        fprintf(stderr,
                "compositor doesn't support wlr-gamma-control-v1 protocol");
        exit(1);
    }
    if (!state.output_manager) {
        fprintf(
            stderr,
            "compositor doesn't support wlr-output-management-v1 protocol.");
    }

    wl_list_for_each(output, &state.outputs, link) {
        output->gamma_control = zwlr_gamma_control_manager_v1_get_gamma_control(
            state.gamma_control_manager, output->wl_output);
        zwlr_gamma_control_v1_add_listener(output->gamma_control,
                                           &gamma_control_listener, output);
    }
    wl_display_roundtrip(state.wl_display);

    wl_list_for_each(head, &state.output_heads, link) {
        printf("%s\n", head->name ? head->name : "?");
    }

    wl_list_for_each(output, &state.outputs, link) {
        printf("%s\n", output->name ? output->name : "?");
        if (output->gamma_failed) {
            fprintf(stderr, "gamma cannot be configured for %s\n",
                    output->name);
            continue;
        }
        wl_list_for_each(head, &state.output_heads, link) {
            if (!strcmp(head->name, output->name))
                output->head = head;
        }
    }

    return 0;
}
