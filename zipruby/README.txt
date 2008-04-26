= Zip/Ruby

Copyright (c) 2008 SUGAWARA Genki <sgwr_dts@yahoo.co.jp>

== Description

Ruby bindings for libzip.

libzip is a C library for reading, creating, and modifying zip archives.

== Project Page

http://rubyforge.org/projects/zipruby

== Install

gem install zipruby

== Download

https://rubyforge.org/frs/?group_id=6124

== Example
=== reading zip archives

    require 'zipruby'
    
    Zip::Archive.open('filename.zip') do |ar|
      n = ar.num_files # number of entries
    
      n.times do |i|
        entry_name = ar.get_name(i) # get entry name from archive
    
        # open entry
        ar.fopen(entry_name) do |f| # or ar.fopen(i) do |f|
          name = f.name           # name of the file
          size = f.size           # size of file (uncompressed)
          comp_size = f.comp_size # size of file (compressed)
    
          content = f.read # read entry content
        end
      end
    
      # Zip::Archive includes Enumerable
      entry_names = ar.map do |f|
        f.naem
      end
    end

=== creating zip archives

    require 'zipruby'
    
    Zip::Archive.open('filename.zip', Zip::CREATE) do |ar|
      ar.add_file('foo.txt') # add file to zip archive
    
      # add file to zip archive from File object
      open('bar.txt') do |f|
        ar << f # or ar.add_filep(f)
      end
    
      # add file to zip archive from buffer
      ar.add_buffer('zoo.txt', 'Hello, world!')
    end
    
    # include directory in zip archive
    Zip::Archive.open('filename.zip') do |ar|
      ar.add_file('dirname/foo.txt', 'foo.txt')
          # args: <entry name>,      <source>
    
      open('bar.txt') do |f|
        ar.add_filep('dirname/bar.txt', f)
             # args: <entry name>,      <source>
      end
    
      ar.add_buffer('dirname/zoo.txt', 'Hello, world!')
            # args: <entry name>,      <source>
    end

=== modifying zip archives

    require 'zipruby'
    
    Zip::Archive.open('filename.zip') do |ar|
      # replace file in zip archive
      ar.replace_file(0, 'foo.txt')
    
      # replace file in zip archive with File object
      open('bar.txt') do |f|
        ar.replace_filep(1, f)
      end
    
      # replace file in zip archive with buffer
      ar.replace_buffer(2, 'Hello, world!')

      # add or replace file in zip archive
      ar.add_or_replace_file(3, 'foo.txt')
    end
    
    # appends comment    
    Zip::Archive.open('filename.zip') do |ar|
      ar.comment = <<-EOS
        jugem jugem gokou no surikere
        kaijari suigyo no
        suigyoumatsu unraimatsu furaimatsu
      EOS
    end
    
    # ar1 imports ar2 entries
    Zip::Archive.open('ar1.zip') do |ar1|
      Zip::Archive.open('ar2.zip') do |ar2|
        ar1.update(ar2)
      end
    end

== License
    Copyright (c) 2008 SUGAWARA Genki <sgwr_dts@yahoo.co.jp>
    All rights reserved.
    
    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:
    
        * Redistributions of source code must retain the above copyright notice, 
          this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright notice, 
          this list of conditions and the following disclaimer in the documentation 
          and/or other materials provided with the distribution.
        * The names of its contributors may be used to endorse or promote products 
           derived from this software without specific prior written permission.
    
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
    ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
    FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
    DAMAGE.

=== libzip
Zip/Ruby contains libzip.

libzip is a C library for reading, creating, and modifying zip archives.

* distribution site:
  * http://www.nih.at/libzip/
  * ftp.nih.at /pub/nih/libzip

* authors:
  * Dieter Baron <dillo@giga.or.at>
  * Thomas Klausner <tk@giga.or.at>
