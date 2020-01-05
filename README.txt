An application that implements a rudimentary bank. A bank contains N
service desks, each of which should run in it’s own thread.

Each thread waits for messages from it’s own message queue. Based on queue lengths in a shared
memory pool the client selects the desk with the shortest queue and waits until
the desk is free (if there’s a previous client) or starts communication with the
desk using the message queue, one service request at a time. The accounts have
R/W locks.

The bank servers maintain the collected balance of the bank, that is the sum of
all balances in the bank.

In addition there is a master thread that queries the balance of each desk, that
is how much the clients have deposited and withdrawn funds. The master thread
can overtake all queued customers but must not interrupt the current client.
The desks should not proceed with other clients until all the desks have reported
to the master thread.

Clients can send, for example, the following commands:
“l 1”: give balance of account 1 “w 1 123”: withdraw 123 euros from account 1
“t 1 2 123”: transfer 123 euros from account 1 to account 2 “d 1 234”: deposit
234 euros to account 1
9
Commands are given on command line.
