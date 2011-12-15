 /*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation.
 */
/* ------------------------------------------------------------------------- */
#ifndef _EEPROM_
#define _EEPROM_

#define EEPROM_SIZE	256
#define EEPROM_PAGE_SIZE 16

#define DEFAULT_DRIVER_PATH	"/sys/bus/i2c/devices/3-0050/eeprom"
#define DEFAULT_I2C_PATH	"/dev/i2c-3";
#define DEFAULT_I2C_ADDR	0x50

struct eeprom {
	char *driver_devfile;
	char *i2c_devfile;
	int i2c_addr;
};

enum eeprom_errors {
	EEPROM_NULL_PTR		= -1,
	EEPROM_INVAL_MODE	= -2,
	EEPROM_INVAL_OFFSET	= -3,
	EEPROM_INVAL_SIZE	= -4,
	EEPROM_OPEN_FAILED	= -5,
	EEPROM_NO_SUCH_FUNCTION	= -6,
	EEPROM_NO_I2C_ACCESS	= -7,
	EEPROM_IO_FAILED	= -8,
	EEPROM_READ_FAILED	= -9, /* For wrapper function */
	EEPROM_WRITE_FAILED	= -10 /* For wrapper function */
};

enum access_mode {
	EEPROM_DRIVER_MODE,
	EEPROM_I2C_MODE
};

enum eeprom_cmd {
	EEPROM_READ,
	EEPROM_WRITE
};

struct eeprom *new_eeprom(char *driver_path, char *i2c_path, int i2c_addr);
int eeprom_read(struct eeprom e, char *buf, int offset, int size,
		enum access_mode mode);
int eeprom_write(struct eeprom e, char *buf, int offset, int size,
		enum access_mode mode);

#endif
