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
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "layout.h"
#include "field.h"

#define LAYOUT_CHECK_BYTE	44
#define RESERVED_FIELDS		0
#define NO_LAYOUT_FIELDS	"Could not detect layout. Dumping raw data\n"

static enum layout_names detect_layout(char *data);
static struct field *new_layout_legacy();
static struct field *new_layout_v1();
static struct field *new_layout_v2();
static struct field *new_layout_invalid();

/*
 * Allocates a new layout based on the data given in buf. The layout version
 * is automatically detected. The resulting layout struct contains a copy of
 * the provided data.
 */
struct layout *new_layout(char *buf, int buf_size)
{
	int i, layout_num;
	struct layout *l;
	char *temp;

	l = (struct layout *) malloc(sizeof(struct layout));
	if (l == NULL)
		return NULL;

	layout_num = detect_layout(buf);
	switch (layout_num) {
	case LAYOUT_LEGACY:
		l->fields = new_layout_legacy();
		break;
	case LAYOUT_VER1:
		l->fields = new_layout_v1();
		break;
	case LAYOUT_VER2:
		l->fields = new_layout_v2() ;
		break;
	default:
		l->fields = new_layout_invalid();
	}

	if (l->fields == NULL)
		goto free_shallow;

	l->data = (char *) malloc(sizeof(char) * buf_size);
	if (l->data == NULL)
		goto free_deep;

	for (i = 0; i < buf_size; i++)
		l->data[i] = buf[i];

	temp = l->data;
	for (i = 0; l->fields[i].size != 0; i++) {
		l->fields[i].buf = temp;
		temp += l->fields[i].size;
	}

	l->layout_number = layout_num;
	l->data_size = buf_size;
	return l;

free_deep:
	free(l->fields);
free_shallow:
	free(l);
	return NULL;
}

void free_layout(struct layout *layout)
{
	free(layout->fields);
	free(layout->data);
	free(layout);
}

void print_layout(struct layout *layout)
{
	int i;
	struct field *fields = layout->fields;
	for (i = 0; fields[i].size != 0; i++)
		fields[i].print(fields[i]);
}

enum layout_res update_field(struct layout *layout, char *field_name,
							char *new_data)
{
	int i;
	struct field *fields = layout->fields;
	if (layout == NULL || field_name == NULL || new_data == NULL)
		return LAYOUT_NULL_ARGUMENTS;

	/* Advance until the field name is found. */
	for (i = 0; fields[i].size != 0; i++) {
		if (fields[i].name != RESERVED_FIELDS &&
					!strcmp(fields[i].name, field_name))
			break;
	}

	if (fields[i].size == 0)
		return LAYOUT_NO_SUCH_FIELD;

	fields[i].update(&fields[i], new_data);
	return LAYOUT_SUCCESS;
}

enum layout_res update_byte(struct layout *layout, int offset, char new_byte)
{
	if (layout == NULL)
		return LAYOUT_NULL_ARGUMENTS;

	if (offset >= layout->data_size || offset < 0)
		return LAYOUT_OFFSET_OUT_OF_BOUNDS;

	layout->data[offset] = new_byte;
	return LAYOUT_SUCCESS;
}

static enum layout_names detect_layout(char *data)
{
	int check_byte = LAYOUT_CHECK_BYTE;

	if (data[check_byte] == 0xff || data[check_byte] == 0)
		return LAYOUT_VER1;

	if (data[check_byte] >= 0x20)
		return LAYOUT_LEGACY;

	return LAYOUT_VER2;
}

static struct field *new_layout_legacy()
{
	struct field *f = (struct field *) malloc(sizeof(struct field) * 6);
	if (f == NULL)
		return f;

	f[0] = set_field("MAC address", 6, ":", print_bin, update_binary);
	f[1] = set_field("Board Revision", 2, "", print_bin, update_binary);
	f[2] = set_field("Serial Number", 8, "", print_bin, update_binary);
	f[3] = set_field("Board Configuration", 64, "", print_ascii,
								update_ascii);
	f[4] = set_field(RESERVED_FIELDS, 176, "", print_reserved,
								update_ascii);
	f[5] = set_field(0, 0, 0, 0, 0);/* End of layout */

	return f;
}

static struct field *new_layout_v1()
{
	struct field *f = (struct field *) malloc(sizeof(struct field) * 13);
	if (f == NULL)
		return f;

	f[0] = set_field("Major Revision", 2, ".", print_bin_ver,
								update_binary);
	f[1] = set_field("Minor Revision", 2, ".", print_bin_ver,
								update_binary);
	f[2] = set_field("1st MAC addr", 6, ":", print_bin, update_binary);
	f[3] = set_field("2nd MAC addr", 6, ":", print_bin, update_binary);
	f[4] = set_field("Production Date", 4, "/", print_date, update_binary);
	f[5] = set_field("Serial Number", 12, " ", print_bin_rev,
								update_binary);
	f[6] = set_field(RESERVED_FIELDS, 96, "", print_reserved,
								update_binary);
	f[7] = set_field("Product Name", 16, "", print_ascii, update_ascii);
	f[8] = set_field("Product Options #1", 16, "", print_ascii,
								update_ascii);
	f[9] = set_field("Product Options #2", 16, "", print_ascii,
								update_ascii);
	f[10] = set_field("Product Options #3", 16, "", print_ascii,
								update_ascii);
	f[11] = set_field(RESERVED_FIELDS, 64, "", print_reserved,
								update_ascii);
	f[12] = set_field(0, 0, 0, 0, 0);/* End of layout */

	return f;
}

static struct field *new_layout_v2()
{
	struct field *f = (struct field *) malloc(sizeof(struct field) * 16);
	if (f == NULL)
		return f;

	f[0] = set_field("Major Revision", 2, ".", print_bin_ver,
								update_binary);
	f[1] = set_field("Minor Revision", 2, ".", print_bin_ver,
								update_binary);
	f[2] = set_field("1st MAC addr", 6, ":", print_bin, update_binary);
	f[3] = set_field("2nd MAC addr", 6, ":", print_bin, update_binary);
	f[4] = set_field("Production Date", 4, "/", print_date, update_binary);
	f[5] = set_field("Serial Number", 12, " ", print_bin_rev,
								update_binary);
	f[6] = set_field("WIFI MAC Address", 6, ":", print_bin, update_binary);
	f[7] = set_field("Bluetooth MAC Address", 6, ":", print_bin,
								update_binary);
	f[8] = set_field("Layout Version", 1, " ", print_bin, update_binary);
	f[9] = set_field(RESERVED_FIELDS, 83, "", print_reserved,
								update_binary);
	f[10] = set_field("Product Name", 16, "", print_ascii, update_ascii);
	f[11] = set_field("Product Options #1", 16, "", print_ascii,
								update_ascii);
	f[12] = set_field("Product Options #2", 16, "", print_ascii,
								update_ascii);
	f[13] = set_field("Product Options #3", 16, "", print_ascii,
								update_ascii);
	f[14] = set_field(RESERVED_FIELDS, 64, "", print_reserved,
								update_ascii);
	f[15] = set_field(0, 0, 0, 0, 0);/* End of layout */

	return f;
}

static struct field *new_layout_invalid()
{
	struct field *f = (struct field *) malloc(sizeof(struct field) * 2);
	if (f == NULL)
		return f;

	f[0] = set_field(NO_LAYOUT_FIELDS, 256, " ", print_bin, update_binary);
	f[1] = set_field(0, 0, 0, 0, 0);/* End of layout */

	return f;
}
