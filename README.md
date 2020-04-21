Note Sync
=========

* Create a **non-empty** file called `pool-server/notes.txt`
* Run client(s) and refer to file in `pool-client/notes.txt`
* The client `notes.txt` will update across all clients.
* The default username and password are `m`
* Supported OS: Linux, Mac OS X

```bash
# Terminal #1: Run server
echo "My notes!" >pool-server/notes.txt
./server

# Terminal #2: Run client
./client [optional: server_ip]

# Terminla #3: View/Edit pool-client/notes.txt
vim pool-client/note.txt
```

TODOs
-----
- [ ] BUG: Cannot sync empty file
- [ ] Handle conflicts during patch
