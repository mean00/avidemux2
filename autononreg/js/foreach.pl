#!/usr/bin/perl

$ARGC = scalar(@ARGV);

if ($ARGC <2){
  if ($ARGV[0] eq '-h') {
    &longhelp()
  }else{
    &help();
  }
  exit(1);
}

for ($i=0; $i<$ARGC-1; $i++){

  my $command  = $ARGV[$ARGC-1];
  my $fullfile = $ARGV[$i];
  my ($suffix, $base, $dir, $file);

  if (rindex($fullfile,'/')!=-1){
    $file = substr($fullfile,rindex($fullfile,'/')+1);
    $dir  = substr($fullfile,0,rindex($fullfile,'/'));
  }else{
    $file = $fullfile;
    $dir  = '';
  }

  if (rindex($file,'.')!=-1){
    $suffix = substr($file,rindex($file,'.')+1);
    $base   = substr($file,0,rindex($file,'.'));
  }else{
    $suffix = '';
    $base   = $file; 
  }

  $fullfile = quotemeta($fullfile);
  $dir      = quotemeta($dir);
  $file     = quotemeta($file);
  $base     = quotemeta($base);
  $suffix   = quotemeta($suffix);

  my $count = 0;
  $count += $command =~ s/%l/$fullfile/g;
  $count += $command =~ s/%d/$dir/g;
  $count += $command =~ s/%f/$file/g;
  $count += $command =~ s/%b/$base/g;
  $count += $command =~ s/%s/$suffix/g;

  if ($count){
    system ($command);
  }else{
    system ("$command $fullfile");
  }

}

##############################################################################
# print a short Help text
sub help() {
print STDERR <<STOP
  Syntax: foreach.pl <file[s]> "<command>"

    Missing parameters. Run 'foreach.pl -h' for help 
STOP
}


##############################################################################
# print a longer Help text
sub longhelp() {
print STDERR <<STOP

      SYNTAX

      foreach.pl <file[s]> "<command>"

      INFO

      foreach.pl runs a command on multiple files. It is not as mighty as
      find's execute function, but much easyier to use and it's syntax 
      should be much better to remember.

      PARAMETERS

      <file[s]> are one or more files that should be handled by <command>,
      use the shell's globbing for a lot of files.

      The following parameters can be used for <command>:

      %l full filename (with path)
      %d directory part of above filename
      %f filename without directory
      %b basename of file (no suffix, no directory)
      %s suffix (without the dot) 
      
      When no parameter is given the script assumes "<command> %l" as
      given command. Special chars in each parameter are quoted by the
      script.

      EXAMPLES

      foreach.pl /etc/* "tar -czf /tmp/%b.tgz %l"

      This will tar and gzip every file in /etc to a single tgz in /tmp
      I know that isn't really useful but it's just an example :-)

      foreach.pl *.tar.gz "tar -xzvf"

      This is somehow more useful. It extracts all tar.gz files in the
      current directory.

      Go and find some useful examples for yourself ;-)

      BUGS

      Only the parameters are quoted before passing the to the shell,
      quoting your commands is up to you.

      LICENSE

      foreach.pl - Does things to multiple files
      Copyright (C) 2001 Andreas Gohr <a.gohr\@web.de>

      This program is free software; you can redistribute it and/or
      modify it under the terms of the GNU General Public License as
      published by the Free Software Foundation; either version 2 of
      the License, or (at your option) any later version.

      See COPYING for details
STOP
}
