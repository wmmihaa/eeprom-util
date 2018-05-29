/*
 * Copyright (C) 2009-2011 CompuLab, Ltd.
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 *
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "common.h"
#include "field.h"

#define PRINT_FIELD_SEGMENT	"%-30s"
// Macro for printing field's input value error messages
#define iveprintf(str, value, name) \
	ieprintf("Invalid value \"%s\" for field \"%s\" - " str, value, name);

static void __print_bin(const struct field *field,
			char *delimiter, bool reverse)
{
	ASSERT(field && field->buf && field->name && delimiter);

	printf(PRINT_FIELD_SEGMENT, field->name);
	int i;
	int from = reverse ? field->size - 1 : 0;
	int to = reverse ? 0 : field->size - 1;
	for (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", field->buf[i], delimiter);

	printf("%02x\n", field->buf[i]);
}

static int __update_bin(struct field *field, const char *value, bool reverse)
{
	ASSERT(field && field->buf && field->name && value);

	int len = strlen(value);
	int i = reverse ? len - 1 : 0;

	/* each two characters in the string are fit in one byte */
	if (len > field->size * 2) {
		iveprintf("Value is too long", value, field->name);
		return -1;
	}

	/* pad with zeros */
	memset(field->buf, 0, field->size);

	/* i - string iterator, j - buf iterator */
	for (int j = 0; j < field->size; j++) {
		int byte = 0;
		char tmp[3] = { 0, 0, 0 };

		if ((reverse && i < 0) || (!reverse && i >= len))
			break;

		for (int k = 0; k < 2; k++) {
			if (reverse && i == 0) {
				tmp[k] = value[i];
				break;
			}

			tmp[k] = value[reverse ? i - 1 + k : i + k];
		}

		char *str = tmp;
		if (strtoi_base(&str, &byte, 16) < 0 || byte < 0 || byte >> 8) {
			iveprintf("Syntax error", value, field->name);
			return -1;
		}

		field->buf[j] = (unsigned char)byte;
		i = reverse ? i - 2 : i + 2;
	}

	return 0;
}

static int __update_bin_delim(struct field *field, char *value, char delimiter)
{
	ASSERT(field && field->buf && field->name && value);

	int i, val;
	char *bin = value;

	for (i = 0; i < (field->size - 1); i++) {
		if (strtoi_base(&bin, &val, 16) != STRTOI_STR_CON ||
		    *bin != delimiter || val < 0 || val >> 8) {
			iveprintf("Syntax error", value, field->name);
			return -1;
		}

		field->buf[i] = (unsigned char)val;
		bin++;
	}

	if (strtoi_base(&bin, &val, 16) != STRTOI_STR_END ||
	    val < 0 || val >> 8) {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	field->buf[i] = (unsigned char)val;

	return 0;
}

/**
 * print_bin() - print a field which contains binary data
 *
 * Treat the field data as simple binary data, and print it as two digit
 * hexadecimal values.
 * Sample output:
 * 	Field Name	0102030405060708090a
 *
 * @field:	an initialized field to print
 */
static void print_bin(const struct field *field)
{
	__print_bin(field, "", false);
}

/**
 * print_bin_raw() - print raw data both in hexadecimal and in ascii format
 *
 * @field:	an initialized field to print
 */
static void print_bin_raw(const struct field *field)
{
	ASSERT(field && field->buf && field->name);

	printf(PRINT_FIELD_SEGMENT, field->name);
	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f"
	       "     0123456789abcdef\n");
	int i, j;

	for (i = 0; i < 256; i += 16) {
		printf("%02x: ", i);
		for (j = 0; j < 16; j++) {
			printf("%02x", field->buf[i+j]);
			printf(" ");
		}
		printf("    ");

		for (j = 0; j < 16; j++) {
			if (field->buf[i+j] == 0x00 || field->buf[i+j] == 0xff)
				printf(".");
			else if (field->buf[i+j] < 32 || field->buf[i+j] >= 127)
				printf("?");
			else
				printf("%c", field->buf[i+j]);
		}
		printf("\n");
	}
}

/**
 * update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
static int update_bin(struct field *field, char *value)
{
	return __update_bin(field, value, false);
}

/**
 * print_bin_rev() - print a field which contains binary data in reverse order
 *
 * Treat the field data as simple binary data, and print it in reverse order
 * as two digit hexadecimal values.
 *
 * Data in field:
 *  			0102030405060708090a
 * Sample output:
 * 	Field Name	0a090807060504030201
 *
 * @field:	an initialized field to print
 */
static void print_bin_rev(const struct field *field)
{
	__print_bin(field, "", true);
}

/**
 * update_bin_rev() - Update field with new data in binary form, storing it in
 * 		      reverse
 *
 * This function takes a string of byte values, and stores them
 * in the field in the reverse order. i.e. if the input string was "1234",
 * "3412" will be written to the field.
 *
 * @field:	an initialized field
 * @value:	a string of byte values
 */
static int update_bin_rev(struct field *field, char *value)
{
	return __update_bin(field, value, true);
}

/**
 * print_bin_ver() - print a "version field" which contains binary data
 *
 * Treat the field data as simple binary data, and print it formatted as a
 * version number (2 digits after decimal point).
 * The field size must be exactly 2 bytes.
 *
 * Sample output:
 * 	Field Name	123.45
 *
 * @field:	an initialized field to print
 */
static void print_bin_ver(const struct field *field)
{
	ASSERT(field && field->buf && field->name);

	if ((field->buf[0] == 0xff) && (field->buf[1] == 0xff)) {
		field->buf[0] = 0;
		field->buf[1] = 0;
	}

	printf(PRINT_FIELD_SEGMENT, field->name);
	printf("%#.2f\n", (field->buf[1] << 8 | field->buf[0]) / 100.0);
}

/**
 * update_bin_ver() - update a "version field" which contains binary data
 *
 * This function takes a version string in the form of x.y (x and y are both
 * decimal values, y is limited to two digits), translates it to the binary
 * form, then writes it to the field. The field size must be exactly 2 bytes.
 *
 * This function strictly enforces the data syntax, and will not update the
 * field if there's any deviation from it. It also protects from overflow.
 *
 * @field:	an initialized field
 * @value:	a version string
 *
 * Returns 0 on success, -1 on failure.
 */
static int update_bin_ver(struct field *field, char *value)
{
	ASSERT(field && field->buf && field->name && value);

	char *version = value;
	int num, remainder;

	if (strtoi(&version, &num) != STRTOI_STR_CON && *version != '.') {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	version++;
	if (strtoi(&version, &remainder) != STRTOI_STR_END) {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	if (num < 0 || remainder < 0) {
		iveprintf("Version must be positive", value, field->name);
		return -1;
	}

	if (remainder > 99) {
		iveprintf("Minor version is 1-2 digits", value, field->name);
		return -1;
	}

	num = num * 100 + remainder;
	if (num >> 16) {
		iveprintf("Version is too big", value, field->name);
		return -1;
	}

	field->buf[0] = (unsigned char)num;
	field->buf[1] = num >> 8;

	return 0;
}

/**
 * print_mac_addr() - print a field which contains a mac address
 *
 * Treat the field data as simple binary data, and print it formatted as a MAC
 * address.
 * Sample output:
 * 	Field Name	01:02:03:04:05:06
 *
 * @field:	an initialized field to print
 */
static void print_mac(const struct field *field)
{
	__print_bin(field, ":", false);
}

/**
 * update_mac() - Update a mac address field which contains binary data
 *
 * @field:	an initialized field
 * @value:	a colon delimited string of byte values (i.e. "1:02:3:ff")
 */
static int update_mac(struct field *field, char *value)
{
	return __update_bin_delim(field, value, ':');
}

static char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/**
 * print_date() - print a field which contains date data
 *
 * Treat the field data as simple binary data, and print it formatted as a date.
 * Sample output:
 * 	Field Name	07/Feb/2014
 * 	Field Name	56/BAD/9999
 *
 * @field:	an initialized field to print
 */
static void print_date(const struct field *field)
{
	ASSERT(field && field->buf && field->name);

	printf(PRINT_FIELD_SEGMENT, field->name);
	printf("%02d/", field->buf[0]);
	if (field->buf[1] >= 1 && field->buf[1] <= 12)
		printf("%s", months[field->buf[1] - 1]);
	else
		printf("BAD");

	printf("/%d\n", field->buf[3] << 8 | field->buf[2]);
}

static int validate_date(unsigned char day, unsigned char month,
			unsigned int year)
{
	int days_in_february;

	switch (month) {
	case 0:
	case 2:
	case 4:
	case 6:
	case 7:
	case 9:
	case 11:
		if (day > 31)
			return -1;
		break;
	case 3:
	case 5:
	case 8:
	case 10:
		if (day > 30)
			return -1;
		break;
	case 1:
		days_in_february = 28;
		if (year % 4 == 0) {
			if (year % 100 != 0) {
				days_in_february = 29;
			} else if (year % 400 == 0) {
				days_in_february = 29;
			}
		}

		if (day > days_in_february)
			return -1;

		break;
	default:
		return -1;
	}

	return 0;
}

/**
 * update_date() - update a date field which contains binary data
 *
 * This function takes a date string in the form of x/Mon/y (x and y are both
 * decimal values), translates it to the binary representation, then writes it
 * to the field.
 *
 * This function strictly enforces the data syntax, and will not update the
 * field if there's any deviation from it. It also protects from overflow in the
 * year value, and checks the validity of the date.
 *
 * @field:	an initialized field
 * @value:	a date string
 *
 * Returns 0 on success, -1 on failure.
 */
static int update_date(struct field *field, char *value)
{
	ASSERT(field && field->buf && field->name && value);

	char *date = value;
	int day, month, year;

	if (strtoi(&date, &day) != STRTOI_STR_CON || *date != '/') {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	if (day == 0) {
		iveprintf("Invalid day", value, field->name);
		return -1;
	}

	date++;
	if (strlen(date) < 4 || *(date + 3) != '/') {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	for (month = 1; month <= 12; month++)
		if (!strncmp(date, months[month - 1], 3))
			break;

	if (strncmp(date, months[month - 1], 3)) {
		iveprintf("Invalid month", value, field->name);
		return -1;
	}

	date += 4;
	if (strtoi(&date, &year) != STRTOI_STR_END) {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	if (validate_date(day, month - 1, year)) {
		iveprintf("Invalid date", value, field->name);
		return -1;
	}

	if (year >> 16) {
		iveprintf("Year overflow", value, field->name);
		return -1;
	}

	field->buf[0] = (unsigned char)day;
	field->buf[1] = (unsigned char)month;
	field->buf[2] = (unsigned char)year;
	field->buf[3] = (unsigned char)(year >> 8);

	return 0;
}

/**
 * print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
static void print_ascii(const struct field *field)
{
	ASSERT(field && field->buf && field->name);

	char format[8];
	int *str = (int*)field->buf;
	int pattern = *str;
	/* assuming field->size is a multiple of 32bit! */
	int block_count = field->size / sizeof(int);
	char *print_buf = "";

	/* check if str is trivial (contains only 0's or only 0xff's), if so print nothing */
	for (int i = 0; i < block_count - 1; i++) {
		str++;
		if (*str != pattern || (pattern != 0 && pattern != -1)) {
			print_buf = (char*)field->buf;
			break;
		}
	}

	sprintf(format, "%%.%ds\n", field->size);
	printf(PRINT_FIELD_SEGMENT, field->name);
	printf(format, print_buf);
}

/**
 * update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
static int update_ascii(struct field *field, char *value)
{
	ASSERT(field && field->buf && field->name && value);

	if (strlen(value) >= field->size) {
		iveprintf("Value is too long", value, field->name);
		return -1;
	}

	strncpy((char *)field->buf, value, field->size - 1);
	field->buf[field->size - 1] = '\0';

	return 0;
}

/**
 * print_reserved() - print the "Reserved fields" field
 *
 * Print a notice that the following field_size bytes are reserved.
 *
 * Sample output:
 * 	Reserved fields	              (64 bytes)
 *
 * @field:	an initialized field to print
 */
static void print_reserved(const struct field *field)
{
	ASSERT(field);
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", field->size);
}

/**
 * clear_field() - clear a field
 *
 * A cleared field is defined by having all bytes set to 0xff.
 *
 * @field:	an initialized field to clear
 */
static void clear_field(struct field *field)
{
	ASSERT(field && field->buf);
	memset(field->buf, 0xff, field->size);
}

#define OPS_UPDATABLE(type) { \
	.print		= print_##type, \
	.update		= update_##type, \
	.clear		= clear_field, \
}

#define OPS_PRINTABLE(type) { \
	.print		= print_##type, \
	.update		= NULL, \
	.clear		= NULL, \
}

static struct field_ops field_ops[] = {
	[FIELD_BINARY]		= OPS_UPDATABLE(bin),
	[FIELD_REVERSED]	= OPS_UPDATABLE(bin_rev),
	[FIELD_VERSION]		= OPS_UPDATABLE(bin_ver),
	[FIELD_ASCII]		= OPS_UPDATABLE(ascii),
	[FIELD_MAC]		= OPS_UPDATABLE(mac),
	[FIELD_DATE]		= OPS_UPDATABLE(date),
	[FIELD_RESERVED]	= OPS_PRINTABLE(reserved),
	[FIELD_RAW]		= OPS_PRINTABLE(bin_raw)
};

/**
 * init_field() - init field according to field.type
 *
 * @field:	an initialized field with a known field.type to init
 */
void init_field(struct field *field)
{
	ASSERT(field);

	field->ops = &field_ops[field->type];
}
