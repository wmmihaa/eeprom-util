/*
 * Copyright (C) 2009-2017 CompuLab, Ltd.
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
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include "layout.h"
#include "common.h"
#include "field.h"

#define LAYOUT_CHECK_BYTE	44
#define NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"
#define ARRAY_LEN(x)		(sizeof(x) / sizeof((x)[0]))

struct field layout_legacy[5] = {
	{ "MAC address",		"mac",	6,	FIELD_MAC },
	{ "Board Revision",		"rev",	2,	FIELD_BINARY },
	{ "Serial Number",		"sn",	8,	FIELD_BINARY },
	{ "Board Configuration",	"conf",	64,	FIELD_ASCII },
	{ "Reserved fields",		"rsvd",	176,	FIELD_RESERVED },
};

struct field layout_v1[12] = {
	{ "Major Revision",	"major",	2,	FIELD_VERSION },
	{ "Minor Revision",	"minor",	2,	FIELD_VERSION },
	{ "1st MAC Address",	"mac1",		6,	FIELD_MAC },
	{ "2nd MAC Address",	"mac2",		6,	FIELD_MAC },
	{ "Production Date",	"date",		4,	FIELD_DATE },
	{ "Serial Number",	"sn",		12,	FIELD_REVERSED },
	{ "Reserved fields",	"rsvd",		96,	FIELD_RESERVED },
	{ "Product Name",	"name",		16,	FIELD_ASCII },
	{ "Product Options #1",	"opt1",		16,	FIELD_ASCII },
	{ "Product Options #2",	"opt2",		16,	FIELD_ASCII },
	{ "Product Options #3",	"opt3",		16,	FIELD_ASCII },
	{ "Reserved fields",	"rsvd",		64,	FIELD_RESERVED },
};

struct field layout_v2[15] = {
	{ "Major Revision",			"major",	2,	FIELD_VERSION },
	{ "Minor Revision",			"minor",	2,	FIELD_VERSION },
	{ "1st MAC Address",			"mac1",		6,	FIELD_MAC },
	{ "2nd MAC Address",			"mac2",		6,	FIELD_MAC },
	{ "Production Date",			"date",		4,	FIELD_DATE },
	{ "Serial Number",			"sn",		12,	FIELD_REVERSED },
	{ "3rd MAC Address (WIFI)",		"mac3",		6,	FIELD_MAC },
	{ "4th MAC Address (Bluetooth)",	"mac4",		6,	FIELD_MAC },
	{ "Layout Version",			"layout",	1,	FIELD_BINARY },
	{ "Reserved fields",			"rsvd",		83,	FIELD_RESERVED },
	{ "Product Name",			"name", 	16,	FIELD_ASCII },
	{ "Product Options #1",			"opt1", 	16,	FIELD_ASCII },
	{ "Product Options #2",			"opt2", 	16,	FIELD_ASCII },
	{ "Product Options #3",			"opt3", 	16,	FIELD_ASCII },
	{ "Reserved fields",			"rsvd",		64,	FIELD_RESERVED },
};

struct field layout_v3[16] = {
	{ "Major Revision",			"major",	2,	FIELD_VERSION },
	{ "Minor Revision",			"minor",	2,	FIELD_VERSION },
	{ "1st MAC Address",			"mac1",		6,	FIELD_MAC },
	{ "2nd MAC Address",			"mac2",		6,	FIELD_MAC },
	{ "Production Date",			"date",		4,	FIELD_DATE },
	{ "Serial Number",			"sn",		12,	FIELD_REVERSED },
	{ "3rd MAC Address (WIFI)",		"mac3",		6,	FIELD_MAC },
	{ "4th MAC Address (Bluetooth)",	"mac4",		6,	FIELD_MAC },
	{ "Layout Version",			"layout",	1,	FIELD_BINARY },
	{ "CompuLab EEPROM ID",			"id",		3,	FIELD_BINARY },
	{ "Reserved fields",			"rsvd",		80,	FIELD_RESERVED },
	{ "Product Name",			"name",		16,	FIELD_ASCII },
	{ "Product Options #1",			"opt1",		16,	FIELD_ASCII },
	{ "Product Options #2",			"opt2",		16,	FIELD_ASCII },
	{ "Product Options #3",			"opt3",		16,	FIELD_ASCII },
	{ "Reserved fields",			"rsvd",		64,	FIELD_RESERVED },
};

struct field layout_v4[21] = {
	{ "Major Revision",			"major",	2,	FIELD_VERSION },
	{ "Minor Revision",			"minor",	2,	FIELD_VERSION },
	{ "1st MAC Address",			"mac1",		6,	FIELD_MAC },
	{ "2nd MAC Address",			"mac2",		6,	FIELD_MAC },
	{ "Production Date",			"date",		4,	FIELD_DATE },
	{ "Serial Number",			"sn",		12,	FIELD_REVERSED },
	{ "3rd MAC Address (WIFI)",		"mac3",		6,	FIELD_MAC },
	{ "4th MAC Address (Bluetooth)",	"mac4",		6,	FIELD_MAC },
	{ "Layout Version",			"layout",	1,	FIELD_BINARY },
	{ "CompuLab EEPROM ID",			"id",		3,	FIELD_BINARY },
	{ "5th MAC Address",			"mac5",		6,	FIELD_MAC },
	{ "6th MAC Address",			"mac6",		6,	FIELD_MAC },
	{ "Scratchpad",				"spad",		4,	FIELD_BINARY },
	{ "Reserved fields",			"rsvd",		64,	FIELD_RESERVED },
	{ "Product Name",			"name",		16,	FIELD_ASCII },
	{ "Product Options #1",			"opt1",		16,	FIELD_ASCII },
	{ "Product Options #2",			"opt2",		16,	FIELD_ASCII },
	{ "Product Options #3",			"opt3",		16,	FIELD_ASCII },
	{ "Product Options #4",			"opt4",		16,	FIELD_ASCII },
	{ "Product Options #5",			"opt5",		16,	FIELD_ASCII },
	{ "Reserved fields",			"rsvd",		32,	FIELD_RESERVED },
};

struct field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, "raw", 256, FIELD_RAW },
};

/*
 * detect_layout() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
static enum layout_version detect_layout(unsigned char *data)
{
	ASSERT(data);

	switch (data[LAYOUT_CHECK_BYTE]) {
	case 0xff:
	case 0:
		return LAYOUT_VER1;
	case 2:
		return LAYOUT_VER2;
	case 3:
		return LAYOUT_VER3;
	case 4:
		return LAYOUT_VER4;
	}

	if (data[LAYOUT_CHECK_BYTE] >= 0x20)
		return LAYOUT_LEGACY;

	return LAYOUT_UNRECOGNIZED;
}

/*
 * build_layout() - Detect layout and build it with a predefined array
 * @layout:	An allocated layout
 */
static void build_layout(struct layout *layout)
{
	if (layout->layout_version == LAYOUT_AUTODETECT)
		layout->layout_version = detect_layout(layout->data);

	switch (layout->layout_version) {
	case LAYOUT_LEGACY:
		layout->fields = layout_legacy;
		layout->num_of_fields = ARRAY_LEN(layout_legacy);
		break;
	case LAYOUT_VER1:
		layout->fields = layout_v1;
		layout->num_of_fields = ARRAY_LEN(layout_v1);
		break;
	case LAYOUT_VER2:
		layout->fields = layout_v2;
		layout->num_of_fields = ARRAY_LEN(layout_v2);
		break;
	case LAYOUT_VER3:
		layout->fields = layout_v3;
		layout->num_of_fields = ARRAY_LEN(layout_v3);
		break;
	case LAYOUT_VER4:
		layout->fields = layout_v4;
		layout->num_of_fields = ARRAY_LEN(layout_v4);
		break;
	default:
		layout->fields = layout_unknown;
		layout->num_of_fields = ARRAY_LEN(layout_unknown);
	}
}

/*
 * print_layout() - print the layout and the data which is assigned to it.
 * @layout: A pointer to an existing struct layout.
 */
static void print_layout(const struct layout *layout)
{
	ASSERT(layout && layout->fields);

	struct field *fields = layout->fields;

	for (int i = 0; i < layout->num_of_fields; i++)
		fields[i].ops->print(&fields[i]);
}

/*
 * offset_to_string() - convert offset or range to string
 * @dest_str:		A pointer to where the string will be written
 * @offset_start:	The start offset
 * @offset_end:		The end offset
 */
static void offset_to_string(char* dest_str, int offset_start, int offset_end)
{
	ASSERT(dest_str);
	int chars = sprintf(dest_str, "'0x%02x", offset_start);
	if (offset_end != offset_start)
		chars += sprintf(dest_str + chars, "-0x%02x", offset_end);
	sprintf(dest_str + chars, "'");
}

/*
 * get_bytes_range() - Test offsets values and return range
 * @offset_start:	The start offset
 * @offset_end:		The end offset
 *
 * Returns: range on success, 0 on failure.
 */
static size_t get_bytes_range(int offset_start, int offset_end)
{
	if (offset_start < 0 || offset_start >= EEPROM_SIZE ||
	    offset_end < offset_start || offset_end >= EEPROM_SIZE) {
		char offset_str[30];
		offset_to_string(offset_str, offset_start, offset_end);
		ieprintf("Invalid offset %s", offset_str);
		return 0;
	}

	return offset_end - offset_start + 1;
}

/*
 * Selectively update EEPROM data by bytes.
 * @layout:	An initialized layout.
 * @data:	A data array. Each element contains the following:
 * 		start: The first byte in EEPROM to be written.
 * 		end: The last byte in EEPROM to be written.
 * 		value: The value to be written to EEPROM.
 *
 * Returns: number of updated bytes.
 */
static int update_bytes(struct layout *layout, struct data_array *data)
{
	ASSERT(layout && data && data->bytes_changes);

	int updated_bytes = 0;

	for (int i = 0; i < data->size; i++) {
		int offset_start = data->bytes_changes[i].start;
		int offset_end = data->bytes_changes[i].end;
		size_t range = get_bytes_range(offset_start, offset_end);
		if (range == 0)
			return 0;

		int value = data->bytes_changes[i].value;
		if (value >= 0 && value <= 255){
			memset(layout->data + offset_start, value, range);
			updated_bytes += range;
			continue;
		}

		char value_str[60];
		int chars = sprintf(value_str, "'0x%02x' at offset ", value);
		offset_to_string(value_str + chars, offset_start, offset_end);
		ieprintf("Invalid value %s", value_str);
		return 0;
	}

	return updated_bytes;
}

/*
 * Selectively clear EEPROM data by bytes.
 * @layout:	An initialized layout.
 * @data:	A data array. Each element contains the following:
 * 		start: The first byte in EEPROM to be cleared.
 * 		end: The last byte in EEPROM to be cleared.
 *
 * Returns: number of cleared bytes.
 */
static int clear_bytes(struct layout *layout, struct data_array *data)
{
	ASSERT(layout && data && data->bytes_list);

	int cleared_bytes = 0;

	for (int i = 0; i < data->size; i++) {
		int offset_start = data->bytes_list[i].start;
		int offset_end = data->bytes_list[i].end;
		size_t range = get_bytes_range(offset_start, offset_end);
		if (range == 0)
			return 0;

		memset(layout->data + offset_start, 0xff, range);
		cleared_bytes += range;
	}

	return cleared_bytes;
}

/*
 * find_field() - Find a field by name from the layout data.
 * @layout:	An initialized layout
 * @field_name:	The name of the field to find
 *
 * Returns: A pointer to the field on success, NULL on failure.
 */
static struct field* find_field(struct layout *layout, char *field_name)
{
	ASSERT(layout && layout->fields && field_name);

	struct field *fields = layout->fields;

	if (fields == layout_unknown) {
		eprintf("Layout error: Can't operate on fields. "
			"The layout is unknown.\n");
		return NULL;
	}

	for (int i = 0; i < layout->num_of_fields; i++)
		if (fields[i].ops->is_named(&fields[i], field_name))
			return &fields[i];

	ieprintf("Field \"%s\" not found", field_name);

	return NULL;
}

/*
 * Selectively update EEPROM data by fields.
 * @layout:	An initialized layout.
 * @data:	A data array. Each element contains field and value strings
 *
 * Returns: number of updated fields.
 */
static int update_fields(struct layout *layout, struct data_array *data)
{
	ASSERT(data && data->fields_changes);

	int updated_fields_cnt = 0;

	for (int i = 0; i < data->size; i++) {
		char *field_name = data->fields_changes[i].field;
		char *field_value = data->fields_changes[i].value;

		struct field *field = find_field(layout, field_name);
		if (!field)
			return 0;

		if (*field_value == '\0')
			field->ops->clear(field);
		else if (field->ops->update(field, field_value))
			return 0;

		updated_fields_cnt++;
	}

	return updated_fields_cnt;
}

/*
 * clear_fields() - Selectively clear EEPROM data by fields.
 * @layout:	An initialized layout
 * @data:	A data array. Each element contains field name string
 *
 * Returns: number of cleared fields.
 */
static int clear_fields(struct layout *layout, struct data_array *data)
{
	ASSERT(data && data->fields_list);

	int cleared_fields_cnt = 0;

	for (int i = 0; i < data->size; i++) {
		struct field *field = find_field(layout, data->fields_list[i]);
		if (!field)
			return 0;

		field->ops->clear(field);
		cleared_fields_cnt++;
	}

	return cleared_fields_cnt;
}

/*
 * new_layout() - Allocate a new layout based on the data given in buf.
 * @buf:	Data seed for layout
 * @buf_size:	Size of buf
 *
 * Allocates a new layout based on data in buf. The layout version is
 * automatically detected. The resulting layout struct contains a copy of the
 * provided data.
 *
 * Returns: pointer to a new layout on success, NULL on failure
 */
struct layout *new_layout(unsigned char *buf, unsigned int buf_size,
			  enum layout_version layout_version,
			  enum print_format print_format)
{
	ASSERT(buf);

	struct layout *layout = malloc(sizeof(struct layout));
	if (!layout)
		return NULL;

	layout->layout_version = layout_version;
	layout->data = buf;
	layout->data_size = buf_size;

	build_layout(layout);

	for (int i = 0; i < layout->num_of_fields; i++) {
		struct field *field = &layout->fields[i];
		init_field(field, buf, print_format);
		buf += field->ops->get_data_size(field);
	}

	layout->print = print_layout;
	layout->update_fields = update_fields;
	layout->update_bytes = update_bytes;
	layout->clear_fields = clear_fields;
	layout->clear_bytes = clear_bytes;

	return layout;
}

/*
 * free_layout() - a destructor for layout
 * @layout:	the layout to deallocate
 */
void free_layout(struct layout *layout)
{
	free(layout);
}
