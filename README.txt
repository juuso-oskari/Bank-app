Make an application that implements a rudimentary bank. A bank contains N
service desks, each of which should run in it’s own thread.

- N 'palvelupistettä', joista jokainen omassa threadissä. Mahdollisesti struct rakenne per asiakas.

Each thread waits for messages from it’s own message queue. Based on queue lengths in a shared
memory pool the client selects the desk with the shortest queue and waits until
the desk is free (if there’s a previous client) or starts communication with the
desk using the message queue, one service request at a time. The accounts have
R/W locks.

- Palvelupisteillä omat viestijononsa, jotka käsitellään viesti kerrallaan. Yhteensä siis N viestijonoa,
ja N viestin käsittelyä käynnissä samaan aikaan.

The bank servers maintain the collected balance of the bank, that is the sum of
all balances in the bank.

- Täytyy pitää yllä pankin tasetta, eli jokaisen palvelupisteen on kirjattava talletukset ja nostot taseeseen. Tämä pankin taseeseen
kirjaus on siis kriittinen ominaisuus, jossa pitää käyttää synkronointia (mutex) ettei samanaikaista kirjaamista tapahdu, joka voi johtaa
virheeseen.

In addition there is a master thread that queries the balance of each desk, that
is how much the clients have deposited and withdrawn funds. The master thread
can overtake all queued customers but must not interrupt the current client.
The desks should not proceed with other clients until all the desks have reported
to the master thread.

- Master thread voi tasekyselyllään ohittaa jonossa olevat asiakkaat, mutta ei käsittelyssä olevia asiakkaita. Kyselyn
tulos palautuu, kunhan kaikki asiakkaat on käsitelty ja heidän toimintonsa on kirjattu taseeseen (palvelupisteet eivät
ota uusia asiakkaita käsittelyyn ennen kuin master threadin tasekysely on valmis).

Clients can send, for example, the following commands:
“l 1”: give balance of account 1 “w 1 123”: withdraw 123 euros from account 1
“t 1 2 123”: transfer 123 euros from account 1 to account 2 “d 1 234”: deposit
234 euros to account 1
9
Commands are given on command line.
For more challenge, add some kind of bank statement that can be queried. Bank
statement is not an absolute requirement for this work and is not required for
full points.