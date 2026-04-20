#pragma once

typedef void (*button_isrCallback_t)(void);

void button_Init(button_isrCallback_t callback);
