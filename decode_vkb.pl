#!/usr/bin/perl -w

# decode_vkb is a perl script to decode binary virtual keyboard definition
# files from Nokia n800 Internet tablet to a XML format for editing them.
# The generated XML-files can be encoded back to binary .vkb format using
# gen_vkb utility from Nokia.
#
# (C) 2007 Kimmo Jukarainen <kimju-2007@inside.org>
#
# This program is licensed under GPLv2. See LICENSE.txt

# -------------------------------------------------------------------- #

# usage: ./decode_vkb file.vkb > file.xml

# -------------------------------------------------------------------- #

# Version: 0.01 - 2007-08-21
#  - initial release

# -------------------------------------------------------------------- #

use strict;
use vars qw/$fh $idt/; # the ugly globals: filehandle and indentation level

# -------------------------------------------------------------------- #

die "Invalid parameters.\n" .
    "Usage: $0 file.vkb > file.xml\n" unless scalar @ARGV == 1;
die "Can't open file\n" unless open($fh, $ARGV[0]);
binmode $fh;

# -------------------------------------------------------------------- #

sub getByteInt() {
  my $tmp;
  read($fh, $tmp, 1);
  return ord($tmp);
}

sub getWordInt() {
  my $tmp = getByteInt;
  $tmp += getByteInt << 8;
  return $tmp;
}

sub getString() {
  my $len = getByteInt();
  my $tmp;
  read($fh, $tmp, $len);
  return $tmp;
}

sub getStringBytes() {
  my $len = getByteInt();
  my $tmp;
  for (my $i = 0; $i < $len; $i++) {
    $tmp .= getByteInt();
    $tmp .= ' ';
  }
  return $tmp;
}
 

# -------------------------------------------------------------------- #

sub xprint($) {
  my $line = shift;
  print "\t" x $idt, $line, "\n";
}

# -------------------------------------------------------------------- #

sub escapeXML($) {
  my $str = shift;
  $str =~ s:&:&amp;:g;
  $str =~ s:<:&lt;:g;
  $str =~ s:>:&gt;:g;
  return $str;
}

# -------------------------------------------------------------------- #

sub decodeHeader() {
  # 00000000  01 02 0d 53 75 6f 6d 69  20 28 53 75 6f 6d 69 29  |...Suomi (Suomi)|
  # 00000010  05 66 69 5f 46 49 05 66  69 5f 46 49 02 00 01 05  |.fi_FI.fi_FI....|
  # 00000020  23 23 1a 00 00 32 23 1a  00 00 37 23 1a 00 00 48  |##...2#...7#...H|
  # 00000030  37 29 00 00 8f 37 29 00  00 51 00 58 02 00 00 00  |7)...7)..Q.X....|
  # 00000040  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|

  my %vkb;
  $vkb{'version'} = getByteInt();
  $vkb{'layouts'} = getByteInt();
  $vkb{'langname'} = getString();
  $vkb{'langcode'} = getString();
  $vkb{'wordcomp'} = getString();
  $vkb{'screenmodes'} = getByteInt();
  for (my $i = 0; $i < $vkb{'screenmodes'}; $i++) {
    $vkb{'screenmode_'.$i} = getByteInt();
  }
  $vkb{'keysizes'} = getByteInt();
  for (my $i = 0; $i < $vkb{'keysizes'}; $i++) {
    my %keysize;
    $keysize{'width'} = getByteInt();
    $keysize{'height'} = getByteInt();
    $keysize{'baseline'} = getByteInt();
    $keysize{'top'} = getByteInt();
    $keysize{'left'} = getByteInt();
    $vkb{'keysize_'.$i} = \%keysize;
  }

  # There's no information about how many layouts the file has.
  # I'm guessing that there can be as many layouts as 16-bit 
  # offsets can handle.
  # The vkb specification says that there are 20 reserved bytes
  # after the offset array. Currently those are all zeros, so I'm
  # using them as sentinel to the offset array and read offsets
  # until we hit the offset 0x0000.
  
  my $tmp;
  my $i = 0;
  while ($tmp = getWordInt()) {
    $vkb{'offset_'.$i} = $tmp;
    $i++;
  }
  $vkb{'offsets'} = $i;
  
  return %vkb;
}

# -------------------------------------------------------------------- #

sub printHeader($) {

  my %vkb = %{$_[0]};
  $idt = 0;

  my $fname = $ARGV[0];
  $fname =~ s:.*/([^/]+)$:$1:;
  $fname =~ s:\.([^.]+)$::;

  xprint 
    '<keyboards file="' . $fname . '" version="' . $vkb{'version'} . 
    '" lang="' . $vkb{'langcode'} . '" name="' . $vkb{'langname'} . 
    '" wc="' . $vkb{'wordcomp'} .'">';

  $idt++;
  xprint '<screen>';
  $idt++;

  my %screenmodes = ( '0' => 'NORMAL', '1' => 'FULL', '2' => 'ROTATED' );
  for (my $i = 0; $i < $vkb{'screenmodes'}; $i++) {
    if (defined $screenmodes{$vkb{'screenmode_'.$i}}) {
      xprint '<mode name="'.$screenmodes{$vkb{'screenmode_'.$i}}.'"/>';
    } else {
      xprint '<mode name="'.$vkb{'screenmode_'.$i}.'"/>';
      warn "Unknown screenmode: " . $vkb{'screenmode_'.$i};
    }
  }
  $idt--;
  xprint '</screen>';

  xprint "<keysizes>";
  $idt++;
  for (my $i = 0; $i < $vkb{'keysizes'}; $i++) {
    my %k = %{$vkb{'keysize_'.$i}};
    xprint
      '<size height="'.$k{'height'}.
      '" baseline="'.$k{'baseline'}.
      '" width="'.$k{'width'}.
      '" margin_top="'.$k{'top'}.
      '" margin_left="'.$k{'left'}.
      '" />';
  }
  $idt--;
  xprint "</keysizes>";

}

# -------------------------------------------------------------------- #

sub decodeLayout($) {

  my $offset=shift;
  seek($fh, $offset, 0);

  my %layout;

  $layout{'type'} = getByteInt();
  $layout{'subs'} = getByteInt();
  $layout{'numeric'} = getByteInt();
  $layout{'defsize'} = getByteInt();
  
  for (my $i = 0; $i < $layout{'subs'}; $i++) {
    $layout{'sub_'.$i} = decodeSubLayout();
  }
  
  return \%layout;
}

# -------------------------------------------------------------------- #

sub printLayout($) {
  my %l = %{$_[0]};

  my %layouttypes = ( 
    '0' => 'NORMAL',
    '1' => 'SPECIAL',
    '3' => 'LARGE',
    '4' => 'THUMB',
    '5' => 'PINYIN',
    '6' => 'ZHUYIN',
    '7' => 'CANGJIE',
    '8' => 'STROKE'
  );

  if (defined $layouttypes{$l{'type'}}) {
    xprint '<keyboard layout="'.$layouttypes{$l{'type'}}.
      '" default_key_size="'.$l{'defsize'}.'"' . 
      ( $l{'numeric'} ? ' numeric="'.$l{'numeric'}.'"' : '' ) . '>';
  } else {
    xprint '<keyboard layout="'.$l{'type'}.
      '" default_key_size="'.$l{'defsize'}.'"'.
      ( $l{'numeric'} ? ' numeric="'.$l{'numeric'}.'"' : '' ) . '>';

    warn "Unknown layouttype: " . $l{'type'};
  }

  $idt++;
  for (my $i = 0; $i < $l{'subs'}; $i++) {
    printSubLayout($l{'sub_'.$i});
  }
  $idt--;
  xprint '</keyboard>';
}

# -------------------------------------------------------------------- #

sub decodeSubLayout() {

  my %sublay;

  $sublay{'type'} = getByteInt();
  $sublay{'variance'} = getByteInt();
  $sublay{'label'} = getString();
  $sublay{'keysects'} = getByteInt();

  for (my $i = 0; $i < $sublay{'keysects'}; $i++) {
    $sublay{'keysect_'.$i} = decodeKeysect();
  }
  
  return \%sublay;
}

# -------------------------------------------------------------------- #

sub printSubLayout($) {
  my %s = %{$_[0]};

  my %sublayouttypes = (
    '0' => 'LOWERCASE',
    '1' => 'UPPERCASE',
    '2' => 'SINGLE',
  );
  
  if (defined $sublayouttypes{$s{'type'}}) {
    xprint '<sublayout type="'.$sublayouttypes{$s{'type'}}.
      '" variance_index="'.$s{'variance'}.'" label="'.$s{'label'}.'">';
  } else {
    xprint '<sublayout type="'.$s{'type'}.
      '" variance_index="'.$s{'variance'}.'" label="'.$s{'label'}.'">';
    warn "Unknown sublayouttype: " . $s{'type'};
  }
  
  $idt++;
  for (my $i = 0; $i < $s{'keysects'}; $i++) {
    printKeysect($s{'keysect_'.$i});
  }
  $idt--;
  xprint '</sublayout>';
}

# -------------------------------------------------------------------- #

sub decodeKeysect() {

  my %keysect;

  $keysect{'keys'} = getByteInt();
  $keysect{'rows'} = getByteInt();
  $keysect{'left'} = getByteInt();
  $keysect{'top'} = getByteInt();
  $keysect{'bottom'} = getByteInt();
  $keysect{'right'} = getByteInt();

  for (my $i = 0; $i < $keysect{'rows'}; $i++) {
    $keysect{'rowsize_'.$i} = getByteInt;
  }

  for (my $i = 0; $i < $keysect{'rows'}; $i++) {
    $keysect{'row_'.$i} = [];
    for (my $j = 0; $j < $keysect{'rowsize_'.$i}; $j++) {
      push(@{$keysect{'row_'.$i}}, decodeKey());
    }
  }

  return \%keysect;
}

# -------------------------------------------------------------------- #

sub printKeysect($) {
  my %k = %{$_[0]};
  
  xprint '<keysection margin_top="'.$k{'top'}.
    '" margin_left="'.$k{'left'}.
    '" margin_bottom="'.$k{'bottom'}.
    '" margin_right="'.$k{'right'}.'">';
  $idt++;
  for (my $i = 0; $i < $k{'rows'}; $i++) {
    printKeyrow($k{'row_'.$i});
  }
  $idt--;
  xprint '</keysection>';
}

# -------------------------------------------------------------------- #

sub printKeyrow($) {
  my @row = @{$_[0]};
  xprint '<row>';
  $idt++;
  
  foreach my $keyref (@row) {
    printKey($keyref);
  }
  
  $idt--;
  xprint '</row>';

}

# -------------------------------------------------------------------- #

sub decodeKey();
sub decodeKey() {
  # normal and slide:
  # 000002f0  00 01 01 76 03 00 05 01  62 03 00 01 01 6e 03 00  |...v....b....n..|
  # 00000300  01 01 6d 03 01 10 8a 01  2e 01 2c 01 2d 01 21 01  |..m.......,.-.!.|
  # 00000310  3f 01 26 01 3b 01 3a 01  5f 01 40 03 01 00 03 41  |?.&.;.:._.@....A|
  # 00000320  42 43 01 1f 03 00 00 00  00 0b 0b 09 00 01 01 51  |BC.............Q|

  my %key;
  $key{'type'} = getByteInt();
  $key{'special_font'} = 0;
  if ($key{'type'} & 0x80) {
    $key{'type'} = $key{'type'} ^ 0x80;
    $key{'special_font'} = 1;
  }
  $key{'count'} = 1;

  if ($key{'type'} == 4 ) {
    # multiple
    # 00000070  00 00 00 01 72 00 01 d2  04 07 00 01 01 74 00 00  |....r........t..|
    # 00000080  01 01 79 00 00 01 01 75  00 00 01 01 69 00 00 01  |..y....u....i...|
    # 00000090  01 6f 00 00 09 01 70 00  00 01 02 c3 a5 01 00 05  |.o....p.........|
    # 000000a0  01 61 00 00 01 01 73 00  00 05 01 64 00 00 05 01  |.a....s....d....|
    # type size # 1st normal key # 2nd            #     # last              #
    # 04   07   # 00 01 01 74 00 # 00 01 01 79 00 # ... # 00 01 02 c3 a5 01 # 

    $key{'count'} = getByteInt();

    my @tmpkeys;
    for (my $i = 0; $i < $key{'count'}; $i++) {
      push(@tmpkeys, decodeKey());
    }
    $key{'keys'} = \@tmpkeys;
  } else {
    $key{'attributes'} = getByteInt;
    if ($key{'attributes'} & 0x80) {
      $key{'attributes'} += (getByteInt() << 8);
    }
    if ($key{'type'} == 1) {
      $key{'count'} = getByteInt() ^ 0x80;
      my @labels;
      for (my $i = 0; $i < $key{'count'}; $i++ )
      {
        my $label = getString;
        push(@labels, $label)
      }
      $key{'labels'} = \@labels;
    } else {
      $key{'label'} = getString;
    }
    $key{'size'} = getByteInt();
    if ($key{'attributes'} & 0x100) {
      $key{'scancode'} = getStringBytes;
    }
  }
  return \%key;
}

# -------------------------------------------------------------------- #

sub printKey($);
sub printKey($) {
  my $keyref = shift;
  if ( $keyref->{'type'} == 0 || $keyref->{'type'} == 2 
     ) {
     # normal
     xprint '<key size="' . $keyref->{'size'} . '"' .
       (($keyref->{'special_font'} == 1) ? ' special_font="SPECIAL_FONT"' : "" ) .
       (($keyref->{'attributes'} & (0x01)) ? ' alpha="ALPHA"' : "" ) .
       (($keyref->{'attributes'} & (0x02)) ? ' numeric="NUMERIC"' : "" ) .
       (($keyref->{'attributes'} & (0x04)) ? ' hexa="HEXA"' : "" ) .
       (($keyref->{'attributes'} & (0x08)) ? ' tele="TELE"' : "" ) .
       (($keyref->{'attributes'} & (0x10)) ? ' special="SPECIAL"' : "" ) .
       (($keyref->{'attributes'} & (0x20)) ? ' dead="DEAD"' : "" ) .
       (($keyref->{'attributes'} & (0x40)) ? ' whitespace="WHITESPACE"' :"" ) .
       (($keyref->{'attributes'} & (0x100)) ? ' raw="RAW" scancode="' . $keyref->{'scancode'} . '"'  :"" ) .
       (($keyref->{'attributes'} & (0x200)) ? ' tone="TONE"' :"" ) .
       (($keyref->{'attributes'} & (0x400)) ? ' tab="TAB"' :"" ) .
       (($keyref->{'attributes'} & (0x800)) ? ' backspace="BACKSPACE"' :"" ) .
       (($keyref->{'attributes'} & (0x1000)) ? ' shift="SHIFT"' :"" ) .
       (($keyref->{'type'} == 0x02 ) ? ' modifier="MODIFIER"' : "" ) .
       '>' . escapeXML($keyref->{'label'}) . '</key>';

  } elsif ( $keyref->{'type'} == 1 ) {
    # slide
    xprint '<key type="SLIDE" size="' . $keyref->{'size'} . '"' .
      (($keyref->{'special_font'} == 1) ? ' special_font="SPECIAL_FONT"' : "" ) .
      (($keyref->{'attributes'} & (0x01)) ? ' alpha="ALPHA"' : "" ) .
      (($keyref->{'attributes'} & (0x02)) ? ' numeric="NUMERIC"' : "" ) .
      (($keyref->{'attributes'} & (0x04)) ? ' hexa="HEXA"' : "" ) .
      (($keyref->{'attributes'} & (0x08)) ? ' tele="TELE"' : "" ) .
      (($keyref->{'attributes'} & (0x10)) ? ' special="SPECIAL"' : "" ) .
      (($keyref->{'attributes'} & (0x20)) ? ' dead="DEAD"' : "" ) .
      (($keyref->{'attributes'} & (0x40)) ? ' whitespace="WHITESPACE"' :"" ) .
      (($keyref->{'attributes'} & (0x100)) ? ' raw="RAW" scancode="' . $keyref->{'scancode'} . '"'  :"" ) .
      (($keyref->{'attributes'} & (0x200)) ? ' tone="TONE"' :"" ) .
      (($keyref->{'attributes'} & (0x400)) ? ' tab="TAB"' :"" ) .
      (($keyref->{'attributes'} & (0x800)) ? ' backspace="BACKSPACE"' :"" ) .
      (($keyref->{'attributes'} & (0x1000)) ? ' shift="SHIFT"' :"" ) .
      '>';

    $idt++;
    foreach my $a (@{$keyref->{'labels'}}) {
      xprint '<slide>' . escapeXML($a) . '</slide>';
    }
    $idt--;
    xprint '</key>';

  } elsif ( $keyref->{'type'} == 4 ) {
    # multiple
    xprint '<multiple>';
    $idt++;
    for $a (@{$keyref->{keys}}) {
      printKey($a);
    }
    $idt--;
    xprint '</multiple>';
  } else {
    warn "Unknown key type"
  }
}

# -------------------------------------------------------------------- #

sub printXML($) {
  my $vkbref = shift;
  printHeader($vkbref);
  for (my $i = 0; $i < $vkbref->{'offsets'}; $i++) {
    printLayout($vkbref->{'layout_'.$i});
  }
  $idt--;
  xprint '</keyboards>';
}

# -------------------------------------------------------------------- #

my %vkb = decodeHeader;

for (my $i = 0; $i < $vkb{'offsets'}; $i++) {
  $vkb{'layout_'.$i} = decodeLayout($vkb{'offset_'.$i});
}

printXML(\%vkb);

# -------------------------------------------------------------------- #
