/**
  @file gen_vkb.c

  Copyright (C) 2011 Jonathan Wilson

  @author Jonathan wilson <jfwfreo@tpgi.com.au>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License version 2.1 as
  published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include <expat.h>
#include <glib.h>
#include "imlayouts.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

void
clear_vkb_keyboard_layout(vkb_keyboard_layout *layout)
{
	layout->file = 0;
	layout->layout.num_layouts = 0;
	layout->layout.wc = 0;
	layout->layout.lang = 0;
	layout->layout.name = 0;
	layout->layout.layout_types = 0;
	layout->layout.special_font = 0;
	layout->layout.offsets = 0;
	layout->layout.screen_modes = 0;
	layout->layout.num_screen_modes = 0;
	layout->layout.key_sizes = 0;
	layout->layout.num_key_sizes = 0;
	layout->layout.filename = 0;
	layout->layouts = 0;
	layout->body_fd = -1;
	layout->header_fd = -1;
	layout->is_slide_key = 0;
	layout->is_key = 0;
	layout->parsing_multiple = 0;
	layout->is_multiple_key = 0;
}

int string_length(char *str)
{
	int len;
	char *i;
	len = 0;
	for (i = str; i && *i; ++i)
		++len;
	return len;
}

void open_vkb_file(vkb_keyboard_layout *layout)
{
	layout->header_file = g_strdup_printf(".%s.H", layout->file);
	layout->body_file = g_strdup_printf(".%s.B", layout->file);
	layout->header_fd = creat(layout->header_file, 420);
	layout->body_fd = open(layout->body_file, 578, 420);
	if (layout->header_fd < 0 || layout->body_fd < 0)
	{
		printf("File couldn't be created: %s\n",strerror(errno));
	}
}

void close_vkb_file(vkb_keyboard_layout *layout)
{
	int sz;
	char buf[1024];
	lseek(layout->body_fd, 0, 0);
	do
	{
		sz = read(layout->body_fd, &buf, 1024);
		write(layout->header_fd, &buf, sz);
	}
	while (sz > 0);
	close(layout->body_fd);
	close(layout->header_fd);
	unlink(layout->body_file);
	rename(layout->header_file, layout->file);
	g_free(layout->header_file);
	g_free(layout->body_file);
	g_free(layout->file);
}

int write_vkb_file(vkb_keyboard_layout *layout, int write_space)
{
	int bytes = 0;
	int i;
	int fd = layout->header_fd;
	if (write_space)
	{
		if (write_space == 1)
		{
			char s[20];
			memset(&s, 0, 20);
			bytes += write(fd, &s, 20);
		}
	}
	else
	{
		unsigned char buf = 1;
		bytes += write(fd, &buf, 1);
		bytes += write(fd, &layout->layout.num_layouts, 1);
		buf = string_length(layout->layout.name);
		bytes += write(fd, &buf, 1);
		bytes += write(fd, layout->layout.name, buf);
		buf = string_length(layout->layout.lang);
		bytes += write(fd, &buf, 1);
		bytes += write(fd, layout->layout.lang, buf);
		buf = string_length(layout->layout.wc);
		bytes += write(fd, &buf, 1);
		bytes += write(fd, layout->layout.wc, buf);
		bytes += write(fd, &layout->layout.num_screen_modes, 1);
		for (i = 0;layout->layout.num_screen_modes > i;i++)
		{
			bytes += write(fd, &layout->layout.screen_modes[i], 1);
		}
		bytes += write(fd, &layout->layout.num_key_sizes, 1);
		for (i = 0;layout->layout.num_key_sizes > i;i++)
		{
			bytes += write(fd, &layout->layout.key_sizes[i].width, 1);
			bytes += write(fd, &layout->layout.key_sizes[i].height, 1);
			bytes += write(fd, &layout->layout.key_sizes[i].baseline, 1);
			bytes += write(fd, &layout->layout.key_sizes[i].margin_left, 1);
			bytes += write(fd, &layout->layout.key_sizes[i].margin_top, 1);
		}
	}
	return bytes;
}

int write_vkb_key(vkb_keyboard_layout *layout, vkb_key *key)
{
	int bytes = 0;
	int fd = layout->body_fd;
	unsigned char buf;
	buf = key->key_type;
	if (key->special_font == 1)
	{
		buf |= 128;
	}
	bytes += write(fd, &buf, 1);
	if (key->key_flags & 128)
	{
		bytes += write(fd, &key->key_flags, 2);
	}
	else
	{
		unsigned char flags = (unsigned char)key->key_flags;
		bytes += write(fd, &flags, 1);
	}
	if (key->key_type == KEY_TYPE_SLIDING)
	{
		buf = key->byte_count | 0x80;
		bytes += write(fd, &buf, 1);
		int i;
		for (i = 0; key->byte_count > i; ++i)
		{
			int length = string_length(key->labels[i]);
			bytes += write(fd, &length, 1);
			if (length)
			{
				bytes += write(fd, key->labels[i], length);
			}
		}
	}
	else
	{
		bytes += write(fd, &key->byte_count, 1);
		if (key->byte_count)
		{
			bytes += write(fd, (gchar *)key->labels, key->byte_count);
		}
	}
	bytes += write(fd, &key->key_size, 1);
	if (key->key_flags & 256)
	{
		bytes += write(fd, &key->scancode_length, 1);
		bytes += write(fd, key->scancode, key->scancode_length);
	}
	return bytes;
}

int write_vkb_key_section(vkb_keyboard_layout *layout, vkb_key_section *section)
{
	int fd = layout->body_fd;
	int bytes = 0;
	bytes = write(fd, &section->num_keys, 1);
	bytes += write(fd, &section->num_rows, 1);
	bytes += write(fd, &section->margin_left, 1);
	bytes += write(fd, &section->margin_top, 1);
	bytes += write(fd, &section->margin_right, 1);
	bytes += write(fd, &section->margin_bottom, 1);
	bytes += write(fd, section->num_keys_in_rows, section->num_rows);
	int i;
	for (i = 0;i < section->num_keys;i++)
	{
		if (section->keys[i].key_type == 4)
		{
			bytes += write(fd, &section->keys[i].key_type, 1);
			bytes += write(fd, &section->keys[i].num_sub_keys, 1);
			int j;
			for (j = 0;j < section->keys[i].num_sub_keys;j++)
			{
				bytes += write_vkb_key(layout, &section->keys[i].sub_keys[j]);
			}
		}
		else
		{
			bytes += write_vkb_key(layout, &section->keys[i]);
		}
	}
	return bytes;
}

int write_vkb_sub_layout(vkb_keyboard_layout *layout, vkb_sub_layout *sub_layout)
{
	int bytes = 0;
	int fd = layout->body_fd;
	bytes = write(fd, sub_layout, 1);
	bytes += write(fd, &sub_layout->variance_index, 1);
	int n = string_length(sub_layout->label);
	bytes += write(fd, &n, 1);
	if (n > 0)
	{
		bytes += write(fd, sub_layout->label, n);
	}
	bytes += write(fd, &sub_layout->num_key_sections, 1);
	int i;
	for (i = 0;i < sub_layout->num_key_sections;i++)
	{
		bytes += write_vkb_key_section(layout, &sub_layout->key_sections[i]);
	}
	return bytes;
}

int write_offset(vkb_keyboard_layout *layout, unsigned short offset)
{
	return write(layout->header_fd, &offset, 2);
}

void write_vkb_layouts(vkb_keyboard_layout *layout, unsigned short start_offset)
{
	int fd = layout->body_fd;
	unsigned short offset = start_offset;
	int i;
	for (i = 0;i < layout->layout.num_layouts;i++)
	{
		write_offset(layout,offset);
		offset += write(fd, &layout->layouts[i].type, 1);
		offset += write(fd, &layout->layouts[i].num_sub_layouts, 1);
		offset += write(fd, &layout->layouts[i].numeric, 1);
		offset += write(fd, &layout->layouts[i].default_key_size, 1);
		int j;
		for (j = 0;j < layout->layouts[i].num_sub_layouts;j++)
		{
			offset += write_vkb_sub_layout(layout, &layout->layouts[i].sub_layouts[j]);
		}
	}
}

unsigned char *parse_scancode(const char *layout, char *len)
{
	unsigned char *ptr = 0;
	int i = 0;
	*len = 0;
	gchar **str_array;
	if (layout)
	{
		str_array = g_strsplit(layout, " ", 0);
		if (str_array)
		{
			while (str_array[i])
			{
				errno = 0;
				unsigned char ch = strtol(str_array[i], 0, 10);
				if (errno)
				{
					g_print("Invalid number %s in scancode", str_array[i]);
				}
				else
				{
					++*len;
					ptr = (unsigned char *)realloc(ptr, *len);
					if (!ptr)
					{
						g_print("unable to add more scancode, memory exhausted");
						break;
					}
					ptr[*len - 1] = ch;
				}
				++i;
			}
			g_strfreev(str_array);
		}
	}
	return ptr;
}

void parse_keyboard(vkb_keyboard_layout *layout, const XML_Char *name, const XML_Char **atts)
{
	add_layout(layout);
	vkb_layout *lay = get_layout(layout);
	(void)name;
	while (atts && *atts)
	{
		if (strncmp(*atts, "default_key_size", 16))
		{
			if (strncmp(*atts, "numeric", 7))
			{
				if (strncmp(*atts, "layout", 6))
				{
					printf("Unknown attribute for 'keyboard': %s\n", *atts);
				}
				else
				{
					if (strncmp(atts[1], "THUMB", 5))
					{
						if (strncmp(atts[1], "SPECIAL", 7))
						{
							if (strncmp(atts[1], "LARGE", 5))
							{
								if (strncmp(atts[1], "PINYIN", 6))
								{
									if (strncmp(atts[1], "ZHUYIN", 6))
									{
										if (strncmp(atts[1], "STROKE", 6))
										{
											if (strncmp(atts[1], "CANGJIE", 7))
											{
												lay->type = LAYOUT_TYPE_NORMAL;
											}
											else
											{
												lay->type = LAYOUT_TYPE_CANGJIE;
											}
										}
										else
										{
											lay->type = LAYOUT_TYPE_STROKE;
										}
									}
									else
									{
										lay->type = LAYOUT_TYPE_ZHUYIN;
									}
								}
								else
								{
									lay->type = LAYOUT_TYPE_PINYIN;
								}
							}
							else
							{
								lay->type = LAYOUT_TYPE_LARGE;
							}
						}
						else
						{
							lay->type = LAYOUT_TYPE_SPECIAL;
						}
					}
					else
					{
						lay->type = LAYOUT_TYPE_THUMB;
					}
				}
			}
			else
			{
				if (strncmp(atts[1], "UPPERCASE", 9))
				{
					lay->numeric = 0;
				}
				else
				{
					lay->numeric = 1;
				}
			}
		}
		else
		{
			lay->default_key_size = atoi(atts[1]);
		}
		atts += 2;
	}
}

void parse_keyboards(vkb_keyboard_layout *layout, const XML_Char *name, const XML_Char **atts)
{
  (void)name;
        while (atts && *atts)
	{
		if (strncmp(*atts, "file", 4))
		{
			if (strncmp(*atts, "lang", 4))
			{
				if (strncmp(*atts, "version", 7))
				{
					if (strncmp(*atts, "name", 4))
					{
						if (strncmp(*atts, "wc", 2))
						{
							if (strncmp(*atts, "special_font", 2))
							{
								printf("Unknown attribute for 'keyboards': %s\n", *atts);
							}
							else
							{
								if (!strncmp(atts[1], "yes", 3)) 
								{
									layout->layout.special_font = 1;
								}
							}
						}
						else
						{
							layout->layout.wc = g_strdup(atts[1]);
						}
					}
					else
					{
						layout->layout.name = g_strdup(atts[1]);
					}
				}
				else
				{
					layout->layout.version = atoi(atts[1]);
				}
			}
			else
			{
				if (!layout->file)
				{
					layout->file = g_strdup_printf("%s.vkb", atts[1]);
				}
				layout->layout.lang = g_strdup(atts[1]);
			}
		}
		else
		{
			if (layout->file)
			{
				g_free(layout->file);
			}
			layout->file = g_strdup_printf("%s.vkb", atts[1]);
		}
		atts += 2;
	}
}

void parse_multiple(vkb_keyboard_layout *layout, const XML_Char *name, const XML_Char **atts)
{
	vkb_layout *lay = get_layout(layout);
	layout->parsing_multiple = 1;
	add_key(layout);
	vkb_key *key = get_key(layout);
	key->key_size = lay->default_key_size;
	(void)name;
	while (atts && *atts)
	{
		if (strncmp(*atts, "size", 4))
		{
			printf("Unknown attribute for 'multiple': %s\n", *atts);
		}
		else
		{
			key->key_size = atoi(atts[1]);
		}
		atts += 2;
	}
	increase_num_keys_in_row(layout);
	layout->parsing_multiple = 0;
}

void parse_key(vkb_keyboard_layout *layout, const XML_Char *name, const XML_Char **atts)
{
	vkb_layout *lay = get_layout(layout);
	add_key(layout);
	(void)name;
	if (!layout->is_multiple_key)
	{
		increase_num_keys_in_row(layout);
	}
	vkb_key *key = get_key(layout);
	key->scancode_length = 0;
	key->scancode = 0;
	key->byte_count = 0;
	key->labels = NULL;
	key->num_sub_keys = 0;
	key->sub_keys = NULL;
	key->special_font = 0;
	key->key_type = 0;
	key->key_flags = 0;
	key->key_size = lay->default_key_size;
	while (atts && *atts)
	{
		if (strncmp(*atts, "special_font", 12))
		{
			if (strncmp(*atts, "modifier", 8))
			{
				if (strncmp(*atts, "size", 4))
				{
					if (strncmp(*atts, "type", 4))
					{
						if (strncmp(*atts, "dead", 4))
						{
							if (strncmp(*atts, "tone", 4))
							{
								if (strncmp(*atts, "alpha", 5))
								{
									if (strncmp(*atts, "numeric", 7))
									{
										if (strncmp(*atts, "hexa", 4))
										{
											if (strncmp(*atts, "tele", 4))
											{
												if (strncmp(*atts, "special", 7))
												{
													if (strncmp(*atts, "whitespace", 10))
													{
														if (strncmp(*atts, "raw", 3))
														{
															if (strncmp(*atts, "tab", 3))
															{
																if (strncmp(*atts, "backspace", 9))
																{
																	if (strncmp(*atts, "shift", 5))
																	{
																		if (strncmp(*atts, "scancode", 8) || !(key->key_flags & KEY_TYPE_RAW))
																		{
																			printf("Unknown attribute for 'key': %s\n", *atts);
																		}
																		else
																		{
																			char size;
																			key->scancode = parse_scancode(atts[1], &size);
																			key->scancode_length = size;
																		}
																	}
																	else
																	{
																		key->key_flags |= 0x1080;
																	}
																}
																else
																{
																	key->key_flags |= 0x0880;
																}
															}
															else
															{
																key->key_flags |= 0x0480;
															}
														}
														else
														{
															key->key_flags |= 0x0180;
														}
													}
													else
													{
														key->key_flags |= KEY_TYPE_WHITESPACE;
													}
												}
												else
												{
													key->key_flags |= KEY_TYPE_SPECIAL;
												}
											}
											else
											{
												key->key_flags |= KEY_TYPE_TELE;
											}
										}
										else
										{
											key->key_flags |= KEY_TYPE_HEXA;
										}
									}
									else
									{
										key->key_flags |= KEY_TYPE_NUMERIC;
									}
								}
								else
								{
									key->key_flags |= KEY_TYPE_ALPHA;
								}
							}
							else
							{
								key->key_flags |= 0x0280;
							}
						}
						else
						{
							key->key_flags |= KEY_TYPE_DEAD;
						}
					}
					else
					{
						if (strncmp(atts[1], "SLIDE", 5))
						{
							if (strncmp(atts[1], "MODIFIER", 8))
							{
								key->key_type = KEY_TYPE_NORMAL;
							}
							else
							{
								key->key_type = KEY_TYPE_SLIDING;
							}
						}
						else
						{
							key->key_type = KEY_TYPE_SLIDING;
						}
					}
				}
				else
				{
					key->key_size = atoi(atts[1]);
				}
			}
			else
			{
				key->key_type = KEY_TYPE_MODIFIER;
			}
		}
		else
		{
			key->special_font = 1;
		}
		atts += 2;
	}
}

void parse_sub_layout(vkb_keyboard_layout *layout, const XML_Char *name, const XML_Char **atts)
{
	add_sublayout(layout);
	vkb_sub_layout *sublayout = get_sublayout(layout);
	sublayout->variance_index = -1;
	(void)name;
	while (atts && *atts)
	{
		if (strncmp(*atts, "type", 4))
		{
			if (strncmp(*atts, "label", 5))
			{
				if (strncmp(*atts, "variance_index", 14))
				{
					printf("Unknown attribute for 'sublayout': %s\n", *atts);
				}
				else
				{
					sublayout->variance_index = atoi(atts[1]);
				}
			}
			else
			{
				sublayout->label = g_strdup(atts[1]);
			}
		}
		else
		{
			if (strncmp(atts[1], "LOWERCASE", 9))
			{
				if (strncmp(atts[1], "UPPERCASE", 9))
				{
					sublayout->type = 2;
				}
				else
				{
					sublayout->type = 1;
				}
			}
			else
			{
				sublayout->type = 0;
			}
		}
		atts += 2;
	}
}

void parse_key_size(vkb_keyboard_layout *layout, const XML_Char *name, const XML_Char **atts)
{
	add_keysize(layout);
	vkb_key_size *size = get_keysize(layout);
	(void)name;
	while (atts && *atts)
	{
		if (strncmp(*atts, "margin_top", 10))
		{
			if (strncmp(*atts, "margin_left", 11))
			{
				if (strncmp(*atts, "baseline", 8))
				{
					if (strncmp(*atts, "height", 6))
					{
						if (strncmp(*atts, "width", 5))
						{
							printf("Unknown attribute for 'size': %s\n", *atts);
						}
						else
						{
							int val = atoi(atts[1]);
							if (val < 0)
							{
								printf("Invalid value for width %d (%s)\n", val, atts[1]);
								val = 0;
							}
							size->width = (unsigned char)val;
						}
					}
					else
					{
						int val = atoi(atts[1]);
						if (val < 0)
						{
							printf("Invalid value for height %d (%s)\n", val, atts[1]);
							val = 0;
						}
						size->height = (unsigned char)val;
					}
				}
				else
				{
					int val = atoi(atts[1]);
					if (val < 0)
					{
						printf("Invalid value for height %d (%s)\n", val, atts[1]);
						val = 0;
					}
					size->baseline = (unsigned char)val;
				}
			}
			else
			{
				int val = atoi(atts[1]);
				if (val < 0)
				{
					printf("Invalid value for %s %d (%s)\n", *atts, val, atts[1]);
					val = 0;
				}
				size->margin_left = (unsigned char)val;
			}
		}
		else
		{
			int val = atoi(atts[1]);
			if (val < 0)
			{
				printf("Invalid value for %s %d (%s)\n", *atts, val, atts[1]);
				val = 0;
			}
			size->margin_top = (unsigned char)val;
		}
		atts += 2;
	}
}

void parse_screen_mode(vkb_keyboard_layout *layout, const XML_Char *name, const XML_Char **atts)
{
	unsigned char *mode;
	add_screen(layout);
	mode = get_screen(layout);
	(void)name;
	while (atts && *atts)
	{
		if (strncmp(*atts, "name", 4))
		{
			printf("Unknown attribute for 'screen': %s\n", *atts);
		}
		else
		{
			if (strncmp(atts[1], "NORMAL", 6))
			{
				if (strncmp(atts[1], "FULL", 4))
				{
					if (strncmp(atts[1], "ROTATED", 7))
					{
						printf("Unknown screen mode: %s\n", atts[1]);
					}
					else
					{
						*mode = 2;
					}
				}
				else
				{
					*mode = 1;
				}
			}
			else
			{
				*mode = 0;
			}
		}
		atts += 2;
	}
}

void parse_key_section(vkb_keyboard_layout *layout, const XML_Char *name, const XML_Char **atts)
{
	add_keysection(layout);
	vkb_key_section *section = get_keysection(layout);
	(void)name;
	while (atts && *atts)
	{
		if (strncmp(*atts, "margin_top", 10))
		{
			if (strncmp(*atts, "margin_left", 11))
			{
				if (strncmp(*atts, "margin_bottom", 13))
				{
					if (strncmp(*atts, "margin_right", 12))
					{
						printf("Unknown attribute for 'keysection': %s\n", *atts);
					}
					else
					{
						int val = atoi(atts[1]);
						if (val < 0)
						{
							printf("Invalid value for %s %d (%s)\n", *atts, val, atts[1]);
							val = 0;
						}
						section->margin_right = (unsigned char)val;
					}
				}
				else
				{
					int val = atoi(atts[1]);
					if (val < 0)
					{
						printf("Invalid value for %s %d (%s)\n", *atts, val, atts[1]);
						val = 0;
					}
					section->margin_bottom = (unsigned char)val;
				}
			}
			else
			{
				int val = atoi(atts[1]);
				if (val < 0)
				{
					printf("Invalid value for %s %d (%s)\n", *atts, val, atts[1]);
					val = 0;
				}
				section->margin_left = (unsigned char)val;
			}
		}
		else
		{
			int val = atoi(atts[1]);
			if (val < 0)
			{
				printf("Invalid value for %s %d (%s)\n", *atts, val, atts[1]);
				val = 0;
			}
			section->margin_top = (unsigned char)val;
		}
		atts += 2;
	}
}

void parse_xml(vkb_keyboard_layout *userData, const XML_Char *name, const XML_Char **atts)
{
	if (strncmp(name, "row", 3))
	{
		if (strncmp(name, "keyboards", 9))
		{
			if (strncmp(name, "keyboard", 8))
			{
				if (strncmp(name, "keysection", 10))
				{
					if (strncmp(name, "keysizes", 8))
					{
						if (strncmp(name, "multiple", 8))
						{
							if (strncmp(name, "key", 3))
							{
								if (strncmp(name, "mode", 4))
								{
									if (strncmp(name, "sublayout", 9))
									{
										if (strncmp(name, "slide", 5))
										{
											if (strncmp(name, "screen", 6))
											{
												if (strncmp(name, "size", 4))
												{
													printf("Unknown tag: %s\n", name);
												}
												else
												{
													parse_key_size(userData, name, atts);
												}
											}
										}
										else
										{
											userData->is_slide_key = 1;
										}
									}
									else
									{
										parse_sub_layout(userData, name, atts);
									}
								}
								else
								{
									parse_screen_mode(userData, name, atts);
								}
							}
							else
							{
								userData->is_key = 1;
								parse_key(userData, name, atts);
							}
						}
						else
						{
							userData->is_multiple_key = 1;
							parse_multiple(userData, name, atts);
						}
					}
				}
				else
				{
					parse_key_section(userData, name, atts);
				}
			}
			else
			{
				userData->is_key = 0;
				userData->is_slide_key = 0;
				parse_keyboard(userData, name, atts);
			}
		}
		else
		{
			parse_keyboards(userData, name, atts);
		}
	}
	else
	{
		add_rows(userData);
	}
}

void parse_xml_end(vkb_keyboard_layout *userData, const XML_Char *name)
{
	if (strncmp(name, "keyboards", 9))
	{
		if (strncmp(name, "key", 3))
		{
			if (strncmp(name, "slide", 5))
			{
				if (!strncmp(name, "multiple", 8))
				{
					userData->is_multiple_key = 0;
				}
			}
			else
			{
				userData->is_slide_key = 0;
			}
		}
		else
		{
			userData->is_key = 0;
		}
	}
	else
	{
		if (userData->file)
		{
			open_vkb_file(userData);
			write_vkb_layouts(userData, (write_vkb_file(userData, 0) + 2 * userData->layout.num_layouts + 20));
			write_vkb_file(userData, 1);
			close_vkb_file(userData);
			print_info(&userData->layout);
		}
		else
		{
			printf("file attribute is not given.\n");
		}
	}
}

void cdata_handler(vkb_keyboard_layout *userData, const XML_Char *s, int len)
{
	vkb_key *key;
	if (userData->is_key == 1)
	{
		key = get_key(userData);
		if (key->key_type == 1)
		{
			if (userData->is_slide_key == 1)
			{
				add_slide(userData, s, len);
			}
		}
		else
		{
			key->byte_count = len;
			key->labels = (gchar **)g_strndup(s, len);
		}
	}
}

void setup_xml_parser(const char *filename)
{
	vkb_keyboard_layout layout;
	clear_vkb_keyboard_layout(&layout);
	int fd = open(filename,0);
	if (fd > 0)
	{
		XML_Parser parser = XML_ParserCreate(0);
		XML_SetElementHandler(parser,(XML_StartElementHandler)parse_xml,(XML_EndElementHandler)parse_xml_end);
		XML_SetCharacterDataHandler(parser,(XML_CharacterDataHandler)cdata_handler);
		XML_SetUserData(parser, &layout);
		int len;
		do
		{
			void *buf = XML_GetBuffer(parser, 1024);
			if (!buf)
			{
				g_log(0, G_LOG_LEVEL_ERROR, "Couldn't allocate expat buffer");
				break;
			}
			len = read(fd, buf, 1024);
			if (!XML_ParseBuffer(parser, len, len == 0))
			{
				g_log(0, G_LOG_LEVEL_ERROR, "Error parsing xml %s",XML_ErrorString(XML_GetErrorCode(parser)));
			}
		} while (len);
		XML_ParserFree(parser);
		close(fd);
	}
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: %s file.xml\n", *argv);
		exit(0);
	}
	setup_xml_parser(argv[1]);
	return 0;
}
