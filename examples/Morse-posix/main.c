/**
 * @file main.c
 * @brief Morse-posix — Morse example running on a POSIX host.
 *
 * Identical action logic to examples/Morse (the iteration pattern,
 * nested NextChar / ElementOn / ElementOff threads, "HELLO WORLD "
 * message, 100 ms unit timing). The only difference is the platform:
 * LEDs print to stdout with timestamps and SysTick is simulated by
 * `posix_Tick()` in the exec loop (see Blinky-posix for the
 * limitations discussion).
 *
 * Useful as a smoke test: diff the stdout log against the expected
 * Morse timing and you have a cross-check that the OS behaves the
 * same way on a host as it does on the bluepill.
 *
 * Stop the program with Ctrl-C.
 */

#include "Led.h"
#include "os/os.h"
#include "posix.h"
#include <stddef.h>
#include <stdint.h>

#define UNIT_MS      100
#define DIT_MS       (1 * UNIT_MS)
#define DAH_MS       (3 * UNIT_MS)
#define ELEM_GAP     (1 * UNIT_MS)
#define LETTER_GAP   (3 * UNIT_MS)
#define WORD_GAP     (7 * UNIT_MS)
#define HEARTBEAT_MS 500

typedef struct {
    const char *message;
    uint8_t     msgIndex;
    const char *pattern;
    uint8_t     patIndex;
} morseState_t;

static const char *MorsePattern(char c) {
    switch (c) {
        case 'A': return ".-";
        case 'B': return "-...";
        case 'C': return "-.-.";
        case 'D': return "-..";
        case 'E': return ".";
        case 'F': return "..-.";
        case 'G': return "--.";
        case 'H': return "....";
        case 'I': return "..";
        case 'J': return ".---";
        case 'K': return "-.-";
        case 'L': return ".-..";
        case 'M': return "--";
        case 'N': return "-.";
        case 'O': return "---";
        case 'P': return ".--.";
        case 'Q': return "--.-";
        case 'R': return ".-.";
        case 'S': return "...";
        case 'T': return "-";
        case 'U': return "..-";
        case 'V': return "...-";
        case 'W': return ".--";
        case 'X': return "-..-";
        case 'Y': return "-.--";
        case 'Z': return "--..";
        default:  return NULL;
    }
}

void Heartbeat(os_context_t context) {
    (void)context;
    led_Toggle(LED);
    os_DoAfter(Heartbeat, OS_NO_CONTEXT, OS_NO_KEY, HEARTBEAT_MS);
}

void NextChar(os_context_t context);
void ElementOn(os_context_t context);
void ElementOff(os_context_t context);

void NextChar(os_context_t context) {
    morseState_t *s = (morseState_t *)context;
    char c = s->message[s->msgIndex];
    if (c == '\0') {
        return;
    }
    if (c == ' ') {
        s->msgIndex++;
        os_DoAfterWith(NextChar, context, OS_NO_KEY, WORD_GAP - LETTER_GAP);
        return;
    }
    const char *pat = MorsePattern(c);
    if (pat == NULL) {
        s->msgIndex++;
        os_DoWith(NextChar, context);
        return;
    }
    s->pattern  = pat;
    s->patIndex = 0;
    os_DoWith(ElementOn, context);
}

void ElementOn(os_context_t context) {
    morseState_t *s = (morseState_t *)context;
    led_On(LED_0);
    uint32_t dur = (s->pattern[s->patIndex] == '.') ? DIT_MS : DAH_MS;
    os_DoAfterWith(ElementOff, context, OS_NO_KEY, dur);
}

void ElementOff(os_context_t context) {
    morseState_t *s = (morseState_t *)context;
    led_Off(LED_0);
    s->patIndex++;
    if (s->pattern[s->patIndex] == '\0') {
        s->msgIndex++;
        os_DoAfterWith(NextChar, context, OS_NO_KEY, LETTER_GAP);
    } else {
        os_DoAfterWith(ElementOn, context, OS_NO_KEY, ELEM_GAP);
    }
}

int main(void) {
    posix_Init();

    morseState_t *s = (morseState_t *)os_Do(NextChar, sizeof(morseState_t));
    s->message  = "HELLO WORLD ";
    s->msgIndex = 0;

    os_Do(Heartbeat, OS_NO_CONTEXT);

    for (;;) {
        posix_Tick();
        os_Exec();
    }
}
