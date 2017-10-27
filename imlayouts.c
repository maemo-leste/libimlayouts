/**
  @file imlayouts.c

  Copyright (C) 2017 Jonathan Wilson

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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "imlayouts.h"

GSList *imlayout_vkb_get_layout_list_from_prefix(const gchar *path)
{
  return NULL;
  //todo
}

void add_keysection(vkb_keyboard_layout *layout_name)
{
  vkb_sub_layout *sublayout = get_sublayout(layout_name);
  vkb_key_section *keysection = g_renew(vkb_key_section, sublayout->key_sections, sublayout->num_key_sections + 1);
  sublayout->key_sections = keysection;
  if (keysection)
  {
    
  }
}

void imlayout_vkb_free_layout(vkb_layout *layout);
void imlayout_vkb_free_layout_collection(vkb_layout_collection *collection);
void imlayout_vkb_free_layout_list(GSList *list);
vkb_layout *imlayout_vkb_get_layout(vkb_layout_collection *collection, int layout_type);
GSList *imlayout_vkb_get_layout_list();
signed int imlayout_vkb_init_buttons(vkb_layout_collection *collection, vkb_layout *section, int width, int height);
vkb_layout_collection *imlayout_vkb_load_file(const char *fname);
void print_info(vkb_layout_collection *collection);
void print_layout_info(vkb_layout *layout);
void add_key(vkb_keyboard_layout *layout);
void add_keysize(vkb_keyboard_layout *layout);
void add_layout(vkb_keyboard_layout *layout);
void add_rows(vkb_keyboard_layout *layout);
void add_screen(vkb_keyboard_layout *layout);
void add_slide(vkb_keyboard_layout *layout, const char *src, int len);
void add_sublayout(vkb_keyboard_layout *layout);
vkb_key *get_key(vkb_keyboard_layout *layout);
vkb_key *get_key0(vkb_keyboard_layout *layout, int no_subkey);
vkb_key_section *get_keysection(vkb_keyboard_layout *layout);
vkb_key_size *get_keysize(vkb_keyboard_layout *layout);
vkb_layout *get_layout(vkb_keyboard_layout *layout);
unsigned char *get_num_keys_in_row(vkb_keyboard_layout *layout);
unsigned char *get_screen(vkb_keyboard_layout *layout);
vkb_sub_layout *get_sublayout(vkb_keyboard_layout *layout);
void increase_num_keys_in_row(vkb_keyboard_layout *layout);
