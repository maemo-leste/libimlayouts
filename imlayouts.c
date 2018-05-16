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
#include <assert.h>
#include "imlayouts.h"

#define fread_check(var, fp) (fread(&var, sizeof(var), 1, fp) == sizeof(var))
#define fread_check_size(var, size, fp) (fread(&var, size, 1, fp) == size)

static gchar *
imlayout_vkb_get_file_layout(const gchar *fname, const gchar *path)
{
  gchar *fname_abs;
  vkb_layout_collection *coll;
  gchar *layout = NULL;

  if (path)
    fname_abs = g_strdup_printf("%s/%s", path, fname);
  else
    fname_abs = g_strdup_printf("%s/%s", "/usr/share/keyboards", fname);

  coll = imlayout_vkb_load_file(fname_abs);

  if (coll)
  {
    gchar *name = coll->name;

    if (name && *name && coll->lang)
      layout = g_strdup(coll->lang);

    imlayout_vkb_free_layout_collection(coll);
  }

  g_free(fname_abs);

  return layout;
}

GSList *
imlayout_vkb_get_layout_list_from_prefix(const gchar *path)
{
  GDir *dir;
  GSList *l = NULL;
  const gchar *name;

  if (!(dir = g_dir_open(path, 0, NULL)))
    return NULL;

  while ((name = g_dir_read_name(dir)))
  {
    if (g_str_has_suffix(name, ".vkb"))
    {
      gchar *lang = imlayout_vkb_get_file_layout(name, path);

      if (lang)
        l = g_slist_append(l, lang);
    }
  }

  g_dir_close(dir);

  return l;
}

void
add_keysection(vkb_keyboard_layout *layout_name)
{
  vkb_sub_layout *sublayout = get_sublayout(layout_name);
  vkb_key_section *keysection = g_renew(vkb_key_section,
                                        sublayout->key_sections,
                                        (sublayout->num_key_sections + 1));

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

void
add_keysize(vkb_keyboard_layout *layout)
{
  assert(0);
  //todo
}

void
add_sublayout(vkb_keyboard_layout *layout)
{
  vkb_layout *layout2 = get_layout(layout);
  vkb_sub_layout *sublayout = g_renew(vkb_sub_layout, layout2->sub_layouts,
                                      (layout2->num_sub_layouts + 1));
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
  assert(0);
  //todo
}

void add_slide(vkb_keyboard_layout *layout, const char *src, int len)
{
  assert(0);
  //todo
}

void
add_layout(vkb_keyboard_layout *layout)
{
  vkb_layout *layout2 = g_renew(vkb_layout, layout->layouts,
                                (layout->layout.num_layouts + 1));

  layout->layouts = layout2;

  if (layout2)
  {
    vkb_layout *newlayout = &layout2[layout->layout.num_layouts];

    newlayout->type = LAYOUT_TYPE_NORMAL;
    newlayout->sub_layouts = NULL;
    newlayout->numeric = 0;
    newlayout->num_sub_layouts = 0;
    ++layout->layout.num_layouts;
  }
}

unsigned char *
get_num_keys_in_row(vkb_keyboard_layout *layout)
{
  vkb_key_section *section = get_keysection(layout);
  unsigned char *result = section->num_keys_in_rows;

  if (result)
    result = &result[section->num_rows - 1];

  return result;
}

void
imlayout_vkb_free_layout(vkb_layout *layout)
{
  assert(0);
  //todo
}

void
add_screen(vkb_keyboard_layout *layout)
{
  unsigned char *modes = g_renew(unsigned char, layout->layout.screen_modes,
                                 (layout->layout.num_screen_modes + 1));

  layout->layout.screen_modes = modes;

  if (modes)
    modes[layout->layout.num_screen_modes++] = 0;
  else
    printf("No memory %s\n", __FUNCTION__);
}

int
imlayout_vkb_init_buttons(vkb_layout_collection *collection,
                          vkb_layout *section, int width, int height)
{
  assert(0);
  return 0;
  //todo
}

vkb_key_section *
get_keysection(vkb_keyboard_layout *layout)
{
  vkb_sub_layout *sublayout = get_sublayout(layout);
  vkb_key_section *result = sublayout->key_sections;

  if (result)
    result = &result[sublayout->num_key_sections - 1];

  return result;
}

void
add_rows(vkb_keyboard_layout *layout)
{
  vkb_key_section *section = get_keysection(layout);
  unsigned char count = section->num_rows + 1;
  unsigned char *rows = section->num_keys_in_rows;

  section->num_rows = count;
  section->num_keys_in_rows = g_renew(unsigned char, rows, count);
  *get_num_keys_in_row(layout) = 0;
}

vkb_key_size *
get_keysize(vkb_keyboard_layout *layout)
{
  vkb_key_size *result = layout->layout.key_sizes;

  if (result)
    result = &result[layout->layout.num_key_sizes - 1];

  return result;
}

vkb_key *
get_key0(vkb_keyboard_layout *layout, int no_subkey)
{
  if (no_subkey == 1)
  {
    vkb_key_section *keysection = get_keysection(layout);
    return &keysection->keys[keysection->num_keys - 1];
  }
  else
  {
    vkb_key *key = get_key0(layout, 1);

    return &key->sub_keys[key->num_sub_keys - 1];
  }
}

vkb_sub_layout *
get_sublayout(vkb_keyboard_layout *layout)
{
  vkb_layout *layout2 = get_layout(layout);
  vkb_sub_layout *result = layout2->sub_layouts;

  if (result)
    result = &result[layout2->num_sub_layouts - 1];

  return result;
}

int
vkb_init_buttons(vkb_layout_collection *collection, vkb_layout *section)
{
  assert(0);
  return 0;
  //todo
}

GSList *
imlayout_vkb_get_layout_list()
{
  assert(0);
  return NULL;
  //todo
}

vkb_layout *
imlayout_vkb_get_layout(vkb_layout_collection *collection, int layout_type)
{
  assert(0);
  return NULL;
  //todo
}

void
print_sublayout_info(vkb_sub_layout *layout)
{
  assert(0);
  //todo
}

void
imlayout_vkb_free_layout_collection(vkb_layout_collection *coll)
{
  if (coll)
  {
    if (coll->name)
      g_free(coll->name);

    if (coll->lang)
      g_free(coll->lang);

    if (coll->wc)
      g_free(coll->wc);

    if (coll->layout_types)
      g_free(coll->layout_types);

    if (coll->offsets)
      g_free(coll->offsets);

    if (coll->offsets)
      g_free(coll->offsets);

    if (coll->key_sizes)
    {
      g_free(coll->key_sizes);
      coll->key_sizes = NULL;
    }

    if (coll->screen_modes)
      g_free(coll->screen_modes);

    g_free(coll);
  }
}

static vkb_layout_collection *
read_header(FILE *fp)
{
  vkb_layout_collection *coll;
  char unk[20];
  unsigned char c;
  int i;

  g_return_val_if_fail(fp != NULL, NULL);

  coll = g_new0(vkb_layout_collection, 1);

  if (!coll)
  {
    g_log(0, G_LOG_LEVEL_WARNING,
          "insufficient memory to allocate vkb_layout_collection");
    return NULL;
  }

  if (!fread_check(c, fp))
  {
    g_log(0, G_LOG_LEVEL_WARNING, "error when reading version");
    goto cleanup;
  }

  if (c != 1)
  {
    g_log(0, G_LOG_LEVEL_WARNING, "invalid version");
    goto cleanup;
  }

  if (!fread_check(coll->num_layouts, fp))
  {
    g_log(0, G_LOG_LEVEL_WARNING, "error reading number of layout");
    goto cleanup;
  }

  if (!coll->num_layouts)
  {
    g_log(0, G_LOG_LEVEL_WARNING, "number of layout <= 0");
    goto cleanup;
  }

  coll->layout_types = g_new0(int, coll->num_layouts);

  if (!coll->layout_types)
  {
    g_log(0, G_LOG_LEVEL_WARNING,
          "insufficient memory allocation layout collection");
    goto cleanup;
  }

  if (!fread_check(c, fp))
  {
    g_log(0, G_LOG_LEVEL_WARNING, "error reading numeric layout");
    goto cleanup;
  }

  coll->name = g_new0(char, c + 1);

  if (!coll->name)
  {
    g_log(0, G_LOG_LEVEL_WARNING, "insufficient memory allocating name");
    goto cleanup;
  }

  if (!fread_check_size(coll->name, c, fp))
  {
    g_log(0, G_LOG_LEVEL_WARNING, "error reading name");
    goto cleanup;
  }

  if (!fread_check(c, fp))
  {
    g_log(0, G_LOG_LEVEL_WARNING, "error reading length of lang");
    goto cleanup;
  }

  if ( (c & 0x80))
  {
    coll->special_font = 1;
    c &= ~0x80;
  }
  else
    coll->special_font = 0;

  coll->lang = g_new0(char, c + 1);

  if (!coll->lang)
  {
    g_log(0, G_LOG_LEVEL_WARNING, "insufficient memory allocating language");
    goto cleanup;
  }

  if (!fread_check_size(coll->lang, c, fp))
  {
    g_log(0, G_LOG_LEVEL_WARNING, "error reading language");
    goto cleanup;
  }

  if (!fread_check(c, fp))
  {
    g_log(0, G_LOG_LEVEL_WARNING, "error reading length of wc");
    goto cleanup;
  }

  if (c)
  {
    coll->wc = g_new0(char, c + 1);

    if (!coll->wc)
    {
      g_log(0, G_LOG_LEVEL_WARNING, "insufficient memory allocating wc");
      goto cleanup;
    }
  }

  if (!fread_check_size(coll->wc, c, fp))
  {
    g_log(0, G_LOG_LEVEL_WARNING, "error reading wc");
    goto cleanup;
  }

  if (!fread_check(coll->num_screen_modes, fp))
  {
    g_log(0, G_LOG_LEVEL_WARNING, "error reading num_screen_modes");
    goto cleanup;
  }

  if (!coll->num_screen_modes )
  {
    g_log(0, G_LOG_LEVEL_WARNING, "num_screen_modes == 0");
    goto cleanup;
  }

  coll->screen_modes = g_new0(unsigned char, coll->num_screen_modes);

  if (!coll->screen_modes)
  {
    g_log(0, G_LOG_LEVEL_WARNING,
          "insufficient memory allocating screen_modes");
    goto cleanup;
  }

  if (!fread_check_size(coll->screen_modes, coll->num_screen_modes, fp))
  {
    g_log(0, G_LOG_LEVEL_WARNING, "error reading screen_modes");
    goto cleanup;
  }

  if (!fread_check(coll->num_key_sizes, fp))
  {
    g_log(0, G_LOG_LEVEL_WARNING, "error reading num_key_sizes");
    goto cleanup;
  }

  if (!coll->num_key_sizes)
  {
    g_log(0, G_LOG_LEVEL_WARNING, "num_key_sizes == 0");
    goto cleanup;
  }

  coll->key_sizes = g_new0(vkb_key_size, coll->num_key_sizes);

  if (!coll->key_sizes)
  {
    g_log(0, G_LOG_LEVEL_WARNING, "insufficient memory allocating key sizes");
    goto cleanup;
  }

  for (i = 0; i < coll->num_key_sizes; i++)
  {
    vkb_key_size *key_size = &coll->key_sizes[i];

    if (!fread_check(key_size->width, fp) ||
        !fread_check(key_size->height, fp) ||
        !fread_check(key_size->baseline, fp) ||
        !fread_check(key_size->margin_left, fp) ||
        !fread_check(key_size->margin_top, fp))
    {
      g_log(0, G_LOG_LEVEL_WARNING, "error reading key_size");
      goto cleanup;
    }
  }

  coll->offsets = g_new0(unsigned short, coll->num_layouts);

  if (!coll->offsets)
  {
    g_log(0, G_LOG_LEVEL_WARNING, "insufficient memory allocating offsets");
    goto cleanup;
  }

  if (!fread_check_size(coll->offsets,
                        sizeof(coll->offsets[0]) * coll->num_layouts, fp))
  {
    g_log(0, G_LOG_LEVEL_WARNING, "error reading offsets");
    goto cleanup;
  }

  /* what are those 20 bytes ? */
  if (fread_check(unk, fp))
  {
    goto cleanup;
  }

  return coll;

cleanup:
  imlayout_vkb_free_layout_collection(coll);

  return NULL;
}

vkb_layout_collection *
imlayout_vkb_load_file(const char *fname)
{
  FILE *fp;
  vkb_layout_collection *coll;
  int i;

  if (!(fp = fopen(fname, "rb")))
  {
    g_log(0, G_LOG_LEVEL_WARNING, "couldn't open vkb layout file: %s", fname);
    return NULL;
  }

  coll = read_header(fp);

  if (!coll)
    goto cleanup;

  coll->filename = g_strdup(fname);

  for (i = 0; i < coll->num_layouts; i++)
  {
    if (!fseek(fp, coll->offsets[i], SEEK_SET))
    {
      char type;

      coll->layout_types[i] = LAYOUT_TYPE_NORMAL;


      if (!fread_check(type, fp))
      {
        g_log(0, G_LOG_LEVEL_WARNING, "error reading layout type");
        goto cleanup;
      }

      coll->layout_types[i] = type;
    }
  }

cleanup:
  if (coll)
  {
    imlayout_vkb_free_layout_collection(coll);
    coll = NULL;
  }

  fclose(fp);

  return coll;
}

static void
imlayout_vkb_free_entry(void *entry)
{
  g_free(entry);
}

void
imlayout_vkb_free_layout_list(GSList *list)
{
  if (list)
  {
    g_slist_foreach(list, (GFunc)imlayout_vkb_free_entry, NULL);
    g_slist_free(list);
  }
}

vkb_layout *
get_layout(vkb_keyboard_layout *layout)
{
  vkb_layout *result = layout->layouts;

  if (result)
    result = &result[layout->layout.num_layouts - 1];

  return result;
}

unsigned char *
get_screen(vkb_keyboard_layout *layout)
{
  unsigned char *result = layout->layout.screen_modes;

  if (result)
    result = &result[layout->layout.num_screen_modes - 1];

  return result;
}

void
add_key(vkb_keyboard_layout *layout)
{
  vkb_key_section *keysection = get_keysection(layout);

  if (layout->is_multiple_key == 1)
  {
    if (layout->parsing_multiple == 1)
    {
      vkb_key *key = g_renew(vkb_key, keysection->keys,
                             (keysection->num_keys + 1));

      keysection->keys = key;

      if (key)
      {
        vkb_key *newkey = &key[keysection->num_keys];

        newkey->scancode_length = 0;
        newkey->key_type = KEY_TYPE_HEXA;
        newkey->byte_count = 0;
        newkey->num_sub_keys = 0;
        newkey->special_font = 0;
        newkey->key_flags = 0;
        newkey->scancode = 0;
        newkey->label = 0;
        newkey->sub_keys = 0;
        keysection->num_keys++;
      }
    }
    else
    {
      vkb_key *key = get_key0(layout, 1);
      vkb_key *newkey =
          g_renew(vkb_key, key->sub_keys, (key->num_sub_keys + 1));

      key->sub_keys = newkey;

      if (newkey)
        key->num_sub_keys++;
    }
  }
  else
  {
    vkb_key *key =
        g_renew(vkb_key, keysection->keys, (keysection->num_keys + 1));

    keysection->keys = key;

    if (key)
      keysection->num_keys++;
  }
}

void increase_num_keys_in_row(vkb_keyboard_layout *layout)
{
  unsigned char *keys = get_num_keys_in_row(layout);

  if (keys)
    ++*keys;
}

vkb_key *
get_key(vkb_keyboard_layout *layout)
{
  if (layout->is_multiple_key != 1 || layout->parsing_multiple == 1)
    return get_key0(layout, 1);
  else
    return get_key0(layout, 0);
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
    puts("\tLayout is empty");
}
