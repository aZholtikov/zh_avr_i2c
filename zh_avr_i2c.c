#include "zh_avr_i2c.h"

#define I2C_OK AVR_BIT0
#define I2C_NACK AVR_BIT1
#define I2C_COLLISION AVR_BIT2
#define I2C_BUS_FAIL AVR_BIT3

#define I2C_START ((1 << TWINT) | (1 << TWEN) | (1 << TWIE))
#define I2C_MASTER_READ 1
#define I2C_MASTER_WRITE 0

typedef enum
{
	MASTER_WRITE,
	MASTER_READ,
	MASTER_WRITE_REG,
	MASTER_READ_REG
} _work_mode_t;

avr_err_t _zh_avr_i2c_master_start(TickType_t xTicksToWait);

static EventGroupHandle_t _event_group_handle = NULL;
static uint8_t _target_i2c_address = 0;
volatile static uint8_t _work_mode = 0;
volatile static uint8_t *_master_data = NULL;
volatile static uint8_t _master_data_size = 0;
static bool _master_is_initialized = false;

avr_err_t zh_avr_i2c_master_init(const bool pullup)
{
	_event_group_handle = xEventGroupCreate();
	ZH_ERROR_CHECK(_event_group_handle != NULL, AVR_ERR_NO_MEM);
	cli();
	DDRC &= ~(1 << PORTC5 | 1 << PORTC4);
	if (pullup == true)
	{
		PORTC |= 1 << PORTC5 | 1 << PORTC4;
	}
	TWBR = ((F_CPU / 100000) - 16) / 2;
	TWSR = 0xF8;
	sei();
	_master_is_initialized = true;
	return AVR_OK;
}

avr_err_t zh_avr_i2c_master_probe(const uint8_t addr, TickType_t xTicksToWait)
{
	uint8_t temp = 0;
	return zh_avr_i2c_master_transmit(addr, &temp, sizeof(temp), xTicksToWait);
}

avr_err_t zh_avr_i2c_master_transmit(const uint8_t addr, uint8_t *data, uint8_t size, TickType_t xTicksToWait)
{
	ZH_ERROR_CHECK(data != NULL || size > 0, AVR_ERR_INVALID_ARG);
	ZH_ERROR_CHECK(_master_is_initialized == true, AVR_ERR_INVALID_STATE);
	_work_mode = MASTER_WRITE;
	_target_i2c_address = addr;
	_master_data = data;
	_master_data_size = size;
	return _zh_avr_i2c_master_start(xTicksToWait);
}

avr_err_t zh_avr_i2c_master_receive(const uint8_t addr, uint8_t *data, uint8_t size, TickType_t xTicksToWait)
{
	ZH_ERROR_CHECK(data != NULL || size > 0, AVR_ERR_INVALID_ARG);
	ZH_ERROR_CHECK(_master_is_initialized == true, AVR_ERR_INVALID_STATE);
	_work_mode = MASTER_READ;
	_target_i2c_address = addr;
	_master_data = data;
	_master_data_size = size;
	return _zh_avr_i2c_master_start(xTicksToWait);
}

avr_err_t zh_avr_i2c_master_transmit_register(const uint8_t addr, uint16_t *reg, uint8_t *data, uint8_t size, TickType_t xTicksToWait)
{
	// To Do.
	return AVR_OK;
}
avr_err_t zh_avr_i2c_master_receive_register(const uint8_t addr, uint16_t *reg, uint8_t *data, uint8_t size, TickType_t xTicksToWait)
{
	// To Do.
	return AVR_OK;
}

avr_err_t _zh_avr_i2c_master_start(TickType_t xTicksToWait)
{
	TWCR = I2C_START | (1 << TWSTA);
	EventBits_t bits = xEventGroupWaitBits(_event_group_handle, I2C_OK | I2C_NACK | I2C_COLLISION | I2C_BUS_FAIL, pdTRUE, pdFALSE, xTicksToWait);
	if ((bits & I2C_OK) != 0)
	{
		return AVR_OK;
	}
	else if ((bits & I2C_NACK) != 0)
	{
		return AVR_ERR_INVALID_RESPONSE;
	}
	else
	{
		return AVR_FAIL;
	}
}

ISR(TWI_vect)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	switch (TWSR & 0xF8)
	{
	case 0x00: // Bus error.
		TWCR = I2C_START | (1 << TWSTO);
		xEventGroupSetBitsFromISR(_event_group_handle, I2C_BUS_FAIL, &xHigherPriorityTaskWoken);
		break;
	case 0x08: // A START condition has been transmitted.
		switch (_work_mode)
		{
		case MASTER_WRITE:
		case MASTER_WRITE_REG:
		case MASTER_READ_REG:
			TWDR = (_target_i2c_address << 1) | I2C_MASTER_WRITE;
			break;
		case MASTER_READ:
			TWDR = (_target_i2c_address << 1) | I2C_MASTER_READ;
			break;
		default:
			break;
		}
		TWCR = I2C_START;
		break;
	case 0x10: // A repeated START condition has been transmitted.
		// To Do.
		break;
	case 0x18: // SLA+W has been transmitted. ACK has been received.
		TWDR = *(_master_data++);
		--_master_data_size;
		TWCR = I2C_START;
		break;
	case 0x20: // SLA+W has been transmitted. NACK has been received.
		TWCR = I2C_START | (1 << TWSTO);
		xEventGroupSetBitsFromISR(_event_group_handle, I2C_NACK, &xHigherPriorityTaskWoken);
		break;
	case 0x28: // Data byte has been transmitted. ACK has been received.
		if (_master_data_size-- == 0)
		{
			TWCR = I2C_START | (1 << TWSTO);
			xEventGroupSetBitsFromISR(_event_group_handle, I2C_OK, &xHigherPriorityTaskWoken);
		}
		else
		{
			TWDR = *(_master_data++);
			TWCR = I2C_START;
		}
		break;
	case 0x30: // Data byte has been transmitted. NACK has been received.
		TWCR = I2C_START | (1 << TWSTO);
		xEventGroupSetBitsFromISR(_event_group_handle, I2C_NACK, &xHigherPriorityTaskWoken);
		break;
	case 0x38: // Arbitration lost in SLA+W or data bytes. Arbitration lost in SLA+R or NACK bit.
		TWCR = I2C_START | (1 << TWSTO);
		xEventGroupSetBitsFromISR(_event_group_handle, I2C_COLLISION, &xHigherPriorityTaskWoken);
		break;
	case 0x40: // SLA+R has been transmitted. ACK has been received.
		switch (_work_mode)
		{
		case MASTER_WRITE:
			break;
		case MASTER_READ:
			if (_master_data_size == 1)
			{
				TWCR = I2C_START;
			}
			else
			{
				TWCR = I2C_START | (1 << TWEA);
			}
			break;
		case MASTER_WRITE_REG:
			break;
		case MASTER_READ_REG:
			break;
		default:
			break;
		}
		break;
	case 0x48: // SLA+R has been transmitted. NACK has been received.
		TWCR = I2C_START | (1 << TWSTO);
		xEventGroupSetBitsFromISR(_event_group_handle, I2C_NACK, &xHigherPriorityTaskWoken);
		break;
	case 0x50: // Data byte has been received. ACK has been returned.
		*(_master_data++) = TWDR;
		if (--_master_data_size == 1)
		{
			TWCR = I2C_START;
		}
		else
		{
			TWCR = I2C_START | (1 << TWEA);
		}
		break;
	case 0x58: // Data byte has been received. NACK has been returned.
		*(_master_data) = TWDR;
		TWCR = I2C_START | (1 << TWSTO);
		xEventGroupSetBitsFromISR(_event_group_handle, I2C_OK, &xHigherPriorityTaskWoken);
		break;
	default:
		break;
	}
	if (xHigherPriorityTaskWoken == pdTRUE)
	{
		portYIELD();
	};
}