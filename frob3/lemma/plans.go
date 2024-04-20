/*
Plan for a Secret:
  Client presents (Hostname, Mac) as a Principal (and might request authentication/encryption).
  If Client has registered a secret with Server,
    Server an offer a Challenge Nonce.
    Client computes Hash(Secret+Challenge), and replies with it.
    Server replies "+ok." or "-bad".
    Thereafter, if OK,
      Client sends with "Enc(Hash(Secret+Challenge+"client"))"
      Server sends with "Enc(Hash(Secret+Challenge+"server"))"


Client should speak first (to confuse random knockers):
  (1, n<256, 0x0101=c_vsn): "host=XYZZY,mac=123456789a,flags=010000,rom1=4444,fanout=6" "proto=crkey1,enc=arc49"
Server replies:
  (1, n<256, 0x0101=s_vsn): "+ok host=SFO,mac=1111111151,z=16.129+7" "proto=crkey1,enc=arc49,1=763876213862181"
                            "chain=MID,HUB"
Both will use MIN(c_vsn, s_vsn) thereafter.

Perhaps a (2) line for introducing or deleting a parent/kid.

If challenged, client needs to respond:
  (1, n<256, 0x0101=c_vsn): "2=743274983274932798327932".
And server can reply
  (1, n<256, 0x0101=s_vsn): "+ok u=strick,g=cocoio+frobio+f256" | "-bad"

Thereafter, either side can initiate.
To keep channels unique:
  Client-initated channels even.
  Server-initated channels odd.


TODO:
  /b1: per-user       ( at peer )
  /b2: public sharing ( at peer )
  /b3: per-user       ( at top )
  /b4: public sharing ( at top )
  /r/HOST/:   Remote General FS

*/

package lemma
