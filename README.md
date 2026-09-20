# pancake-shell
 
A Unix shell written from scratch in C, implementing command execution, custom PATH searching, built-in commands, and output redirection.
 
## Features
 
- **Command execution** via `fork()`/`execvp()`/`waitpid()`
- **Custom PATH searching** — the shell maintains its own path list (defaults to `/bin` and `/usr/bin`) rather than relying on the system `$PATH`, checked with `access()` before attempting to execute
- **Built-in commands**, handled directly in the shell process rather than forked:
  - `cd <dir>` — change directory
  - `path <dir1> <dir2> ...` — replace the shell's search path (no arguments clears it)
  - `exit` — quit the shell
  - `debug` — print the current search path (development/debugging aid)
- **Output redirection** (`>`) — redirects a command's stdout to a file, implemented with `dup`/`dup2`/`open` around the forked child
- **Custom line parsing** — tokenizes input using `strsep`, with a growable `argv` array (starts at a fixed size, doubles via `realloc` as needed)
## Build & Run
 
```bash
gcc shell.c -o pancake
./pancake
```
 
## Usage
 
```
pancake> ls -la
pancake> cd /tmp
pancake> path /bin /usr/bin /usr/local/bin
pancake> ls > out.txt
pancake> exit
```
 
## Design
 
The execution flow for each line typed at the prompt:
 
```
read line -> parse into tokens (argv)
  -> check for redirection (>)
       -> if found: fork, redirect stdout in the child, run the rest of the pipeline
  -> check built-ins (cd, path, exit, debug)
       -> if not a built-in: search the shell's PATH list, fork + exec if found
```
 
Built-ins are deliberately **not** forked — they run directly in the shell's own process, since things like `cd` only make sense if they affect the shell itself rather than a short-lived child.

## Known Limitations / Future Improvements
 
- No pipes (`|`)
- No background/parallel execution (`&`) yet
- No quoted-string argument parsing (`echo "hello world"` splits into two arguments)
- No job control (`fg`, `bg`, `jobs`)
- 
## License
 
MIT
