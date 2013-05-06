/* Copyright 2013-present Facebook, Inc.
 * Licensed under the Apache License, Version 2.0 */

#include "watchman.h"

/* since /root <timestamp> [patterns] */
void cmd_since(struct watchman_client *client, json_t *args)
{
  struct watchman_rule *rules = NULL;
  w_root_t *root;
  json_t *clock_ele;
  struct w_clockspec_query since;
  char buf[128];

  /* resolve the root */
  if (json_array_size(args) < 3) {
    send_error_response(client, "not enough arguments for 'since'");
    return;
  }

  root = resolve_root_or_err(client, args, 1, false);
  if (!root) {
    return;
  }

  clock_ele = json_array_get(args, 2);
  if (!w_parse_clockspec(root, clock_ele, &since, true)) {
    send_error_response(client,
        "expected argument 2 to be a valid clockspec");
    w_root_delref(root);
    return;
  }

  /* parse argv into a chain of watchman_rule */
  if (!parse_watch_params(3, args, &rules, NULL, buf, sizeof(buf))) {
    send_error_response(client, "invalid rule spec: %s", buf);
    w_root_delref(root);
    return;
  }

  if (!w_root_sync_to_now(root, trigger_settle)) {
    send_error_response(client, "synchronization failure: %s",
        strerror(errno));
    w_free_rules(rules);
    w_root_delref(root);
    return;
  }


  /* now find all matching files */
  run_rules(client, root, &since, rules);
  w_free_rules(rules);
  w_root_delref(root);
}


/* vim:ts=2:sw=2:et:
 */

