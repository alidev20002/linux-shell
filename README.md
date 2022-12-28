# Linux Shell

Our Linux shell supports all Linux commands and a few extra commands.
## Requirements
use the following command to install readline library before running the shell:
```bash
sudo apt-get install libreadline-dev
```
## Run
```bash
gcc shell.c -lreadline
./a.out
```
## List of additional commands

* __fs__ → prints the first string of each line of input file

* __mw__ → prints the most frequent word in file

* __rs__ → removes empty spaces from file

* __rmc__ → prints uncommented lines of file

* __lc__ → prints the number of file lines

* __ft__ → prints first ten lines of file

## Usage
All additional commands are used this way:
```bash
[command] [file path]
``` 
For example if test file be as follows:
```txt
Good morning
hi ali
hello world
```
The result will be:
```bash
fs /Desktop/test.txt
Good
hi
hello
``` 
## Features
* Save commands in history

* User can switch between commands that saved in history and choose one of them

* Support pipeline commands

* Fork new process for every command

* Print errors using stderr

* Customize control-c action
