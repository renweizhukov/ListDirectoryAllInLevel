# ListDirectoryAllInLevel

Given a directory on Windows, this executable lists all its subdirectories and files which are sorted according to their levels.

Command usage:

```sh
ListDirectoryAllInLevel.exe [path-to-directory/file]
```

Note that although the executable itself can handle Unicode characters, e.g., Chinese characters, you need some extra steps to display the Chinese characters correctly in the console.

(1) In the console, switch the code page to UTF-8, i.e., code page 65001.

```sh
chcp 65001
```

(2) Change the console font to one font which supports Chinese, e.g., FangSong. 