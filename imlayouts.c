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
  vkb_key_section *keysection = g_renew(vkb_key_section, sublayout->key_sections, (sublayout->num_key_sections + 1));
  sublayout->key_sections = keysection;
  if (keysection)
  {
    vkb_key_section *newkeysection = &keysection[sublayout->num_key_sections];
    newkeysection->num_keys = 0;
    newkeysection->margin_top = 0;
    newkeysection->num_rows = 0;
    newkeysection->num_keys_in_rows = 0;
    newkeysection->keys = 0;
    newkeysection->margin_right = 0;
    newkeysection->margin_bottom = 0;
    newkeysection->margin_left = 0;
    ++sublayout->num_key_sections;
  }
}

void add_keysize(vkb_keyboard_layout *layout)
{
  //todo
}

void add_sublayout(vkb_keyboard_layout *layout)
{
  vkb_layout *layout2 = get_layout(layout);
  vkb_sub_layout *sublayout = g_renew(vkb_sub_layout, layout2->sub_layouts, (layout2->num_sub_layouts + 1));
  layout2->sub_layouts = sublayout;
  if (sublayout)
  {
    vkb_sub_layout *newsublayout = &sublayout[layout2->num_sub_layouts];
    newsublayout->type = 0;
    newsublayout->key_sections = 0;
    newsublayout->label = 0;
    newsublayout->num_key_sections = 0;
    ++layout2->num_sub_layouts;
  }
}

void print_info(vkb_layout_collection *collection)
{
  //todo
}

void add_slide(vkb_keyboard_layout *layout, const char *src, int len)
{
  //todo
}

void add_layout(vkb_keyboard_layout *layout)
{
  vkb_layout *layout2 = g_renew(vkb_layout, layout->layouts, (layout->layout.num_layouts + 1));
  layout->layouts = layout2;
  if (layout2)
  {
    vkb_layout *newlayout = &layout2[layout->layout.num_layouts];
    newlayout->type = 0;
    newlayout->sub_layouts = 0;
    newlayout->numeric = 0;
    newlayout->num_sub_layouts = 0;
    ++layout->layout.num_layouts;
  }
}

unsigned char *get_num_keys_in_row(vkb_keyboard_layout *layout)
{
  vkb_key_section *section = get_keysection(layout);
  unsigned char *result = section->num_keys_in_rows;
  if (result)
    result = &result[section->num_rows - 1];
  return result;
}

void imlayout_vkb_free_layout(vkb_layout *layout)
{
  //todo
}

void add_screen(vkb_keyboard_layout *layout)
{
  unsigned char *modes = g_renew(unsigned char, layout->layout.screen_modes, (layout->layout.num_screen_modes + 1));
  layout->layout.screen_modes = modes;
  if (modes)
    modes[layout->layout.num_screen_modes++] = 0;
  else
    printf("No memory %s\n", "add_screen");
}

int imlayout_vkb_init_buttons(vkb_layout_collection *collection, vkb_layout *section, int width, int height)
{
  return 0;
  //todo
}

vkb_key_section *get_keysection(vkb_keyboard_layout *layout)
{
  vkb_sub_layout *sublayout = get_sublayout(layout);
  vkb_key_section *result = sublayout->key_sections;
  if (result)
    result = &result[sublayout->num_key_sections - 1];
  return result;
}

void add_rows(vkb_keyboard_layout *layout)
{
  vkb_key_section *section = get_keysection(layout);
  unsigned char count = section->num_rows + 1;
  unsigned char *rows = section->num_keys_in_rows;
  section->num_rows = count;
  section->num_keys_in_rows = g_renew(unsigned char, rows, count);
  *get_num_keys_in_row(layout) = 0;
}

vkb_key_size *get_keysize(vkb_keyboard_layout *layout)
{
  vkb_key_size *result = layout->layout.key_sizes;
  if (result)
    result = &result[layout->layout.num_key_sizes - 1];
  return result;
}

vkb_key *get_key0(vkb_keyboard_layout *layout, int no_subkey)
{
  vkb_key *result;
  if (no_subkey == 1)
  {
    vkb_key_section *keysection = get_keysection(layout);
    result = &keysection->keys[keysection->num_keys - 1];
  }
  else
  {
    vkb_key *key = get_key0(layout, 1);
    result = &key->sub_keys[key->num_sub_keys - 1];
  }
  return result;
}

vkb_sub_layout *get_sublayout(vkb_keyboard_layout *layout)
{
  vkb_layout *layout2 = get_layout(layout);
  vkb_sub_layout *result = layout2->sub_layouts;
  if (result)
    result = &result[layout2->num_sub_layouts - 1];
  return result;
}

int vkb_init_buttons(vkb_layout_collection *collection, vkb_layout *section)
{
  return 0;
  //todo
}

GSList *imlayout_vkb_get_layout_list()
{
  return NULL;
  //todo
}

vkb_layout *imlayout_vkb_get_layout(vkb_layout_collection *collection, int layout_type)
{
  return NULL;
  //todo
}

void print_sublayout_info(vkb_sub_layout *layout)
{
  //todo
}

//read_header (cant figure out its prototype)
//todo

void imlayout_vkb_free_layout_collection(vkb_layout_collection *collection)
{
  if (collection)
  {
    char *v1;
    char *v2;
    char *v3;
    int *v4;
    unsigned short *v5;
    char *v6;
    vkb_key_size *v7;
    unsigned char *v8;
    v1 = collection->name;
    if ( v1 )
      g_free(v1);
    v2 = collection->lang;
    if ( v2 )
      g_free(v2);
    v3 = collection->wc;
    if ( v3 )
      g_free(v3);
    v4 = collection->layout_types;
    if ( v4 )
      g_free(v4);
    v5 = collection->offsets;
    if ( v5 )
      g_free(v5);
    v6 = collection->filename;
    if ( v6 )
      g_free(v6);
    v7 = collection->key_sizes;
    if ( v7 )
    {
      g_free(v7);
      collection->key_sizes = 0;
    }
    v8 = collection->screen_modes;
    if ( v8 )
      g_free(v8);
    g_free(collection);
  }
}

vkb_layout_collection *imlayout_vkb_load_file(const char *fname)
{
  return NULL;
  //todo
}

static void imlayout_vkb_free_entry(void *entry)
{
  g_free(entry);
}

void imlayout_vkb_free_layout_list(GSList *list)
{
  if (list)
  {
    g_slist_foreach(list, (GFunc)imlayout_vkb_free_entry, 0);
    g_slist_free(list);    
  }
}

vkb_layout *get_layout(vkb_keyboard_layout *layout)
{
  vkb_layout *result = layout->layouts;
  if (result)
    result = &result[layout->layout.num_layouts - 1];
  return result;
}

unsigned char *get_screen(vkb_keyboard_layout *layout)
{
  unsigned char *result = layout->layout.screen_modes;
  if (result)
    result = &result[layout->layout.num_screen_modes - 1];
  return result;
}

void add_key(vkb_keyboard_layout *layout)
{
  vkb_key_section *keysection = get_keysection(layout);
  if (layout->is_multiple_key == 1)
  {
    if (layout->parsing_multiple == 1)
    {
      vkb_key *key = g_renew(vkb_key, keysection->keys, (keysection->num_keys + 1));
      keysection->keys = key;
      if (key)
      {
        vkb_key *newkey = &key[keysection->num_keys];
        newkey->scancode_length = 0;
        newkey->key_type = 4;
        newkey->byte_count = 0;
        newkey->num_sub_keys = 0;
        newkey->special_font = 0;
        unsigned int i = keysection->num_keys;
        newkey->key_flags = 0;
        newkey->scancode = 0;
        keysection->num_keys = i + 1;
        newkey->label = 0;
        newkey->sub_keys = 0;
      }
    }
    else
    {
      vkb_key *key2 = get_key0(layout, 1);
      vkb_key *newkey2 = g_renew(vkb_key, key2->sub_keys, (key2->num_sub_keys + 1));
      key2->sub_keys = newkey2;
      if ( newkey2 )
        ++key2->num_sub_keys;
    }
  }
  else
  {
    vkb_key *key3 = g_renew(vkb_key, keysection->keys, (keysection->num_keys + 1));
    keysection->keys = key3;
    if ( key3 )
      ++keysection->num_keys;
  }
}

void increase_num_keys_in_row(vkb_keyboard_layout *layout)
{
  unsigned char *keys = get_num_keys_in_row(layout);
  if (keys)
    ++*keys;
}

vkb_key *get_key(vkb_keyboard_layout *layout)
{
  if (layout->is_multiple_key != 1)
    return get_key0(layout, 1);
  int i = layout->parsing_multiple;
  if ( i != 1 )
    i = 0;
  return get_key0(layout, i);
}

void print_layout_info(vkb_layout *layout)
{
  if (layout)
  {
    printf("\tType:\t\t\t%d\n", layout->type);
    printf("\tNumeric Layout:\t\t%02X\n", layout->numeric);
    printf("\tNum Layouts:\t\t%d\n", layout->num_sub_layouts);
    if (layout->num_sub_layouts)
    {
      vkb_sub_layout *sublayout = layout->sub_layouts;
      if (sublayout)
      {
        int count = 0;
        do
        {
          printf("\t\tSublayout #%d ..........\n", count);
          print_sublayout_info(sublayout);
          putchar('\n');
          ++count;
          ++sublayout;
        }
        while (layout->num_sub_layouts > count);
      }
    }
  }
  else
  {
    puts("\tLayout is empty");
  }
}
