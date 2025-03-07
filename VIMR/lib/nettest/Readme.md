This is to test whether you can establish a peer-to-peer conncetion between two computers.

1. Make sure none of the VIMR or eHui stuff is running on either of the computers
2. One computer runs `host.bat`
3. The other computer waits until the host is at the `Waiting for Peer(s) to join` line and then runs `test.bat`
4. Both of them should see something like the included succes-ping image
   1. The important bit is where it says `(it works!)`, that means you're good to go!
   2. If the output gets stuck repeating `Waiting for Peer(s) to join` then one of the computers is behind an unfriendly firwall and you can't do telepresence.