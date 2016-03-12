#ifndef __CONTROL_DEFINITIONS_H
#define __CONTROL_DEFINITIONS_H

#include "pid.h"
#include "trigonometry.h"
#include "vector3.h"
#include "quaternion.h"

/*===========================================================================*/
/* Module global definitions.                                                */
/*===========================================================================*/

#define RATE_PI_OFFSET                          3
#define ATTITUDE_PI_OFFSET                      0

#define CONTROL_NUMBER_OF_CONTROLLERS           (6) /* 3 rate + 3 attitude */
#define OUTPUT_MIXER_SIZE                       (sizeof(output_mixer_t))
#define CONTROL_ARM_SIZE                        (sizeof(control_arm_settings_t))
#define CONTROL_LIMITS_SIZE                     (sizeof(control_limits_t))
#define CONTROL_REFERENCE_SIZE                  (sizeof(control_reference_t))
#define CONTROL_DATA_SIZE                       (sizeof(control_data_t))
#define CONTROL_PARAMETERS_SIZE                 (sizeof(control_parameters_t))

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Possible reference sources for the controllers.
 */
typedef enum PACKED_VAR
{
    /**
     * @brief   Reference comes from manual (RC receiver).
     */
    REFERENCE_SOURCE_MANUAL = 0,
    /**
     * @brief   Reference comes from the computer control.
     */
    REFERENCE_SOURCE_COMPUTER_CONTROL
} reference_source_t;

/**
 * @brief   Position, velocity, attitude and rate controller gains and states.
 */
typedef struct PACKED_VAR
{
    /**
     * @brief   Attitude controller gains and states.
     */
    pi_data_t attitude_controller[3];
    /**
     * @brief   Rate controller gains and states.
     */
    pi_data_t rate_controller[3];
} control_data_t;

/**
 * @brief   Velocity, attitude and rate control limits.
 */
typedef struct PACKED_VAR
{
    /**
     * @brief   Holder for the rate limits.
     */
    vector3f_t max_rate;
    /**
     * @brief   Holder for the rate limits in attitude mode.
     */
    vector3f_t max_rate_attitude;
    /**
     * @brief   Holder for the attitude limits.
     */
    struct
    {
        /**
         * @brief   Roll attitude limit in radians.
         */
        float roll;
        /**
         * @brief   Pitch attitude limit in radians.
         */
        float pitch;
    } max_angle;
    /**
     * @brief   Holder for the velocity limits.
     */
    struct
    {
        /**
         * @brief   Horizontal velocity limit in m/s.
         */
        float horizontal;
        /**
         * @brief   Vertical velocity limit in m/s.
         */
        float vertical;
    } max_velocity;
} control_limits_t;

/**
 * @brief   Output mixer weights.
 */
typedef struct PACKED_VAR
{
    /**
     * @brief   Weights.
     */
    float weights[8][4];
    /**
     * @brief   Offsets to compensate for, as an example, the zero of an servo.
     */
    float offset[8];
} output_mixer_t;

/*
 * Data transfer structures
 */

/**
 * @brief   PI controller parameters structure.
 */
typedef struct PACKED_VAR
{
    /**
     * @brief   Controller proportional gain.
     */
    float P_gain;
    /**
     * @brief   Controller integral gain.
     */
    float I_gain;
} pi_parameters_t;

/**
 * @brief   Control parameters structure for moving data.
 */
typedef struct PACKED_VAR
{
    /**
     * @brief   Attitude controller parameters.
     */
    pi_parameters_t attitude_parameters[3];
    /**
     * @brief   Rate controller parameters.
     */
    pi_parameters_t rate_parameters[3];
} control_parameters_t;

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

void ComputerControlInit(void);
reference_source_t GetReferenceSource(void);
void vParseComputerControlPacket(const uint8_t *payload, const uint8_t size);

#endif