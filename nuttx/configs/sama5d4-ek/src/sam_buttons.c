/****************************************************************************
 * configs/sama5d4-ek/src/sam_buttons.c
 *
 *   Copyright (C) 2014 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/
/* A single button, PB_USER1 (PB2), is available on the SAMA5D4-EK:
 *
 * ------------------------------ ------------------- ----------------------
 * SAMA5D4 PIO                    SIGNAL              USAGE
 * ------------------------------ ------------------- ----------------------
 * PE13/A13/TIOB1/PWML2           PB_USER1_PE13       PB_USER1
 * ------------------------------ ------------------- ----------------------
 *
 * Closing JP2 will bring PE13 to ground so 1) PE13 should have a weak
 * pull-up, and 2) when PB2 is pressed, a low value will be senses.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>

#include <arch/irq.h>
#include <arch/board/board.h>

#include "sam_pio.h"
#include "sama5d4-ek.h"

#ifdef CONFIG_ARCH_BUTTONS

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

#if defined(CONFIG_SAMA5_PIOE_IRQ) && defined(CONFIG_ARCH_IRQBUTTONS)
static xcpt_t g_irquser1;
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_button_initialize
 *
 * Description:
 *   board_button_initialize() must be called to initialize button resources.
 *   After that, board_buttons() may be called to collect the current state
 *   of all buttons or board_button_irq() may be called to register button
 *   interrupt handlers.
 *
 ****************************************************************************/

void board_button_initialize(void)
{
  (void)sam_configpio(PIO_BTN_USER);
}

/****************************************************************************
 * Name: board_buttons
 *
 * Description:
 *   After board_button_initialize() has been called, board_buttons() may be
 *   called to collect the state of all buttons.  board_buttons() returns an
 *   8-bit bit set with each bit associated with a button.  See the BUTTON*
 *   definitions above for the meaning of each bit in the returned value.
 *
 ****************************************************************************/

uint8_t board_buttons(void)
{
  return sam_pioread(PIO_BTN_USER) ? 0 : BUTTON_USER_BIT;
}

/****************************************************************************
 * Name: board_button_irq
 *
 * Description:
 *   This function may be called to register an interrupt handler that will
 *   be called when a button is depressed or released.  The ID value is one
 *   of the BUTTON* definitions provided above. The previous interrupt
 *   handler address is returned (so that it may restored, if so desired).
 *
 * Configuration Notes:
 *   Configuration CONFIG_SAMA5_PIO_IRQ must be selected to enable the
 *   overall PIO IRQ feature and CONFIG_SAMA5_PIOE_IRQ must be enabled to
 *   select PIOs to support interrupts on PIOE.
 *
 ****************************************************************************/

#if defined(CONFIG_SAMA5_PIOE_IRQ) && defined(CONFIG_ARCH_IRQBUTTONS)
xcpt_t board_button_irq(int id, xcpt_t irqhandler)
{
  xcpt_t oldhandler = NULL;

  if (id == BUTTON_USER)
    {
      irqstate_t flags;

      /* Disable interrupts until we are done.  This guarantees that the
       * following operations are atomic.
       */

      flags = irqsave();

      /* Get the old button interrupt handler and save the new one */

      oldhandler = g_irquser1;
      g_irquser1 = irqhandler;

      /* Are we attaching or detaching? */

      if (irqhandler != NULL)
        {
          /* Configure the interrupt */

          sam_pioirq(PIO_BTN_USER);
          (void)irq_attach(IRQ_BTN_USER, irqhandler);
          sam_pioirqenable(IRQ_BTN_USER);
        }
      else
        {
          /* Disable and detach the interrupt */

          sam_pioirqdisable(IRQ_BTN_USER);
          (void)irq_detach(IRQ_BTN_USER);
        }

      irqrestore(flags);
    }

  /* Return the old button handler (so that it can be restored) */

  return oldhandler;
}
#endif

#endif /* CONFIG_ARCH_BUTTONS */
