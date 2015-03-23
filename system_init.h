#ifndef __SYSTEM_INIT_H
#define __SYSTEM_INIT_H

/*===========================================================================*/
/* Module global definitions.                                                */
/*===========================================================================*/

/** @brief Key for the system shutdown. */
#define SYSTEM_SHUTDOWN_KEY				0xdeadbeef

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/** @brief Structure for critical task subscription. */
typedef struct _system_critical_subscription
{
    /** @brief Pointer to the next critical task structure. */
    struct _system_critical_subscription *next;
    /** @brief Pointer to the critical task. */
    thread_t *thread;
} system_critical_subscription_t;

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/


#endif