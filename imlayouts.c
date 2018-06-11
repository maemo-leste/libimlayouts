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
#include <string.h>
#include <glib.h>
#include <assert.h>

#include "imlayouts.h"

#define fread_check(var, fp) (fread(&var, sizeof(var), 1, fp) == 1)
#define fread_check_size(var, size, fp) (!size || (fread(var, size, 1, fp) == 1))

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
  layout->layout.key_sizes = g_renew(vkb_key_size, layout->layout.key_sizes,
                                     layout->layout.num_key_sizes + 1);
  if (layout->layout.key_sizes)
  {
    memset(&layout->layout.key_sizes[layout->layout.num_key_sizes], 0,
           sizeof(layout->layout.key_sizes[0]));
    layout->layout.num_key_sizes++;
  }
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

void
print_info(vkb_layout_collection *collection)
{
  int i;

  if (!collection)
  {
    puts("Layout collection is empty");
    return;
  }

  printf("Name:\t\t\t%s\n", collection->name);
  printf("Language:\t\t%s\n", collection->lang);
  printf("WC:\t\t\t%s\n", collection->wc);
  printf("Filename:\t\t%s\n", collection->filename);
  printf("Num Layouts:\t\t%d\n", collection->num_layouts);
  printf("Screen Modes:\t\t");

  for (i = 0; i < collection->num_screen_modes;i++)
  {
    switch (collection->screen_modes[i])
    {
      case SCREEN_TYPE_NORMAL:
        printf(" NORMAL ");
        break;
      case SCREEN_TYPE_THUMB:
        printf(" FULL ");
        break;
      case SCREEN_TYPE_ROTATED:
        printf(" ROTATED ");
        break;
      default:
        printf(" UNKNOWN ");
        break;
    }
  }

  putchar('\n');
  puts("Key sizes:");

  for (i = 0; i < collection->num_key_sizes; i++)
  {
    vkb_key_size *key_size = &collection->key_sizes[i];

    printf("Width %d, Height %d, Margins: %d:%d\n",
           key_size->width, key_size->height, key_size->margin_left,
           key_size->margin_top);
  }

  putchar('\n');
  printf("Use special font:\t%s\n", collection->special_font ? "Yes" : "No");

  if (collection->num_layouts && collection->offsets)
  {
    printf("Offsets:\t\t");

    for (i = 0; i < collection->num_layouts; i++)
      printf("%04X ", collection->offsets[i]);

    putchar('\n');
  }
}

void
add_slide(vkb_keyboard_layout *layout, const char *src, int len)
{
  vkb_key *key = get_key(layout);

  key->labels = g_renew(gchar *, key->labels, key->byte_count + 1);
  key->labels[key->byte_count] = g_malloc0(len + 1);
  memcpy(key->labels[key->byte_count], src, len);
  key->byte_count++;
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

static int
init_buttons(vkb_layout_collection *coll, vkb_sub_layout *sub_layout,
             int width, int height, int flag)
{
  unsigned int hash;
  int left;
  int key_left;
  int key_top;
  int key_end;
  vkb_key_size *key_size;
  int key_right;
  vkb_key_section *key_section;
  vkb_key_size *key_sizes;
  vkb_key *key;
  int key_idx;
  unsigned int key_width;
  unsigned int key_height;
  unsigned char *num_keys_in_rows;
  unsigned int top;
  unsigned int max_width;
  unsigned int max_height;
  unsigned int max_section_width;
  unsigned int margin_height;
  unsigned int section_width;
  unsigned int max_key_height;
  int section_idx;
  unsigned int margin_left;
  unsigned int margin_top;
  unsigned int margin_bottom;
  unsigned int margin_right;

  if (flag)
  {
    key_sizes = coll->key_sizes;
    key_width = 0;
    key_height = 0;
  }
  else
  {
    int i;

    key_width = (width << 16) / sub_layout->width;
    key_height = (height << 16) / sub_layout->height;
    key_sizes = g_new(vkb_key_size, coll->num_key_sizes);
    memcpy(key_sizes, coll->key_sizes,
           coll->num_key_sizes * sizeof(coll->key_sizes[0]));

    for (i = 0; i < coll->num_key_sizes; i++)
    {
      vkb_key_size *key_size = &key_sizes[i];

      key_size->margin_left = key_width * key_size->margin_left >> 16;
      key_size->width = key_width * key_size->width >> 16;
      key_size->margin_top = key_height * key_size->margin_top >> 16;
      key_size->height = key_height * key_size->height >> 16;
      key_size->baseline = key_height * key_size->baseline >> 16;
    }
  }

  key_section = sub_layout->key_sections;

  if (sub_layout->num_key_sections)
  {
    section_idx = 0;
    hash = 0;
    max_width = 0;
    max_height = 0;
    max_key_height = 0;
    key = key_section->keys;

    if (flag)
      goto init_margins;

loop:
    margin_left = key_width * key_section->margin_left >> 16;
    margin_right = key_width * key_section->margin_right >> 16;
    margin_top = key_height * key_section->margin_top >> 16;
    margin_bottom = key_height * key_section->margin_bottom >> 16;

    while (1)
    {
      max_section_width = margin_left + margin_right;
      margin_height = margin_top + margin_bottom;
      num_keys_in_rows = key_section->num_keys_in_rows;
      hash += max_width + margin_left;

      if (key_section->num_keys)
      {
        int i;
        top = margin_top;
        left = max_width + margin_left;
        section_width = margin_left + margin_right;

        key_idx = 0;

        for (i = 0; i < key_section->num_keys; i++)
        {
          key = &key_section->keys[i];

          if (key->key_size >= coll->num_key_sizes)
          {
            g_warning("Invalid key size %d", key->key_size);
            key_size = &key_sizes[0];
          }
          else
            key_size = &key_sizes[key->key_size];

          key_left = left + key_size->margin_left;
          key->left = key_left;
          key_right = key_size->width + key_left;
          key->right = key_right;

          key_top = key_size->margin_top;
          key_height = key_size->height + key_top;
          key->top = top + key_top;
          key->bottom = key_height + top;
          key->offset = key_size->baseline;
          key_end = key_size->margin_left + key_size->width;
          section_width += key_end;

          hash += key_end + key_size->margin_top + key_size->height;

          if (max_key_height < key_height)
            max_key_height = key_height;

          if (++key_idx < *num_keys_in_rows)
          {
            left = key_right;
          }
          else
          {
            if (!flag)
            {
              int w =
                  (key_section->width * key_width >> 16) + 1 - section_width;

              if (width < w + key_right)
                w = width - key_right;

              section_width += w;
            }

            if (max_section_width < section_width)
              max_section_width = section_width;

            margin_height += max_key_height;
            top += max_key_height;

            ++num_keys_in_rows;
            left = max_width + margin_left;
            key_idx = 0;
            section_width = 0;
          }
          ++key;
        }
      }

      if (flag == 1)
        key_section->width = max_section_width;

      hash += key_section->margin_left + key_section->margin_right +
          key_section->margin_top + key_section->margin_bottom;

      max_width += max_section_width;

      if ( max_height < margin_height )
        max_height = margin_height;

      if (sub_layout->num_key_sections <= ++section_idx)
        break;

      key_section++;
      key = key_section->keys;

      if (!flag)
        goto loop;

init_margins:
      margin_left = key_section->margin_left;
      margin_top = key_section->margin_top;
      margin_bottom = key_section->margin_bottom;
      margin_right = key_section->margin_right;
    }
  }
  else
  {
    hash = 0;
    max_width = 0;
    max_height = 0;
  }

  if (flag == 1)
  {
    sub_layout->width = max_width;
    sub_layout->height = max_height;
    sub_layout->hash = hash;
  }
  else
    g_free(key_sizes);

  return 0;
}

int
imlayout_vkb_init_buttons(vkb_layout_collection *collection,
                          vkb_layout *section, int width, int height)
{
  int i;

  if (!collection || !section || width <= 0 || height <= 0)
    return -1;

  for (i = 0; i < section->num_sub_layouts; i++)
  {
    vkb_sub_layout *sub_layout = &section->sub_layouts[i];

    if (init_buttons(collection, sub_layout, width, height, 0) == -1)
      return -1;
  }

  return 0;
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
  int i;

  if (!collection || !section)
    return -1;

  for (i = 0; i < section->num_sub_layouts; i++)
  {
    vkb_sub_layout *sub_layout = &section->sub_layouts[i];

    if (init_buttons(collection, sub_layout, 0, 0, 1) == -1)
      return -1;
  }

  return 0;
}

GSList *
imlayout_vkb_get_layout_list()
{
  GDir *dir = g_dir_open("/usr/share/keyboards", 0, NULL);
  GSList *l = NULL;
  const gchar *name;

  if (!dir)
    return NULL;

  while ((name = g_dir_read_name(dir)))
  {
    if (g_str_has_suffix(name, ".vkb"))
    {
      gchar *layout = imlayout_vkb_get_file_layout(name, NULL);

      if (layout)
        l = g_slist_append(l, layout);
    }
  }

  g_dir_close(dir);

  return l;
}

static int
read_key(FILE *fp, vkb_key *key)
{
  if (!fread_check(key->key_type, fp))
    return 0;

  if (key->key_type != KEY_TYPE_MULTIPLE)
  {
    unsigned char c;

    if (!fread_check(c, fp))
      return 0;

    key->key_flags = c;
    if (c & 0x80)
    {
      if (!fread_check(c, fp))
        return 0;

      key->key_flags |= ((unsigned short)c) << 8;
    }

    if (!fread_check(key->byte_count, fp))
      return 0;

    if (key->byte_count & 0x80)
    {
      int i;
      key->byte_count &= ~0x80;
      key->labels = g_new0(gchar *, key->byte_count);

      if (!key->labels)
        return 0;

      key->current_slide_key = 0;

      for (i = 0; i < key->byte_count; i++)
      {
        unsigned char label_len;

        if (!fread_check(label_len, fp))
          return 0;

        if (label_len)
        {
          gchar *label = g_new0(gchar, label_len + 1);

          key->labels[i] = label;

          if (!label)
            return 0;

          if (fread_check_size(label, label_len, fp))
            return 0;
        }
      }
    }
    else
    {
      key->labels = (gchar **)g_new0(gchar, key->byte_count + 1);

      if (!key->labels || !fread_check_size(key->labels, key->byte_count, fp))
        return 0;
    }

    if (!fread_check(key->key_size, fp))
      return 0;

    if (!(key->key_flags & 0x100))
      return 1;

    if (!fread_check(key->scancode_length, fp))
      return 0;

    if (!key->scancode_length)
      return 1;

    key->scancode = g_new0(unsigned char, key->scancode_length);

    if (!key->scancode)
      return 0;

    return fread_check_size(key->scancode, key->scancode_length, fp);
  }
  else
  {
    int i;

    if (!fread_check(key->num_sub_keys, fp))
      return 0;

    key->sub_keys = g_new0(vkb_key, key->num_sub_keys);

    if (!key->num_sub_keys)
      return 1;

    if (!key->sub_keys)
      return 0;

    for (i = 0; i < key->num_sub_keys; i++)
    {
      if (!read_key(fp, &key->sub_keys[i]))
        return 0;
    }

    return 1;
  }
}

vkb_layout *
imlayout_vkb_get_layout(vkb_layout_collection *collection, int layout_type)
{
  __off_t offset = 0;
  FILE *fp;
  vkb_layout *layout;
  int i;

  if (!collection)
    return NULL;

  for (i = 0; i < collection->num_layouts; i++)
  {
    if (collection->layout_types[i] == layout_type)
    {
      offset = collection->offsets[i];
      break;
    }
  }

  if (i == collection->num_layouts)
    return NULL;

  fp = fopen(collection->filename, "rb");

  if (!fp)
    return NULL;

  fseek(fp, offset, SEEK_SET);
  layout = g_new0(vkb_layout, 1);

  if (!layout)
    goto error;

  if (!fread_check(layout->type, fp) ||
      !fread_check(layout->num_sub_layouts, fp) ||
      !fread_check(layout->numeric, fp) ||
      !fread_check(layout->default_key_size, fp))
  {
    goto error;
  }

  layout->sub_layouts = g_new0(vkb_sub_layout, layout->num_sub_layouts);

  if (!layout->sub_layouts)
    goto error;

  for (i = 0; i < layout->num_sub_layouts; i++)
  {
    vkb_sub_layout *sub_layout = &layout->sub_layouts[i];
    char label_len;
    int j;

    if (!fread_check(sub_layout->type, fp) ||
        !fread_check(sub_layout->variance_index, fp) ||
        !fread_check(label_len, fp))
    {
      break;
    }

    if (label_len)
    {
      sub_layout->label = g_new0(char, label_len + 1);

      if (!sub_layout->label ||
          !fread_check_size(sub_layout->label, label_len, fp))
      {
        break;
      }
    }

    if (!fread_check(sub_layout->num_key_sections, fp) ||
        !sub_layout->num_key_sections)
    {
      break;
    }

    sub_layout->key_sections =
        g_new0(vkb_key_section, sub_layout->num_key_sections);

    if (!sub_layout->key_sections)
      break;

    for (j = 0; j < sub_layout->num_key_sections; j++)
    {
      vkb_key_section *key_section = &sub_layout->key_sections[j];
      int k;

      if (!fread_check(key_section->num_keys, fp) ||
          !fread_check(key_section->num_rows, fp) ||
          !fread_check(key_section->margin_left, fp) ||
          !fread_check(key_section->margin_top, fp) ||
          !fread_check(key_section->margin_bottom, fp) ||
          !fread_check(key_section->margin_right, fp))
      {
        goto error;
      }

      key_section->num_keys_in_rows =
          g_new0(unsigned char, key_section->num_rows);

      if (!key_section->num_keys_in_rows)
        goto error;

      if (!fread_check_size(key_section->num_keys_in_rows,
                            key_section->num_rows, fp))
      {
        goto error;
      }

      if (!key_section->num_keys ||
          !(key_section->keys = g_new0(vkb_key, key_section->num_keys)))
      {
        goto error;
      }

      for (k = 0; k < key_section->num_keys; k++)
      {
        vkb_key *key = &key_section->keys[k];

        if (!read_key(fp, key))
          goto error;
      }
    }
  }

  if (i < layout->num_sub_layouts)
  {
error:
    imlayout_vkb_free_layout(layout);
    layout = NULL;
  }

  fclose(fp);

  if (layout)
    vkb_init_buttons(collection, layout);

  return layout;
}


void
print_sublayout_info(vkb_sub_layout *layout)
{
  int i;

  if (!layout)
  {
    puts("\t\tSublayout is empty");
    return;
  }

  printf("\t\tVar index:\t\t\t%d\n", layout->variance_index);

  if (layout->label)
    printf("\t\tLabel:\t\t\t%s\n", layout->label);

  printf("\t\tType:\t\t\t%d\n", layout->type);
  printf("\t\tHash:\t\t\t%d\n", layout->hash);

  for (i = 0; i < layout->num_key_sections; i++)
  {
    vkb_key_section *key_section = &layout->key_sections[i];
    int j;

    printf("\t\tSection:\t\t%d\n", i);
    printf("\t\tNum Keys:\t\t%d\n", key_section->num_keys);
    printf("\t\tNum Rows:\t\t%d\n", key_section->num_rows);
    printf("\t\tMargins:\t\t%d:%d:%d:%d\n",
           key_section->margin_left, key_section->margin_top,
           key_section->margin_bottom, key_section->margin_right);


    if (!key_section->num_rows)
      continue;

    printf("\t\tNum Keys in Rows:\t");

    for (j = 0; j < key_section->num_rows; j++)
      printf("%d ", key_section->num_keys_in_rows[j]);

    putchar(10);

    if (!key_section->num_keys)
      continue;


    for (j = 0; j < key_section->num_rows; j++)
    {
      int k;

      printf("Row #%d: ", j);

      for (k = 0; k < key_section->num_keys_in_rows[j]; k++)
      {
        vkb_key *key = &key_section->keys[k];

        switch (key->key_type)
        {
          case KEY_TYPE_NORMAL:
          case KEY_TYPE_MODIFIER:
            printf(" %2s ", (gchar *)key->labels);
            break;
          case KEY_TYPE_SLIDING:
          {
            int l;

            for (l = 0; l < key->byte_count; l++)
              printf("{%s}", key->labels[l]);
          }
          case KEY_TYPE_MULTIPLE:
          {
            int l;

            putchar('[');

            for (l = 0; l < key->num_sub_keys; l++)
              printf(".%s.", (gchar *)key->sub_keys[l].labels);

            printf("] ");
          }
        }
      }

      putchar(10);
    }

    putchar(10);
  }
}

void
imlayout_vkb_free_layout_collection(vkb_layout_collection *coll)
{
  if (coll)
  {
    if (coll->name)
      g_free(coll->filename);

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
  char padding[20];
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

  if (!fread_check(padding, fp))
    goto cleanup;

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
  {
    fclose(fp);
    return NULL;
  };

  coll->filename = g_strdup(fname);

  for (i = 0; i < coll->num_layouts; i++)
  {
    if (!fseek(fp, coll->offsets[i], SEEK_SET))
    {
      char type;

      coll->layout_types[i] = LAYOUT_TYPE_NORMAL;

      if (!fread_check(type, fp))
        goto error;

      coll->layout_types[i] = type;
    }
    else
      goto error;
  }

  return coll;

error:
  g_warning("error reading layout type");
  imlayout_vkb_free_layout_collection(coll);
  return NULL;
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
        newkey->key_type = KEY_TYPE_MULTIPLE;
        newkey->byte_count = 0;
        newkey->num_sub_keys = 0;
        newkey->special_font = 0;
        newkey->key_flags = 0;
        newkey->scancode = 0;
        newkey->labels = NULL;
        newkey->sub_keys = NULL;
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

void
increase_num_keys_in_row(vkb_keyboard_layout *layout)
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

void
imlayout_vkb_free_layout(vkb_layout *layout)
{
  if (!layout)
    return;

  if (layout->sub_layouts)
  {
    int i;

    for (i = 0; i < layout->num_sub_layouts; i++)
    {
      vkb_sub_layout *sub_layout = &layout->sub_layouts[i];
      int j;

      if (sub_layout->label)
      {
        g_free(sub_layout->label);
        sub_layout->label = NULL;
      }

      for (j = 0; j < sub_layout->num_key_sections; j++)
      {
        vkb_key_section *key_section = &sub_layout->key_sections[j];
        int k;

        if (!key_section)
          continue;

        if (key_section->num_keys_in_rows)
        {
          g_free(key_section->num_keys_in_rows);
          key_section->num_keys_in_rows = NULL;
        }

        if (!key_section->keys)
          continue;

        for (k = 0; k < key_section->num_keys; k++)
        {
          vkb_key *key = &key_section->keys[k];
          int l;

          if (key->scancode)
          {
            g_free(key->scancode);
            key->scancode = NULL;
          }

          if (key->labels)
          {
            if (key->key_type == KEY_TYPE_SLIDING)
            {
              for (l = 0; l < key->byte_count; l++)
              {
                gchar *label = key->labels[l];

                if (label)
                {
                  g_free(label);
                  key->labels[l] = NULL;
                }
              }
            }

            g_free(key->labels);
            key->labels = NULL;
          }

          if (key->sub_keys)
          {
            for (l = 0; l < key->num_sub_keys; l++)
            {
              vkb_key *sub_key = &key->sub_keys[l];

              if (sub_key->scancode)
              {
                g_free(sub_key->scancode);
                sub_key->scancode = NULL;
              }

              if (sub_key->labels)
              {
                g_free(sub_key->labels);
                sub_key->labels = NULL;
              }
            }

            g_free(key->sub_keys);
            key->sub_keys = NULL;
          }
        }

        g_free(key_section->keys);
        key_section->keys = NULL;
      }

      g_free(sub_layout->key_sections);
    }
  }

  g_free(layout->sub_layouts);
  layout->sub_layouts = NULL;
  g_free(layout);
}
