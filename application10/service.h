#pragma once

void service_init (void);
void service_fini (void);
void service_start_accepting_new_connections (void);
void service_stop_accepting_new_connections (void);
