/* *
 *
 * Working layer for circular buffers
 *
 * */

#include "ch.h"
#include "hal.h"
#include "crc.h"
#include "statemachine_types.h"
#include "circularbuffer.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief                   Initializes a circular buffer except the mutex.
 * 
 * @param[in] Cbuff         Pointer to the circular buffer.
 * @param[in] buffer        Pointer to where the circular buffer data is stored.
 * @param[in] buffer_size   Size of the circular buffer in bytes.
 */
void CircularBuffer_Init(circular_buffer_t *Cbuff,
                         uint8_t *buffer, 
                         uint32_t buffer_size)
{
    Cbuff->head = 0;
    Cbuff->tail = 0;
    Cbuff->size = buffer_size;
    Cbuff->buffer = buffer;
}

/**
 * @brief               Initializes the circular buffer mutex.
 * 
 * @param[in] Cbuff     Pointer to the circular buffer.
 */
void CircularBuffer_InitMutex(circular_buffer_t *Cbuff)
{
    chMtxObjectInit(&Cbuff->write_lock);
}

/**
 * @brief               Claims a circular buffer using the mutex.
 * 
 * @param[in] Cbuff     Pointer to the circular buffer.
 */
void CircularBuffer_Claim(circular_buffer_t *Cbuff)
{
    chMtxLock(&Cbuff->write_lock);
}

/**
 * @brief               Releases a circular buffer using the mutex.
 * 
 * @param[in] Cbuff     Pointer to the circular buffer.
 */
void CircularBuffer_Release(circular_buffer_t *Cbuff)
{
    chMtxUnlock(&Cbuff->write_lock);
}

/**
 * @brief               Calculates the space left in a circular buffer.
 * 
 * @param[in] Cbuff     Pointer to the circular buffer.
 */
uint32_t CircularBuffer_SpaceLeft(circular_buffer_t *Cbuff)
{
    return (Cbuff->tail + Cbuff->size - Cbuff->head - 1) % Cbuff->size;
}

/**
 * @brief               Writes a byte to a circular buffer.
 * 
 * @param[in] Cbuff     Pointer to the circular buffer.
 */
void CircularBuffer_WriteSingle(circular_buffer_t *Cbuff, uint8_t data)
{
    Cbuff->buffer[Cbuff->head] = data;
    Cbuff->head = ((Cbuff->head + 1) % Cbuff->size);
}

/**
 * @brief               Writes a chunk of data to a circular buffer.
 * @note                This algorithm assumes you have checked that the
 *                      data will fit inside the buffer.
 *                      
 * @param[in/out] Cbuff Pointer to the circular buffer.
 * @param[in] data      Pointer to the data being written.
 * @param[in] count     Size of the data being written in bytes.
 */
void CircularBuffer_WriteChunk(circular_buffer_t *Cbuff, 
                               uint8_t *data, 
                               const uint32_t count)
{
    uint32_t i, head, from_bot, to_top;

    head = Cbuff->head;
    to_top = Cbuff->size - Cbuff->head;


    if (to_top < count)
    {   /* If we need to wrap around during the write */
        from_bot = count - to_top;

        /* First we fill to the top */
        for (i = 0; i < to_top; i++)
            Cbuff->buffer[head + i] = data[i];

        /* Then we fill the rest */
        for (i = 0; i < from_bot; i++)
            Cbuff->buffer[i] = data[to_top + i];

        Cbuff->head = from_bot; /* The end value of head was pre-calculated */
    }
    else
    {   /* No wrap around needed, chunk will fit in the space left to the top */
        for (i = 0; i < count; i++)
            Cbuff->buffer[head + i] = data[i];

        Cbuff->head = (Cbuff->head + count); /* There will be no wrap around */
    }
}

/**
 * @brief               Writes a SYNC byte to the circular buffer.
 *                      
 * @param[in/out] Cbuff Pointer to the circular buffer.
 * @param[in/out] count Pointer to tracking variable for the size of the
 *                      data being written to the circular buffer.
 * @param[in] crc8      Pointer to the CRC8 data holder.
 * @param[in] crc16     Pointer to the CRC16 data holder.
 */
void CircularBuffer_WriteSYNCNoIncrement(circular_buffer_t *Cbuff, 
                                         int32_t *count, 
                                         uint8_t *crc8, 
                                         uint16_t *crc16)
{
    /* Check if we have 4 byte free for SYNC + Header */
    if (CircularBuffer_SpaceLeft(Cbuff) >= 4)
    {
        Cbuff->buffer[(Cbuff->head + *count) % Cbuff->size] = SYNC_BYTE;
        *count += 1;

        /* When writing the SYNC CRC8 must be calculated */
        *crc8 = CRC8_step(SYNC_BYTE, 0x00);

        if (crc16 != NULL)
            *crc16 = CRC16_step(SYNC_BYTE, 0xffff); 
    }
    else
        *count = -1;
}

/**
 * @brief               Writes a byte to the circular buffer, if its vaule is
 *                      SYNC: write it twice.
 *  
 * @param[in/out] Cbuff Pointer to the circular buffer.
 * @param[in/out] data  Byte being written.
 * @param[in/out] count Pointer to tracking variable for the size of the
 *                      data being written to the circular buffer.
 * @param[in] crc8      Pointer to the CRC8 data holder.
 * @param[in] crc16     Pointer to the CRC16 data holder.
 */
void CircularBuffer_WriteNoIncrement(circular_buffer_t *Cbuff,
                                     uint8_t data, 
                                     int32_t *count, 
                                     uint8_t *crc8, 
                                     uint16_t *crc16)
{
    /* Check if we have an error from previous write */
    if (*count >= 0)
    {
        /* Check if we have 2 bytes free, in case of data = SYNC */
        if ((CircularBuffer_SpaceLeft(Cbuff) - *count) >= 2)
        {
            Cbuff->buffer[(Cbuff->head + *count) % Cbuff->size] = data;
            *count += 1;

            /* Only add CRCs of they are needed */
            if (crc8 != NULL)
                *crc8 = CRC8_step(data, *crc8);

            if (crc16 != NULL)
                *crc16 = CRC16_step(data, *crc16);  

            if (data == SYNC_BYTE)
            {
                Cbuff->buffer[(Cbuff->head + *count) % Cbuff->size] = SYNC_BYTE;
                *count += 1;
            }
        }
        else
            *count = -1;
    }
}

/**
 * @brief               Increment the circular buffer pointer to math the
 *                      number of bytes written.
 *  
 * @param[in/out] Cbuff Pointer to the circular buffer.
 * @param[in] count     Number of bytes to increment the pointer.
 */
bool CircularBuffer_Increment(circular_buffer_t *Cbuff, int32_t count)
{
    if (count == -1) /* Error! */
        return HAL_FAILED;

    else
    {
        Cbuff->head = ((Cbuff->head + count) % Cbuff->size);
        return HAL_SUCCESS;
    }
}

/**
 * @brief               Reads a byte from a circular buffer.
 *  
 * @param[in/out] Cbuff Pointer to the circular buffer.
 */
uint8_t CircularBuffer_ReadSingle(circular_buffer_t *Cbuff)
{
    uint8_t data;

    data = Cbuff->buffer[Cbuff->tail];
    Cbuff->tail = ((Cbuff->tail + 1) % Cbuff->size);

    return data;
}

/**
 * @brief               Reads a chunk of data from a circular buffer.
 *  
 * @param[in/out] Cbuff Pointer to the circular buffer.
 * @param[out] data     Pointer to write the data.
 * @param[in] count     Number of bytes to read.
 */
void CircularBuffer_ReadChunk(circular_buffer_t *Cbuff, 
                              uint8_t *data, 
                              uint32_t count)
{
    (void)Cbuff;
    (void)data;
    (void)count;
}

/**
 * @brief               Generates a pointer to the tail byte and returns a size
 *                      which for how many bytes can be read from the circular
 *                      buffer.
 *  
 * @param[in/out] Cbuff Pointer to the circular buffer.
 * @param[out] size     Pointer to the size holder.
 * @return              Pointer to the buffer with offset.
 */
uint8_t *CircularBuffer_GetReadPointer(circular_buffer_t *Cbuff,
                                       uint32_t *size)
{
    uint8_t *p;

    p = (Cbuff->buffer + Cbuff->tail);

    if (Cbuff->head < Cbuff->tail)
        *size = Cbuff->size - Cbuff->tail;
    else
        *size = Cbuff->head - Cbuff->tail;

    return p;
}

void CircularBuffer_IncrementTail(circular_buffer_t *Cbuff, int32_t count)
{
    Cbuff->tail = ((Cbuff->tail + count) % Cbuff->size);
}
