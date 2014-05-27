mlocate32
=========

mlocate for Windows.


Installation
------------

Copy exe files below into one of the PATH directory.

- bin\updatedb32.exe
- bin\locate32.exe


Usage
-----

### make cache ###

    > updatedb32

This command make mlocate32 database named "{current drive}\updatedb".


### find location of files ###

    > locate32 <filename-pattern>

This command find files quickly from mlocate32 database.

#### locate32 detailed usage
    
    Usage: locate32 [options] <file-name-pattern>
    Search for entries in a mlocate32 database.
    
    Options:
      -h [ --help ]               Display this help message
      -s [ --size ] [+-]N[bcwkMG] File size for filtering (Specifying it as one of
                                  'find' command options)
    
    Sample usage:
      locate32 hoge.exe      # Match named "*hoge.exe*".
      locate32 .dat -s +10M  # Match named "*.dat*" and size is bigger than 10MB.
      locate32 .dat -s +10G  # Match named "*.dat*" and size is bigger than 10GB.


Build
-----

You can build this solution "mlocate32.sln" by Visual Studio 2010 if you want.
