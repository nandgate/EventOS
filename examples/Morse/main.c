/**
 * @file main.c
 * @brief Morse — Demonstrates nested iteration threads with no blocking loops.
 *
 * A common objection to cooperative, event-driven systems is "you can't loop
 * in an action." The answer is the *iteration pattern*: an action that does
 * one step of work, re-enqueues itself (or a successor) carrying its loop
 * state in the context, and yields control back to the FIFO between steps.
 * Other actions interleave freely while the iteration chews through its work.
 *
 * This example emits the Morse code for "HELLO WORLD " on LED_0 using
 * TWO nested iteration threads that share one context:
 *
 *   Outer thread — NextChar — walks the characters of the message.
 *   Inner thread — ElementOn / ElementOff — walks the dits and dahs of the
 *                                            current character and toggles
 *                                            LED_0 with the correct timing.
 *
 * The two threads hand off by calling each other directly — no flags, no
 * pub/sub, no polling. The last inner step schedules NextChar; NextChar,
 * after picking up the next character, schedules ElementOn.
 *
 * Meanwhile a separate heartbeat action toggles LED every 500 ms. On the
 * scope you see LED_0 emitting Morse while LED never stops blinking —
 * proof that the iteration did not block the system.
 *
 * Timing uses the classic Morse ratios (1 unit = 100 ms):
 *   dit = 1 unit          intra-letter gap = 1 unit
 *   dah = 3 units         inter-letter gap = 3 units
 *
 * Saleae capture: ../captures/Morse/Morse.sal
 */

#include "bluepill.h"
#include "Led.h"
#include "os/os.h"
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
    const char *message;   // message to emit, e.g. "SOS"
    uint8_t     msgIndex;  // index of the current character in message
    const char *pattern;   // Morse pattern for the current character (dits/dahs)
    uint8_t     patIndex;  // index of the current element in pattern
} morseState_t;

// Morse patterns for A-Z. Unknown characters return NULL and are skipped.
// Spaces are handled separately by NextChar as word boundaries.
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
        // Message complete. Returning releases the context.
        return;
    }
    if (c == ' ') {
        // Word boundary. A LETTER_GAP has already elapsed (scheduled by
        // the previous letter's last ElementOff), so wait the remainder
        // to bring the total pause up to WORD_GAP before moving on.
        s->msgIndex++;
        os_DoAfterWith(NextChar, context, OS_NO_KEY, WORD_GAP - LETTER_GAP);
        return;
    }
    const char *pat = MorsePattern(c);
    if (pat == NULL) {
        // Skip unknown character without any delay.
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
        // Last element of this letter — hand back to the outer thread
        // after the inter-letter gap.
        s->msgIndex++;
        os_DoAfterWith(NextChar, context, OS_NO_KEY, LETTER_GAP);
    } else {
        // More elements in this letter — continue the inner thread after
        // the intra-letter gap.
        os_DoAfterWith(ElementOn, context, OS_NO_KEY, ELEM_GAP);
    }
}

int main(void) {
    bluepill_Init();

    morseState_t *s = (morseState_t *)os_Do(NextChar, sizeof(morseState_t));
    s->message  = "HELLO WORLD ";
    s->msgIndex = 0;

    os_Do(Heartbeat, OS_NO_CONTEXT);

    while (1) {
        os_Exec();
    }
}
